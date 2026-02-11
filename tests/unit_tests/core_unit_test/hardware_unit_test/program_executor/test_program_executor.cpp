#include <gtest/gtest.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <iostream>
#include <filesystem>
#include <thread>
#include <chrono>
#include <cstdlib>

#include "hardware/program_executor.hpp"
#include "utils/thread_pool.hpp"
#include "assembly_program_builder.hpp"

class ProgramExecutorTest : public ::testing::Test {
protected:
    void SetUp() override {
        if (!spdlog::default_logger()) {
            spdlog::set_default_logger(spdlog::stdout_color_mt("test_logger_fallback"));
        }
        // Setup logging for this test
        setupTestLogging();

        SPDLOG_INFO("Starting ProgramExecutor test setup");

        // Initialize test data
        instr_clock = 0;
        max_instructions_per_tick = 10;
        instr_ptr_register = 0;

        // Create thread pool with 4 worker threads
        job_pool = std::make_unique<ThreadPool<AsyncProgramJob>>(4);

        // Create program executor
        program_executor = std::make_unique<ProgramExecutor>(
            instr_clock,
            max_instructions_per_tick,
            instr_ptr_register,
            *job_pool
        );

        SPDLOG_DEBUG("ProgramExecutor test setup complete - thread pool ready with 4 workers");
    }

    void TearDown() override {
        SPDLOG_INFO("Cleaning up ProgramExecutor test");

        // Wait for any pending jobs to complete
        if (job_pool) {
            job_pool->await_jobs();
            SPDLOG_TRACE("All jobs completed, cleaning up");
        }

        program_executor.reset();
        job_pool.reset();

        SPDLOG_DEBUG("ProgramExecutor test cleanup complete");

        // Handle log cleanup based on test result
        handleLogCleanup();
    }

    void setupTestLogging() {
        // Create log file for this test case
        std::string test_name = ::testing::UnitTest::GetInstance()->current_test_info()->name();
        log_filename = "test_" + test_name + ".log";
        logger_name = "test_logger_" + test_name;

        try {
            auto logger = spdlog::get(logger_name);
            if (!logger) {
                logger = spdlog::basic_logger_mt(logger_name, log_filename, true);
            }
            spdlog::set_default_logger(logger);
            spdlog::set_level(spdlog::level::trace);
        } catch (const spdlog::spdlog_ex& ex) {
            std::cerr << "Log initialization failed: " << ex.what() << std::endl;
            if (!spdlog::default_logger()) {
                spdlog::set_default_logger(spdlog::stdout_color_mt("test_logger_fallback"));
            }
        }
    }

    void handleLogCleanup() {
        // Check if we should keep logs based on environment variable
        bool keep_logs = false;
#if defined(_WIN32)
        char* keep_logs_env = nullptr;
        size_t keep_logs_len = 0;
        keep_logs = (_dupenv_s(&keep_logs_env, &keep_logs_len,
                               "ANTS_TEST_KEEP_LOGS") == 0 &&
                     keep_logs_env != nullptr);
        if (keep_logs_env) {
            free(keep_logs_env);
        }
#else
        const char* keep_logs_env = std::getenv("ANTS_TEST_KEEP_LOGS");
        keep_logs = (keep_logs_env != nullptr && *keep_logs_env != '\0');
#endif
        bool test_passed = !::testing::Test::HasFailure();

        // Ensure log file is flushed before file operations.
        if (auto logger = spdlog::get(logger_name)) {
            logger->flush();
        }

        auto fallback = spdlog::get("test_logger_fallback");
        if (!fallback) {
            fallback = spdlog::stdout_color_mt("test_logger_fallback");
        }
        spdlog::set_default_logger(fallback);
        spdlog::drop(logger_name);

        // Always create run.log for transcript
        std::filesystem::copy_file(log_filename, "run.log",
            std::filesystem::copy_options::overwrite_existing);

        if (!keep_logs && test_passed) {
            std::filesystem::remove(log_filename);
        }
    }

