#pragma once
// Email: realyoavperetz@gmail.com
// Governor role – extends Player. Gains +3 coins on tax and can undo tax actions.

#include "Player.hpp"

namespace coup {

class Governor : public Player {
public:
    /**
     * Construct a Governor and register it inside the game.
     */
    Governor(Game& game, const std::string& name);
    
    std::string role() const override { return "Governor"; }
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
