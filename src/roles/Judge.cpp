// Email: realyoavperetz@gmail.com
#include "roles/Judge.hpp"
#include "Game.hpp"

namespace coup {

Judge::Judge(Game& game, const std::string& name)
    : Player(game, name) {}

void Judge::undo(Player& action_owner) {
    // look for most recent bribe by action_owner
    auto rec = _game.last_action(&action_owner, ActionType::Bribe);
    if (!rec) {
        COUP_THROW("No bribe action to undo");
    }
    // simply erase the action so extra move won't occur
    _game.prune_log();
}

void Judge::on_sanction(Player& attacker) {
    attacker.spend(1);
    _game.bank() += 1;
}

} // namespace coup