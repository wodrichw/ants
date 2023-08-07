#ifndef __ACTOR_HPP
#define __ACTOR_HPP

#include <libtcod.hpp>
#include <string>


class Actor { 
public :
    int x,y; // position on map
    int ch; // ascii code
    std::string name; // the actor's name
    TCODColor col; // color
    bool blocks; // can we walk on this actor?
    // Attacker *attacker; // something that deals damage
    // Destructible *destructible; // something that can be damaged
    // Ai *ai; // something self-updating

    Actor(int x, int y, int ch, std::string name, const TCODColor &col);
    void update();
    bool moveOrAttack(int x,int y);
    void render() const;
};

class Attacker {
    public :
    float power; // hit points given

    Attacker(float power);
    void attack(Actor *owner, Actor *target);
};

#endif //__ACTOR_HPP
