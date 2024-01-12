#include <sstream>
#include "controller.hpp"



void WorkerController::Move::exec()
{
    int dx, dy;
    switch(dir) {
        case UP:
            dx = 0, dy = -1;
            break;
        case DOWN:
            dx = 0, dy = 1;
            break;
        case LEFT:
            dx = -1, dy = 0;
            break;
        case RIGHT:
            dx = 1, dy = 0;
            break;
    }
    move_ant(dx, dy);
}

WorkerController::WorkerController(move_ant_f move_ant, parse_error_f parse_error, std::vector<std::string>& program_code):
    operations(), operation_idx(), move_ant(move_ant), parse_error(parse_error)
{
    parse(program_code);
}

WorkerController::~WorkerController() {
    for( Op* o: operations) {
        delete o;
    }
}

void WorkerController::parse(std::vector<std::string>& program_code)
{
    bool empty_program = true;
    for (std::string& line: program_code) {
        std::istringstream word_stream(line);
        do {
            std::string word;
            word_stream >> word;

            if ( !word_stream ) continue; // no need to read an empty line
            empty_program = false;

            if ( word == "MOVE" ) {
                if ( word_stream ) {
                    word_stream >> word;
                    Move::Dir dir;
                    if        ( word == "UP" ) {
                        dir = Move::Dir::UP;
                    } else if ( word == "LEFT" ) {
                        dir = Move::Dir::LEFT;
                    } else if ( word == "DOWN" ) {
                        dir = Move::Dir::DOWN;
                    } else if ( word == "RIGHT" ) {
                        dir = Move::Dir::RIGHT;
                    } else {
                        parse_error("TODO: MAKE ERRORS MAKE SENSE");
                        return;
                    }
                    operations.push_back(new Move(dir, move_ant));

                    word_stream >> word;
                    if ( word_stream ) {
                        parse_error("TODO: MAKE ERRORS MAKE SENSE");
                        return;
                    }
                } else {
                    parse_error("TODO: MAKE ERRORS MAKE SENSE");
                    return;
                }
            } else if ( word_stream ) { // There's words here but they're not recognized 
                parse_error("TODO: MAKE ERRORS MAKE SENSE");
                return;
            }

        } while (word_stream);
    }
    if ( empty_program ) {
        parse_error("WILL NOT ADD AN ANT WITHOUT A PROGRAM");
        return;
    }
}


void WorkerController::handleClockPulse()
{
    operations[operation_idx]->exec();
    operation_idx == operations.size()-1? operation_idx = 0: operation_idx++;
}



