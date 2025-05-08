// Email: realyoavperetz@gmail.com
// Implementation of General role – can pay 5 coins to cancel a coup and
// gets 1-coin refund when arrested.

#include "roles/General.hpp"
#include "Game.hpp"

namespace coup {

General::General(Game& game, const std::string& name) : Player(game, name) {}

void General::defend_coup(Player& target) {
    /*
     * Pay 5 coins to cancel (block) the most recent coup against `target`.
     * The attacker still loses the 7 coins they spent; the target is restored
     * to the active players list (via Game::cancel_coup).
     *
     * This ability can be invoked off-turn, so we do NOT call validate_turn().
     */
    spend(5);
    _game.bank() += 5;

    _game.cancel_coup(&target);
}

void General::on_arrest_refund() {
    // Refund the single coin that was stolen by arrest.
    gain(1);
}

} // namespace coup