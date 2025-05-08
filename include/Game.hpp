#pragma once
// Email: realyoavperetz@gmail.com
// Core game engine – manages players, turns, bank, action log, and special blocks (English-only comments).

#include <vector>
#include <string>
#include <cstddef>
#include <unordered_set>

#include "exceptions.hpp"
#include "Action.hpp"

namespace coup {

class Player; // forward declaration

class Game {
private:
    // Active players in join order
    std::vector<Player*>   _players;
    // Index of player whose turn it is now
    std::size_t            _turn_idx = 0;
    // Coins in the central bank
    int                    _bank     = 50;

    // Action log – tracks block‑able actions
    std::vector<ActionRecord> _log;

    // Set of players not allowed to use arrest on their next turn
    std::unordered_set<Player*> _arrest_blocked;

public:
    Game()                          = default;
    Game(const Game&)               = delete;
    Game& operator=(const Game&)    = delete;
    ~Game()                         = default;

    // ───────── Player management ─────────
    void add_player(Player* p);
    void eliminate(Player* p);

    // ───────── Turn control ─────────
    [[nodiscard]] std::string turn() const;
    [[nodiscard]] std::vector<std::string> players() const;
    void next_turn();
    void validate_turn(const Player* p) const;

    // ───────── Winner ─────────
    [[nodiscard]] std::string winner() const;

    // ───────── Bank access ─────────
    [[nodiscard]] int& bank() noexcept { return _bank; }

    // ───────── Action log helpers ─────────
    void register_action(Player* actor, ActionType type, Player* target = nullptr);
    ActionRecord* last_action(Player* actor, ActionType type);
    void prune_log();

    // ───────── Arrest‑block helpers ─────────
    void block_arrest(Player* target);
    bool is_arrest_blocked(Player* p) const;

    // ───────── Coup cancel helper ─────────
    /** Cancel the most recent coup against target and restore them to the game. */
    void cancel_coup(Player* target);
};

} // namespace coup
