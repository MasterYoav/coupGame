// Email: realyoavperetz@gmail.com
// Implementation of Game class.

#include "Game.hpp"
#include "Player.hpp"

#include <algorithm>

namespace coup {

// ───────────────── Player management ─────────────────

void Game::add_player(Player* p) {
    if (p == nullptr) {
        COUP_THROW("Null player pointer");
    }
    if (_players.size() >= 6) {
        COUP_THROW("Game already has 6 players");
    }
    if (std::find(_players.begin(), _players.end(), p) != _players.end()) {
        COUP_THROW("Player already in game");
    }
    _players.push_back(p);
}

void Game::eliminate(Player* p) {
    if (!p) return;
    auto it = std::find(_players.begin(), _players.end(), p);
    if (it == _players.end()) return;

    std::size_t removed = static_cast<std::size_t>(it - _players.begin());
    _players.erase(it);

    if (_players.empty()) {
        _turn_idx = 0;
        return;
    }
    // Shift turn index left if a player before it was removed
    if (removed < _turn_idx) {
        --_turn_idx;
    }
    if (_turn_idx >= _players.size()) {
        _turn_idx %= _players.size();
    }
}

// ───────────────── Turn API ─────────────────

std::string Game::turn() const {
    if (_players.empty()) {
        COUP_THROW("No active players");
    }
    return _players[_turn_idx]->name();
}

std::vector<std::string> Game::players() const {
    std::vector<std::string> names;
    names.reserve(_players.size());
    for (const Player* p : _players) {
        names.push_back(p->name());
    }
    return names;
}

void Game::next_turn() {
    if (_players.empty()) return;

    // remove arrest block on player that just finished their turn
    _arrest_blocked.erase(_players[_turn_idx]);

    prune_log();

    // advance
    _turn_idx = (_turn_idx + 1) % _players.size();

    // start-of-turn bonus for new current player
    _players[_turn_idx]->start_of_turn();
}

void Game::validate_turn(const Player* p) const {
    if (_players.empty()) {
        COUP_THROW("No players in game");
    }
    if (_players[_turn_idx] != p) {
        COUP_THROW("Not this player's turn");
    }
}

// ───────────────── Winner ─────────────────

std::string Game::winner() const {
    if (_players.size() != 1) {
        COUP_THROW("Game is still ongoing");
    }
    return _players.front()->name();
}

// ───────────────── Action log helpers ─────────────────

void Game::register_action(Player* actor,
                           ActionType type,
                           Player* target /* = nullptr */) {
    _log.push_back({actor, type, target, _turn_idx});
}

ActionRecord* Game::last_action(Player* actor, ActionType type) {
    for (auto it = _log.rbegin(); it != _log.rend(); ++it) {
        if (it->actor == actor && it->type == type) {
            return &*it;
        }
    }
    return nullptr;
}

void Game::prune_log() {
    if (_log.empty()) return;
    // Keep only actions within the last N turns (N = number of active players)
    _log.erase(std::remove_if(_log.begin(), _log.end(),
                              [&](const ActionRecord& rec) {
                                  std::size_t diff =
                                      (_turn_idx >= rec.turn_idx)
                                          ? (_turn_idx - rec.turn_idx)
                                          : (_players.size() + _turn_idx - rec.turn_idx);
                                  return diff > _players.size();
                              }),
               _log.end());
}

// ───────────────── Arrest-block helpers ─────────────────

void Game::block_arrest(Player* target) {
    if (target) {
        _arrest_blocked.insert(target);
    }
}

bool Game::is_arrest_blocked(Player* p) const {
    return _arrest_blocked.find(p) != _arrest_blocked.end();
}

// ───────────────── Coup cancel helper ─────────────────

void Game::cancel_coup(Player* target) {
    if (!target) {
        COUP_THROW("Null target for cancel_coup");
    }

    // Find most recent coup record with this target
    ActionRecord* rec = nullptr;
    for (auto it = _log.rbegin(); it != _log.rend(); ++it) {
        if (it->type == ActionType::Coup && it->target == target) {
            rec = &*it;
            break;
        }
    }
    if (!rec) {
        COUP_THROW("No coup to cancel for this target");
    }

    // Remove the record from log
    auto itDel = std::find_if(_log.begin(), _log.end(),
                              [&](const ActionRecord& r) { return &r == rec; });
    if (itDel != _log.end()) {
        _log.erase(itDel);
    }

    // If target is already active, nothing else to do
    if (std::find(_players.begin(), _players.end(), target) != _players.end()) {
        return;
    }

    // Restore target just before the current turn index
    _players.insert(_players.begin() + _turn_idx, target);
}

} // namespace coup