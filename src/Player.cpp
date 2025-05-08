// Email: realyoavperetz@gmail.com
#include "Player.hpp"
#include "Game.hpp"

namespace coup {

// helpers
void Player::spend(int amount) {
    if (amount < 0) {
        COUP_THROW("Negative spend amount");
    }
    if (_coins < amount) {
        COUP_THROW("Not enough coins");
    }
    _coins -= amount;
}

void Player::gain(int amount) noexcept {
    _coins += amount;
}

// ctor
Player::Player(Game& game, const std::string& name)
    : _name(name), _game(game) {
    game.add_player(this);
}

// ───────── default actions ─────────

void Player::gather() {
    _game.validate_turn(this);
    if (_coins >= MANDATORY_COUP_LIMIT) {
        COUP_THROW("Must coup when holding 10 or more coins");
    }
    if (_game.bank() <= 0) {
        COUP_THROW("Bank is empty");
    }
    gain(1);
    _game.bank() -= 1;
    _game.next_turn();
}

void Player::tax() {
    _game.validate_turn(this);
    if (_coins >= MANDATORY_COUP_LIMIT) {
        COUP_THROW("Must coup when holding 10 or more coins");
    }
    if (_game.bank() < 2) {
        COUP_THROW("Bank has less than 2 coins");
    }
    _game.register_action(this, ActionType::Tax);
    gain(2);
    _game.bank() -= 2;
    _game.next_turn();
}

void Player::bribe() {
    _game.validate_turn(this);
    if (_coins >= MANDATORY_COUP_LIMIT) {
        COUP_THROW("Must coup when holding 10 or more coins");
    }
    spend(4);
    _game.bank() += 4;
    _game.register_action(this, ActionType::Bribe);
    // extra action not implemented
}

void Player::arrest(Player& target) {
    _game.validate_turn(this);
    if (&target == this) {
        COUP_THROW("Cannot arrest self");
    }
    if (_game.is_arrest_blocked(this)) {
        COUP_THROW("Arrest action is blocked for this turn");
    }
    if (target._coins <= 0) {
        COUP_THROW("Target has no coins to steal");
    }
    _game.register_action(this, ActionType::Arrest, &target);
    target._coins -= 1;
    gain(1);
    target.on_arrested(*this);
    _game.next_turn();
}

void Player::sanction(Player& target) {
    _game.validate_turn(this);
    spend(3);
    _game.bank() += 3;
    target.on_sanction(*this);
    _game.next_turn();
}

void Player::coup(Player& target) {
    _game.validate_turn(this);
    spend(7);
    _game.bank() += 7;
    _game.register_action(this, ActionType::Coup, &target);
    _game.eliminate(&target);
    _game.next_turn();
}

void Player::undo(Player&) {
    COUP_THROW("This role cannot undo actions");
}

} // namespace coup