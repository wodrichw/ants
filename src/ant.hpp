#ifndef __ANT_HPP
#define __ANT_HPP

#include <cwchar>
#include <libtcod.hpp>
#include <libtcod/color.hpp>
#include <string>
#include "map.hpp"


namespace ant {

class Ant { 
public :
    int x, y;
    char ch;
    TCODColor& col;
    virtual void readEnvironment() { readEnvironment_internal(*this); }
    virtual void performAction() { performAction_internal(*this); }
    virtual void update() { update_internal(*this); }
    virtual void render() { render_internal(*this); }

    Ant(int x, int y, char ch, TCODColor& col,
        std::function<void(Ant&)> readEnvironment_internal,
        std::function<void(Ant&)> performAction_internal,
        std::function<void(Ant&)> update_internal,
        std::function<void(Ant&)> render_internal
    );

private:
    std::function<void(Ant&)> readEnvironment_internal;
    std::function<void(Ant&)> performAction_internal;
    std::function<void(Ant&)> update_internal;
    std::function<void(Ant&)> render_internal;
};


class Player :public Ant {

    struct EnvironmentReader {
        Map& map;
        EnvironmentReader(Map& map); 
        void operator()(Ant& p);
    } er;

    struct  ActionPerformer{
        ActionPerformer();
        void operator()(Ant& p);
    } ap;

    struct Updater {
        Updater();
        void operator()(Ant& p);
    } u;

    struct Renderer {
        Renderer();
        void operator()(Ant& p);
    } r;

public:
    Player(Map& map, int startx, int starty, char ch, TCODColor& col);
};


std::function<void(Ant&)> x = [](Person& p) { printf( "HI") ; };
class dummy {
    int x,y; // position on map
    int ch; // ascii code
    std::string name; // the actor's name
    TCODColor col; // color
    bool blocks; // can we walk on this actor?
    // Attacker *attacker; // something that deals damage
    // Destructible *destructible; // something that can be damaged
    // Ai *ai; // something self-updating

    dummy(int x, int y, int ch, std::string name, const TCODColor &col);
    void update();
    bool moveOrAttack(int x,int y);
    void render() const;
};

}

#endif //__ANT_HPP
