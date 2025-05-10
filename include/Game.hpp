// File: include/Game.hpp
#pragma once
// Email: realyoavperetz@gmail.com

#include <vector>
#include <string>
#include <cstddef>
#include <unordered_set>

#include "exceptions.hpp"
#include "Action.hpp"       // defines ActionRecord, ActionType
class Player;              // forward declaration

namespace coup {

class Game {
private:
    std::vector<Player*>        _players;
    std::size_t                 _turn_idx = 0;
    int                         _bank     = 0;

    std::vector<ActionRecord>   _log;
    std::unordered_set<Player*> _arrest_blocked;
    std::unordered_set<Player*> _sanction_blocked;

public:
    Game()                       = default;
    Game(const Game&)            = delete;
    Game& operator=(const Game&) = delete;
    ~Game()                      = default;

    // Player management
    void add_player(Player* p);
    void eliminate(Player* p);

    // Turn control
    [[nodiscard]] std::string turn() const;
    [[nodiscard]] std::vector<std::string> players() const;
    void next_turn();
    void validate_turn(const Player* p) const;
    // In include/Game.hpp, under Turn control:
    Player* current_player() const { return _players.empty() ? nullptr : _players[_turn_idx]; }

    // Winner
    [[nodiscard]] std::string winner() const;

    // Bank
    [[nodiscard]] int& bank() noexcept { return _bank; }

    // Log
    void register_action(Player* actor, ActionType type, Player* target = nullptr);
    ActionRecord* last_action(Player* actor, ActionType type);
    void prune_log();

    // Blocks
    void block_arrest(Player* target);
    bool is_arrest_blocked(Player* p) const;

    void block_sanction(Player* target);
    bool is_sanctioned(Player* p) const;

    // Coup undo
    void cancel_coup(Player* target);
};

} // namespace coup