    void waitForAsyncCompletion() {
        // Wait for all async jobs to complete
        job_pool->await_jobs();
        SPDLOG_TRACE("All async jobs completed");
    }

protected:
    ulong instr_clock;
    ulong max_instructions_per_tick;
    ushort instr_ptr_register;
    std::unique_ptr<ThreadPool<AsyncProgramJob>> job_pool;
    std::unique_ptr<ProgramExecutor> program_executor;
    std::string log_filename;
    std::string logger_name;
    AssemblyProgramBuilder program_builder;
};

TEST_F(ProgramExecutorTest, BasicConstruction) {
    SPDLOG_INFO("Starting BasicConstruction test");

    EXPECT_NE(program_executor, nullptr);
    EXPECT_EQ(instr_ptr_register, 0);
    EXPECT_EQ(program_executor->instr_trigger, 0);
    EXPECT_FALSE(program_executor->has_executed_async);
    EXPECT_FALSE(program_executor->has_executed_sync);

    SPDLOG_DEBUG("Basic construction test completed successfully");
}

TEST_F(ProgramExecutorTest, AsyncOnlyProgram) {
    SPDLOG_INFO("Starting AsyncOnlyProgram test");

    // Create program with only async instructions
    auto ops = program_builder
        .addNOP()
        .addINC(true)  // INC register A
        .addCOPY(true) // COPY A to B
        .addJMP(0)     // JMP to start
        .addNOP()
        .build();

    program_executor->_ops = std::move(ops);
    SPDLOG_DEBUG("Loaded async-only program with {} instructions", program_executor->_ops.size());

    // Execute async - should run up to max_instructions_per_tick
    program_executor->execute_async();
    waitForAsyncCompletion();

    SPDLOG_TRACE("After async execution, instr_ptr: {}", instr_ptr_register);

    // Should have executed multiple instructions (up to max limit)
    EXPECT_GT(instr_ptr_register, 0);
    EXPECT_LE(instr_ptr_register, max_instructions_per_tick);
    EXPECT_TRUE(program_executor->has_executed_async);

    SPDLOG_DEBUG("Async-only program test completed - executed {} instructions", instr_ptr_register);
}

TEST_F(ProgramExecutorTest, SyncInstructionStopsAsyncExecution) {
    SPDLOG_INFO("Starting SyncInstructionStopsAsyncExecution test");

    // Create program: async, async, sync, async
    auto ops = program_builder
        .addNOP()      // 0: async
        .addINC(true)  // 1: async
        .addMOVE()     // 2: sync - should stop here
        .addNOP()      // 3: async (shouldn't execute)
        .build();

    program_executor->_ops = std::move(ops);
    SPDLOG_DEBUG("Loaded mixed program with sync instruction at position 2");

    // Execute async - should stop at sync instruction
    program_executor->execute_async();
    waitForAsyncCompletion();

    SPDLOG_TRACE("After async execution, instr_ptr: {}", instr_ptr_register);

    // Should stop at position 2 (the MOVE instruction)
    EXPECT_EQ(instr_ptr_register, 2);
    EXPECT_TRUE(program_executor->has_executed_async);

    SPDLOG_DEBUG("Sync instruction correctly stopped async execution at position {}", instr_ptr_register);
}

TEST_F(ProgramExecutorTest, SyncExecutionAfterAsync) {
    SPDLOG_INFO("Starting SyncExecutionAfterAsync test");

    // Create program with sync instruction
    auto ops = program_builder
        .addMOVE()     // 0: sync instruction
        .addNOP()      // 1: async instruction
        .build();

    program_executor->_ops = std::move(ops);
    SPDLOG_DEBUG("Loaded program starting with sync instruction");

    // Execute async first - should prepare for sync
    program_executor->execute_async();
    waitForAsyncCompletion();

    SPDLOG_TRACE("After async execution, instr_ptr: {}", instr_ptr_register);
    EXPECT_EQ(instr_ptr_register, 0);  // Should be at sync instruction

    // Now execute sync
    program_executor->execute_sync();

    SPDLOG_TRACE("After sync execution, instr_ptr: {}", instr_ptr_register);

    // Should have advanced past the sync instruction
    EXPECT_EQ(instr_ptr_register, 1);
    EXPECT_TRUE(program_executor->has_executed_sync);

    SPDLOG_DEBUG("Sync execution successfully completed sync instruction");
}

