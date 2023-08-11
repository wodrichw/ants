#include <libtcod.hpp>
#include <iostream>
#include <libtcod/color.hpp>
#include "ant.hpp"


namespace ant {


    Ant::Ant(int x, int y, int fovRadius, char ch, tcod::ColorRGB col):
        x(x), y(y), fovRadius(fovRadius), ch(ch), col(col)
    {}

    // void Player::Renderer::operator()(Ant& a) 
    // {
    //     TCODConsole::root->setChar(a.x, a.y, a.ch);
    //     TCODConsole::root->setCharForeground(a.x, a.y, a.col);
    // }


    // Player::Player(int startx, int starty, char ch, TCODColor& col):
    //     ap(), u(), r(), Ant(startx, starty, ch, col,  ap, u, r)
    // {}

    Player::Player(int x, int y, int fovRadius, char ch, tcod::ColorRGB col): Ant(x, y, fovRadius, ch, col) {}

    void Player::updatePositionByDelta(int dx, int dy)
    {
        x += dx;
        y += dy;
    }

    void Player::render() 
    {
        TCODConsole::root->setChar(x,y,ch);
        TCODConsole::root->setCharForeground(x,y,col);
    }
}

/*
Actor::Actor(int x, int y, int ch, std::string name, const TCODColor &col) :
    x(x),y(y),ch(ch), name(name), col(col) {
    }

void Actor::update()
{
    std::cout<< "The " << name << " growls!" << std::endl;
}

bool Actor::moveOrAttack(int x, int y)
{
    if ( engine.map->isWall(x,y) ) return false;
    for (Actor **iterator=engine.actors.begin();
            iterator != engine.actors.end(); iterator++) {
        Actor *actor=*iterator;
        if ( actor->x == x && actor->y ==y ) {
            std::cout << "The " << name << 
    char ch;
    TCODColor& col;
                "  laughs at your puny efforts to attack him!" << std::endl;
            return false;
        }
    }
    this->x=x;
    this->y=y;
    return true;
}

void Actor::render() const {
    TCODConsole::root->setChar(x,y,ch);
    TCODConsole::root->setCharForeground(x,y,col);
}
*/

