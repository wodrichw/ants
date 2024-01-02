#ifndef __CONTROLLER_HPP__
#define __CONTROLLER_HPP__

#include <vector>
#include <string>
#include <functional>



class Controller {
public:
    virtual void handleClockPulse() = 0;
    virtual ~Controller() = default;
};



class  Worker_Controller: public Controller {
public:
    typedef std::function<void(int dx, int dy)>  move_ant_f;
    typedef std::function<void(std::string error_msg)>  parse_error_f;
    struct Brain {
        int acc_reg;
        int bak_reg;
    };

    Brain* brain;

    struct Op {
        virtual ~Op() = default;
        virtual void exec() = 0;
    };

    struct Move: public Op {
        enum Dir {UP, DOWN, LEFT, RIGHT} dir;
        move_ant_f& move_ant;

        Move(Dir dir, move_ant_f& move_ant): 
            dir(dir), move_ant(move_ant)
        {}

        void exec();
    };

    std::vector<Op*> operations;
    size_t operation_idx; // essentially program counter
    move_ant_f move_ant;
    parse_error_f parse_error;
    
    Worker_Controller(move_ant_f move_ant, parse_error_f parse_error, std::vector<std::string>& program_code);
    ~Worker_Controller();

    void parse(std::vector<std::string>& program_code);
    void handleClockPulse() override;
};

#endif // __CONTROLLER_HPP__
