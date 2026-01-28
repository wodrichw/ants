# HardwareParserTests: invalid programs throw or bypass errors

## Summary
Parser error handling does not consistently set Status::p_err and sometimes throws exceptions (stoi, _Map_base::at) instead of reporting parse errors for invalid programs and labels.

## Failed Tests
- HardwareParserTests
  - ParserInvalidPrograms/ParserInvalidProgramTest.* (multiple cases)

## Evidence
- std::invalid_argument/std::out_of_range thrown from stoi during LOAD parsing.
- std::out_of_range("_Map_base::at") when resolving missing jump labels.
- Invalid register tokens only log errors but still parse as successful (Status::p_err remains false).

## Suspected Areas
- src/core/hardware/command_parsers.cpp
  - LoadConstantParser
  - JumpParser
- src/core/hardware/token_parser.cpp
  - TokenParser::integer
  - TokenParser::letter_idx

## Suggested Fix Direction
- Guard all stoi calls and convert to Status errors.
- Detect invalid register tokens and propagate Status::p_err.
- Handle missing labels in JumpParser without throwing.

## Impact
Invalid programs can crash parsing or appear successful with corrupted bytecode.
