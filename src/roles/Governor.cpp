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
    if (_game.bank() < 3) {
        COUP_THROW("Bank has less than 3 coins");
    }

    _game.register_action(this, ActionType::Tax);          // ← רישום בלוג
    gain(3);
    _game.bank() -= 3;
    _game.next_turn();
}

void Governor::undo(Player& action_owner) {
    // ביטול מס: מחפש פעולה אחרונה מסוג Tax של action_owner
    auto rec = _game.last_action(&action_owner, ActionType::Tax);
    if (!rec) {
        COUP_THROW("No tax action to undo");
    }

    action_owner.spend(2);
    _game.bank() += 2;
}

} // namespace coup