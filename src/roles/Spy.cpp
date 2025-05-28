// Email: realyoavperetz@gmail.com


#include "roles/Spy.hpp"
#include "Game.hpp"

namespace coup {

Spy::Spy(Game& game, const std::string& name) : Player(game, name) {_roleName = "Spy";}

int Spy::inspect(const Player& target) const noexcept {
    return target.coins();
}

void Spy::block_arrest(Player& target) {
    _game.block_arrest(&target);
}

}
