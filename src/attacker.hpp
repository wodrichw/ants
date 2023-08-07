#ifndef __ATTACKOR_HPP
#define __ATTACKOR_HPP

class Actor;

class Attacker {
    public :
    float power; // hit points given

    Attacker(float power);
    void attack(Actor *owner, Actor *target);
};


#endif // __ATTACKOR__HPP
