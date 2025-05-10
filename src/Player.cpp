// Email: realyoavperetz@gmail.com
// Player.cpp – Implementation of Player with mandatory coup check,
// bribe extra-action, no-repeat arrest, and sanction-block.

#include "Player.hpp"
#include "Game.hpp"

namespace coup {

// Spend coins, throws if insufficient
void Player::spend(int amount) {
    if (amount < 0) {
        COUP_THROW("Negative spend amount");
    }
    if (_coins < amount) {
        COUP_THROW("Not enough coins");
    }
    _coins -= amount;
}

// Gain coins (no limits)
void Player::gain(int amount) noexcept {
    _coins += amount;
}

// Constructor: register player with game
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
    if (_game.is_sanctioned(this)) {
        COUP_THROW("Gather action is blocked by sanction");
    }
    gain(1);
    // consume extra-action or advance turn
    if (_extraActionAllowed) {
        _extraActionAllowed = false;
    } else {
        _game.next_turn();
    }
}

void Player::tax() {
    _game.validate_turn(this);
    if (_coins >= MANDATORY_COUP_LIMIT) {
        COUP_THROW("Must coup when holding 10 or more coins");
    }
    if (_game.is_sanctioned(this)) {
        COUP_THROW("Tax action is blocked by sanction");
    }
    _game.register_action(this, ActionType::Tax);
    gain(2);
    if (_extraActionAllowed) {
        _extraActionAllowed = false;
    } else {
        _game.next_turn();
    }
}

void Player::bribe() {
    _game.validate_turn(this);
    if (_coins >= MANDATORY_COUP_LIMIT) {
        COUP_THROW("Must coup when holding 10 or more coins");
    }
    spend(4);
    _game.register_action(this, ActionType::Bribe);
    _extraActionAllowed = true;
    // Note: turn advances on the next action
}

void Player::arrest(Player& target) {
    _game.validate_turn(this);
    if (_coins >= MANDATORY_COUP_LIMIT) {
        COUP_THROW("Must coup when holding 10 or more coins");
    }
    if (&target == this) {
        COUP_THROW("Cannot arrest self");
    }
    if (_game.is_arrest_blocked(this)) {
        COUP_THROW("Arrest action is blocked for this turn");
    }
    if (_lastArrestTarget == &target) {
        COUP_THROW("Cannot arrest same target twice in a row");
    }
    if (target.coins() <= 0) {
        COUP_THROW("Target has no coins to steal");
    }
    _game.register_action(this, ActionType::Arrest, &target);
    target.spend(1);
    gain(1);
    target.on_arrested(*this);
    _lastArrestTarget = &target;
    if (_extraActionAllowed) {
        _extraActionAllowed = false;
    } else {
        _game.next_turn();
    }
}

void Player::sanction(Player& target) {
    _game.validate_turn(this);
    if (_coins >= MANDATORY_COUP_LIMIT) {
        COUP_THROW("Must coup when holding 10 or more coins");
    }
    spend(3);
    _game.register_action(this, ActionType::Sanction, &target);
    _game.block_sanction(&target);
    target.on_sanction(*this);
    if (_extraActionAllowed) {
        _extraActionAllowed = false;
    } else {
        _game.next_turn();
    }
}

void Player::coup(Player& target) {
    _game.validate_turn(this);
    spend(7);
    _game.register_action(this, ActionType::Coup, &target);
    _game.eliminate(&target);
    if (_extraActionAllowed) {
        _extraActionAllowed = false;
    } else {
        _game.next_turn();
    }
}

void Player::undo(Player&) {
    COUP_THROW("This role cannot undo actions");
}

} // namespace coup