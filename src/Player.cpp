// Email: realyoavperetz@gmail.com
// Player.cpp – Implementation of Player with mandatory-coup in gather/tax/bribe,
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
    try {
        _game.validate_turn(this);
        if (_coins >= MANDATORY_COUP_LIMIT) {
            COUP_THROW("Must coup when holding 10 or more coins");
        }
        if (_game.is_sanctioned(this)) {
            COUP_THROW("Gather action is blocked by sanction");
        }
        gain(1);
        _game.register_action(this, ActionType::Gather, nullptr, true);
        if (_extraActionAllowed) {
            _extraActionAllowed = false;
        } else {
            _game.next_turn();
        }
    } catch (const CoupException&) {
        _game.register_action(this, ActionType::Gather, nullptr, false);
        throw;
    }
}

void Player::tax() {
    try {
        _game.validate_turn(this);
        if (_coins >= MANDATORY_COUP_LIMIT) {
            COUP_THROW("Must coup when holding 10 or more coins");
        }
        if (_game.is_tax_blocked(this)) {
            COUP_THROW("Tax action is blocked by Governor");
        }
        if (_game.is_sanctioned(this)) {
            
            COUP_THROW("Tax action is blocked by sanction");
        }
        gain(2);
        _game.register_action(this, ActionType::Tax, nullptr, true);
        if (_extraActionAllowed) {
            _extraActionAllowed = false;
        } else {
            _game.next_turn();
        }
    } catch (const CoupException&) {
        _game.register_action(this, ActionType::Tax, nullptr, false);
        throw;
    }
}

void Player::bribe() {
    try {
        _game.validate_turn(this);
        if (_coins >= MANDATORY_COUP_LIMIT) {
            COUP_THROW("Must coup when holding 10 or more coins");
        }
        if (_game.is_bribe_blocked(this)) {
            COUP_THROW("Can't bribe – you are blocked");
        }
        spend(4);
        _game.bank() += 4;
        _extraActionAllowed = true;
        _game.register_action(this, ActionType::Bribe, nullptr, true);
        // stay on the same turn (extra‐action)
    } catch (const CoupException&) {
        _game.register_action(this, ActionType::Bribe, nullptr, false);
        throw;
    }
}

void Player::arrest(Player& target) {
    try {
        _game.validate_turn(this);
        // no mandatory-coup here
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
        _game.register_action(this, ActionType::Arrest, &target, true);
        target.spend(1);
        gain(1);
        target.on_arrested(*this);
        _lastArrestTarget = &target;
        if (_extraActionAllowed) {
            _extraActionAllowed = false;
        } else {
            _game.next_turn();
        }
    } catch (const CoupException&) {
        _game.register_action(this, ActionType::Arrest, &target, false);
        throw;
    }
}

void Player::sanction(Player& target) {
    try {
        _game.validate_turn(this);
        // no mandatory-coup here
        // cost 3
        spend(3);
        _game.bank() += 3;
        // extra 1 if target is Judge
        if (target.role() == "Judge") {
            spend(1);
            _game.bank() += 1;
        }
        target.on_sanction(*this);
        _game.register_action(this, ActionType::Sanction, &target, true);
        _game.block_sanction(&target);
        if (_extraActionAllowed) {
            _extraActionAllowed = false;
        } else {
            _game.next_turn();
        }
    } catch (const CoupException&) {
        _game.register_action(this, ActionType::Sanction, &target, false);
        throw;
    }
}

void Player::coup(Player& target) {
    _game.validate_turn(this);
    // coup cost is 7, no mandatory-coup check here (this *is* the coup)
    spend(7);
    _game.register_action(this, ActionType::Coup, &target, true);
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