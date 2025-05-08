// Email: realyoavperetz@gmail.com
#include "roles/Merchant.hpp"
#include "Game.hpp"
#include <algorithm>   // std::min

namespace coup {

Merchant::Merchant(Game& game, const std::string& name)
    : Player(game, name) {}

// start-of-turn bonus: +1 coin if holding ≥3 and bank has funds
void Merchant::start_of_turn() {
    if (_coins >= 3 && _game.bank() > 0) {
        gain(1);
        _game.bank() -= 1;
    }
}

// pay 2 coins to bank instead of losing 1 to thief
void Merchant::on_arrested(Player& thief) {
    // Revert the coin the thief just stole
    thief.spend(1);
    gain(1);

    // Pay up to 2 coins to the bank
    int pay = std::min(2, _coins);
    spend(pay);
    _game.bank() += pay;
}

} // namespace coup