TEST_F(ProgramExecutorTest, ClockTriggerDelaysExecution) {
    SPDLOG_INFO("Starting ClockTriggerDelaysExecution test");

    auto ops = program_builder
        .addNOP()
        .addNOP()
        .addNOP()
        .build();

    program_executor->_ops = std::move(ops);

    // Set instruction trigger to 2 (execute every 3rd clock tick: 0, 3, 6, ...)
    program_executor->instr_trigger = 2;

    // Clock 0: should not execute (0 % 3 != 0 when trigger is set)
    instr_clock = 0;
    program_executor->execute_async();
    waitForAsyncCompletion();
    EXPECT_EQ(instr_ptr_register, 0);
    SPDLOG_TRACE("Clock 0: instr_ptr = {} (expected no execution)", instr_ptr_register);

    // Clock 1: should not execute
    instr_clock = 1;
    program_executor->execute_async();
    waitForAsyncCompletion();
    EXPECT_EQ(instr_ptr_register, 0);
    SPDLOG_TRACE("Clock 1: instr_ptr = {} (expected no execution)", instr_ptr_register);

    // Reset trigger and test execution
    program_executor->instr_trigger = 0;
    instr_clock = 2;
    program_executor->execute_async();
    waitForAsyncCompletion();
    EXPECT_GT(instr_ptr_register, 0);
    SPDLOG_TRACE("Clock 2 with trigger reset: instr_ptr = {} (expected execution)", instr_ptr_register);

    SPDLOG_DEBUG("Clock trigger delay test completed successfully");
}

TEST_F(ProgramExecutorTest, ComplexAssemblyProgram) {
    SPDLOG_INFO("Starting ComplexAssemblyProgram test");

    // Create a more complex program simulating real ant behavior
    auto ops = program_builder
        .addLOAD(100, true)    // Load 100 into register A
        .addLOAD(50, false)    // Load 50 into register B
        .addADD(true)          // Add B to A (A = 150)
        .addCOPY(true)         // Copy A to B (B = 150)
        .addCHK()              // Check environment (sync)
        .addMOVE()             // Move forward (sync)
        .addDIG()              // Dig (sync)
        .addRT()               // Turn right
        .addNOP()              // No operation
        .build();

    program_executor->_ops = std::move(ops);
    SPDLOG_DEBUG("Loaded complex assembly program with {} instructions", program_executor->_ops.size());

    // Execute async in ticks - should stop at first sync instruction (CHK at position 4)
    const ushort expected_sync_index = 4;
    const ulong max_async_ticks = 20;  // Safety cap to avoid infinite loops
    for (ulong tick = 0;
         tick < max_async_ticks &&
         instr_ptr_register < program_executor->_ops.size() &&
         !program_executor->is_sync();
         ++tick) {
        program_executor->execute_async();
        waitForAsyncCompletion();
    }

    EXPECT_EQ(instr_ptr_register, expected_sync_index);  // Should stop at CHK
    SPDLOG_TRACE("Async execution stopped at CHK instruction (position {})", instr_ptr_register);

    // Execute the sync instruction
    program_executor->execute_sync();
    EXPECT_EQ(instr_ptr_register, 5);  // Should advance to MOVE
    SPDLOG_TRACE("After CHK sync execution, advanced to position {}", instr_ptr_register);

    // Execute sync again for MOVE
    program_executor->has_executed_sync = false;  // Reset sync flag
    program_executor->execute_sync();
    EXPECT_EQ(instr_ptr_register, 6);  // Should advance to DIG
    SPDLOG_TRACE("After MOVE sync execution, advanced to position {}", instr_ptr_register);

    SPDLOG_DEBUG("Complex assembly program test completed successfully");
}

