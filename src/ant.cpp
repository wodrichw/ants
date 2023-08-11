#include <libtcod.hpp>
#include <iostream>
#include <libtcod/color.hpp>
#include "engine.hpp"


namespace ant {


    Ant::Ant(int x, int y, char ch, TCODColor& col,
            std::function<void(Ant&)> readEnvironment_internal, std::function<void(Ant&)> performAction_internal,
            std::function<void(Ant&)> update_internal, std::function<void(Ant&)> render_internal
    ):
        x(x), y(y), ch(ch), col(col),
        readEnvironment_internal(readEnvironment_internal),
        performAction_internal(performAction_internal),
        update_internal(update_internal),
        render_internal(render_internal)
    {}

    Player::EnvironmentReader::EnvironmentReader(Map& map): 
        map(map)
    {}

    void Player::EnvironmentReader::operator()(Ant& a) 
    {
    }

    Player::ActionPerformer::ActionPerformer()
    {}

    void Player::ActionPerformer::operator()(Ant& a) 
    {
    }

    Player::Updater::Updater()
    {}

    void Player::Updater::operator()(Ant& a) 
    {
    }

    Player::Renderer::Renderer()
    {}

    void Player::Renderer::operator()(Ant& a) 
    {
        TCODConsole::root->setChar(a.x, a.y, a.ch);
        TCODConsole::root->setCharForeground(a.x, a.y, a.col);
    }


    Player::Player(Map& map, int startx, int starty, char ch, TCODColor& col):
        er(map), ap(), u(), r(), Ant(startx, starty, ch, col, er, ap, u, r)
    {}

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

