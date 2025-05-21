// Email: realyoavperetz@gmail.com
// Implementation of Spy role.

#include "roles/Spy.hpp"
#include "Game.hpp"

namespace coup {

Spy::Spy(Game& game, const std::string& name) : Player(game, name) {}

int Spy::inspect(const Player& target) const noexcept {
    return target.coins();
}

void Spy::block_arrest(Player& target) {
    _game.block_arrest(&target);
}

}
