// Email: realyoavperetz@gmail.com
// Implementation of Governor role.

#include "roles/Governor.hpp"
#include "Game.hpp"

namespace coup {

Governor::Governor(Game& game, const std::string& name) : Player(game, name) {}

void Governor::tax() {
    _game.validate_turn(this);
    if (_coins >= MANDATORY_COUP_LIMIT) {
        COUP_THROW("Must coup when holding 10 or more coins");
    }
    _game.register_action(this, ActionType::Tax);          
    gain(3);
    _game.bank() -= 3;
    _game.register_action(this, ActionType::Tax, nullptr, true);
    _game.next_turn();
}

void Governor::undo(Player& action_owner) {
   
    auto rec = _game.last_action(&action_owner, ActionType::Tax);
    if (!rec) {
        COUP_THROW("No tax action to undo");
    }

    action_owner.spend(2);
    _game.bank() += 2;
}
void Governor::block_tax(Player& target) {
    _game.validate_turn(this);
    if (_game.is_tax_blocked(&target)) {
        COUP_THROW("Already blocked tax for " + target.name());
    }
    // Register the block
    _game.block_tax(&target);
    _game.register_action(this, ActionType::TaxCancel, &target, true);
    // Advance turn
    _game.next_turn();
}
} // namespace coup