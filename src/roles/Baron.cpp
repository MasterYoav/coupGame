// Email: realyoavperetz@gmail.com
#include "roles/Baron.hpp"
#include "Game.hpp"

namespace coup {

Baron::Baron(Game& game, const std::string& name)
    : Player(game, name) {}

void Baron::invest() {
    _game.validate_turn(this);
    spend(3);
    gain(6);
    _game.next_turn();
}

void Baron::on_sanction(Player&) {
    gain(1);   // compensation coin
}

} // namespace coup