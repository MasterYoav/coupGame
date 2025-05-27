// Email: realyoavperetz@gmail.com
#include "roles/Merchant.hpp"
#include "Game.hpp"
#include <algorithm>   // std::min

namespace coup {

Merchant::Merchant(Game& game, const std::string& name)
    : Player(game, name) {_roleName = "Merchant";}

// start-of-turn bonus: +1 coin if holding ≥3 and bank has funds
void Merchant::start_of_turn() {
    Player::start_of_turn();

    // rule: if you begin turn with ≥3 coins, you get +1 free
    if (this->coins() >= 3) {
        // grant the coin
        gain(1);

        // record it as a Gather‐style action for the log
        _game.register_action(this, ActionType::Gather, nullptr, true);
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