// Email: realyoavperetz@gmail.com
#include "roles/Judge.hpp"
#include "Game.hpp"

namespace coup {

Judge::Judge(Game& game, const std::string& name)
    : Player(game, name) {_roleName = "Judge";}

    void Judge::cancel_bribe(Player& target) {
        _game.validate_turn(this);
        if (_game.is_bribe_blocked(&target)) {
            COUP_THROW("Already blocked bribe for " + target.name());
        }
        // Register the block
        _game.block_bribe(&target);
        // Log it
        _game.register_action(this, ActionType::BribeCancel, &target, true);
        // Advance turn
        _game.next_turn();
    }

void Judge::on_sanction(Player& attacker) {
    attacker.spend(1);
    _game.bank() += 1;
}

} // namespace coup