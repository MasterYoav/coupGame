// Email: realyoavperetz@gmail.com
#pragma once


#include "Player.hpp"

namespace coup {

class General : public Player {
public:
    /** Construct a General and register it in the game. */
    General(Game& game, const std::string& name);

    

    /**
     * Pay 5 coins to defend a player (including self) from a coup.
     * If used successfully, the attacker loses the 7 coins spent.
     */
    void block_coup(Player& target);

    /** Refund 1 coin when this General is arrested. */
    void on_arrest_refund();
};

}