TEST_F(ProgramExecutorTest, ProtobufSerializationRoundTrip) {
    SPDLOG_INFO("Starting ProtobufSerializationRoundTrip test");

    // Set some interesting state
    program_executor->instr_trigger = 42;
    program_executor->has_executed_sync = true;

    SPDLOG_DEBUG("Set initial state - instr_trigger: {}, has_executed_sync: {}",
                 program_executor->instr_trigger, program_executor->has_executed_sync);

    // Serialize to protobuf
    auto proto_msg = program_executor->get_proto();
    SPDLOG_TRACE("Serialized ProgramExecutor to protobuf");

    // Create new executor from protobuf with same dependencies
    ushort new_instr_ptr = 0;
    ProgramExecutor restored_executor(proto_msg, instr_clock, max_instructions_per_tick,
                                     new_instr_ptr, *job_pool);

    SPDLOG_TRACE("Restored ProgramExecutor from protobuf");

    // Verify state preservation
    EXPECT_EQ(restored_executor.instr_trigger, 42);
    EXPECT_EQ(restored_executor.has_executed_sync, true);
    EXPECT_EQ(&restored_executor.instr_ptr_register, &new_instr_ptr);
    EXPECT_EQ(&restored_executor.instr_clock, &instr_clock);
    EXPECT_EQ(restored_executor.max_instruction_per_tick, max_instructions_per_tick);

    SPDLOG_DEBUG("Protobuf serialization round-trip test completed successfully");
}

TEST_F(ProgramExecutorTest, BoundaryConditionsAndErrorHandling) {
    SPDLOG_INFO("Starting BoundaryConditionsAndErrorHandling test");

    // Test with empty program
    program_executor->_ops.clear();
    instr_ptr_register = 0;

    SPDLOG_DEBUG("Testing with empty program");
    program_executor->execute_async();
    waitForAsyncCompletion();

    // Should not crash or change instruction pointer
    EXPECT_EQ(instr_ptr_register, 0);
    SPDLOG_TRACE("Empty program handled correctly");

    // Test with instruction pointer beyond program size
    auto ops = program_builder.addNOP().addNOP().addNOP().build();
    program_executor->_ops = std::move(ops);
    instr_ptr_register = 10;  // Beyond program size

    SPDLOG_DEBUG("Testing with instr_ptr beyond program size (ptr: {}, size: {})",
                 instr_ptr_register, program_executor->_ops.size());

    program_executor->execute_async();
    waitForAsyncCompletion();

    // Should not crash or execute anything
    EXPECT_EQ(instr_ptr_register, 10);
    SPDLOG_TRACE("Out-of-bounds instruction pointer handled correctly");

    // Test sync execution without prior async
    instr_ptr_register = 0;
    program_executor->has_executed_async = false;
    program_executor->execute_sync();

    // Should not execute sync if async hasn't run
    EXPECT_EQ(instr_ptr_register, 0);
    SPDLOG_TRACE("Sync execution correctly requires prior async execution");

    SPDLOG_DEBUG("Boundary conditions and error handling test completed successfully");
}

TEST_F(ProgramExecutorTest, ResetFunctionality) {
    SPDLOG_INFO("Starting ResetFunctionality test");

    // Set some state
    program_executor->has_executed_sync = true;
    program_executor->instr_trigger = 5;

    SPDLOG_DEBUG("Set state before reset - has_executed_sync: {}, instr_trigger: {}",
                 program_executor->has_executed_sync, program_executor->instr_trigger);

    // Reset
    program_executor->reset();

    SPDLOG_TRACE("Called reset()");

    // Verify reset behavior
    EXPECT_FALSE(program_executor->has_executed_sync);
    // Note: instr_trigger is not reset by design (based on implementation)
    EXPECT_EQ(program_executor->instr_trigger, 5);

    SPDLOG_DEBUG("Reset functionality test completed successfully");
}
