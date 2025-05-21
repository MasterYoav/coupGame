// Email: realyoavperetz@gmail.com
#include "roles/Baron.hpp"
#include "Game.hpp"

namespace coup {

Baron::Baron(Game& game, const std::string& name)
    : Player(game, name) {}

    void Baron::invest() {
        _game.validate_turn(this);
        if (_coins < 3) {
            COUP_THROW("Not enough coins to invest");
        }
        // Pay 3 into the bank
        spend(3);
        _game.bank() += 3;
        // Gain 6 from the bank
        gain(6);
        _game.bank() -= 6;
    
        // Record & advance
        _game.register_action(this, ActionType::Invest, nullptr, true);
        _game.next_turn();
    }

void Baron::on_sanction(Player&) {
    gain(1);   // compensation coin
}

} // namespace coup