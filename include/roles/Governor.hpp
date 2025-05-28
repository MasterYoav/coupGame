
// Email: realyoavperetz@gmail.com
#pragma once
#include "Player.hpp"

namespace coup {

class Governor : public Player {
public:
    /**
     * Construct a Governor and register it inside the game.
     */
    Governor(Game& game, const std::string& name);
    
    
    /**
     * Collect 3 coins instead of 2.
     */
    void tax() override;

    /**
     * Undo (block) another player's tax action.
     * @throws CoupException if action_owner did not perform tax recently.
     */
    void undo(Player& action_owner) override;
    void block_tax(Player& target);
};
}
