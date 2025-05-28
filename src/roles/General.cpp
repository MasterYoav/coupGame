// Email: realyoavperetz@gmail.com


#include "roles/General.hpp"
#include "Game.hpp"

namespace coup {

General::General(Game& game, const std::string& name) : Player(game, name) {_roleName = "General";}

void General::block_coup(Player& target) {
    
    if (_coins < 5) {
        COUP_THROW("Not enough coins to block a coup");
    }
    // pay 5 to the bank
    spend(5);
    // undo the coup (restores target & returns their card)
    _game.cancel_coup(&target);

    // record & advance
    _game.register_action(this, ActionType::BlockCoup, &target, true);
    _game.next_turn();
}

void General::on_arrest_refund() {
    // Refund the single coin that was stolen by arrest.
    gain(1);
}

} // namespace coup