
// Email: realyoavperetz@gmail.com

#pragma once
#include "Player.hpp"

namespace coup {

class Spy : public Player {
public:
    /** Construct a Spy and register it in the game. */
    Spy(Game& game, const std::string& name);
    
    /**
     * Inspect target player's coin count.
     * Does not count as a turn and costs 0 coins.
     * @return target's current number of coins.
     */
    int inspect(const Player& target) const noexcept;

    /**
     * Prevent target from using arrest on their next turn.
     * This helper currently marks a flag inside Game (TODO).
     */
    void block_arrest(Player& target);
};

}