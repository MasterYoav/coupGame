#pragma once
// Email: realyoavperetz@gmail.com
// Base class for every role in the Coup game.

#include <string>
#include "exceptions.hpp"
#include "Action.hpp"

namespace coup {

class Game;  // forward declaration

class Player {
protected:
    std::string _name;     // unique player name
    int         _coins = 0;
    Game&       _game;

    void gain(int amount) noexcept;  // add coins

    static constexpr int MANDATORY_COUP_LIMIT = 10;

public:
    Player(Game& game, const std::string& name);
    virtual ~Player() = default;

    // Accessible to all roles (needed to deduct coins from others)
    void spend(int amount);          // throws if insufficient coins

    // ───────── public actions ─────────
    virtual void gather();
    virtual void tax();
    virtual void bribe();
    virtual void arrest(Player& target);
    virtual void sanction(Player& target);
    virtual void coup(Player& target);

    // Attempt to block another player's action
    virtual void undo(Player& action_owner);

    // ───────── reaction hooks ─────────
    virtual void on_sanction([[maybe_unused]] Player& attacker) {}
    virtual void on_arrested([[maybe_unused]] Player& thief)   {}
    virtual void start_of_turn()                               {}

    // ───────── info accessors ─────────
    const std::string& name() const noexcept { return _name; }
    int coins() const noexcept { return _coins; }
};

} // namespace coup