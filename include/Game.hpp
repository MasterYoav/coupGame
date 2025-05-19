// Updated Game.hpp with action-log storage and API
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

    // existing blocking log
    std::vector<ActionRecord>   _log;
    // new formatted string log
    std::vector<std::string>    _actionLogStrings;

    std::unordered_set<Player*> _arrest_blocked;
    std::unordered_set<Player*> _sanction_blocked;
    std::unordered_set<Player*> _tax_blocked;
public:
    Game()                       = default;
    Game(const Game&)            = delete;
    Game& operator=(const Game&) = delete;
    ~Game()                      = default;

    // Player management
    void add_player(Player* p);
    void eliminate(Player* p);
    [[nodiscard]] const std::vector<Player*>& playerObjects() const noexcept {return _players;}

    // Turn control
    [[nodiscard]] std::string turn() const;
    [[nodiscard]] std::vector<std::string> players() const;
    void next_turn();
    void validate_turn(const Player* p) const;
    Player* current_player() const { return _players.empty() ? nullptr : _players[_turn_idx]; }

    // Winner
    [[nodiscard]] std::string winner() const;

    // Bank
    [[nodiscard]] int& bank() noexcept { return _bank; }

    // Action log
    /**
     * Registers an action for internal logic and formatted log.
     * @param actor   the player performing the action
     * @param type    enum ActionType
     * @param target  optional target of action
     * @param success whether the action succeeded
     */
    void register_action(Player* actor,
                         ActionType type,
                         Player* target = nullptr,
                         bool success = true);
                         
    void block_tax(Player* target);
    bool is_tax_blocked(Player* p) const noexcept;

    /**
     * Returns the formatted log entries:
     * "playerName,Action,Succeeded" or "playerName,Action,Failed".
     */
    [[nodiscard]] std::vector<std::string> getActionLog() const noexcept {
        return _actionLogStrings;
    }

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
