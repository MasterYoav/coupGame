#pragma once
// Email: realyoavperetz@gmail.com
// General role – can pay 5 coins to block a coup and refunds 1 coin if arrested.

#include "Player.hpp"

namespace coup {

class General : public Player {
public:
    /** Construct a General and register it in the game. */
    General(Game& game, const std::string& name);

    std::string role() const override { return "General"; }

    /**
     * Pay 5 coins to defend a player (including self) from a coup.
     * If used successfully, the attacker loses the 7 coins spent.
     */
    void defend_coup(Player& target);

    /** Refund 1 coin when this General is arrested. */
    void on_arrest_refund();
};

}