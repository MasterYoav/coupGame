// Email: realyoavperetz@gmail.com


#include "Game.hpp"
#include "Player.hpp"

#include <algorithm>
#include <sstream>

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

    // remove one‐time blocks on the player who just finished a turn
    _arrest_blocked.erase(_players[_turn_idx]);
    _tax_blocked.erase(_players[_turn_idx]);
    _bribe_blocked.erase(_players[_turn_idx]);
    _sanction_blocked.erase(_players[_turn_idx]);
    prune_log();
    _turn_idx = (_turn_idx + 1) % _players.size();
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

std::string Game::winner() const {
    if (_players.size() != 1) {
        COUP_THROW("Game is still ongoing");
    }
    return _players.front()->name();
}
// ─── Tax-block helpers ─────────────────────────────
void Game::block_tax(Player* target) {
        if (target) _tax_blocked.insert(target);
    }
    bool Game::is_tax_blocked(Player* p) const noexcept {
        return _tax_blocked.count(p) != 0;
    }
// ───────────────── Action log helpers ─────────────────

// helper to stringify ActionType
static std::string actionTypeToString(ActionType type) {
    switch (type) {
        case ActionType::Gather:   return "Gather";
        case ActionType::Tax:      return "Tax";
        case ActionType::Bribe:    return "Bribe";
        case ActionType::Arrest:   return "Arrest";
        case ActionType::Sanction: return "Sanction";
        case ActionType::TaxCancel: return "BlockedTax";
        case ActionType::Coup:     return "Coup";
        case ActionType::Invest:    return "Invest";
        case ActionType::BlockCoup: return "BlockCoup";
        case ActionType::BribeCancel:   return "BlockBribe";
        default:                    return "";
    }
    return "";
}

void Game::register_action(Player* actor,
                           ActionType type,
                           Player* target,
                           bool success) {
    // 1. internal record for blocking logic
    // 1) record for blocking logic
    _log.push_back({actor, type, target, _turn_idx});

    // 2) formatted string for the log window
    std::ostringstream out;
    out << actor->name()           // who acted
        << "," << actionTypeToString(type);  // what they did
    if (target) {
        out << " for " << target->name();     // optional target
    }
    out << "," << (success ? "Succeeded" : "Failed");
    _actionLogStrings.push_back(out.str());
}

// new: last_action definition to satisfy Governor/Judge undo
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
    _log.erase(
        std::remove_if(
            _log.begin(), _log.end(),
            [&](const ActionRecord& rec) {
                std::size_t diff =
                    (_turn_idx >= rec.turn_idx)
                        ? (_turn_idx - rec.turn_idx)
                        : (_players.size() + _turn_idx - rec.turn_idx);
                return diff > _players.size();
            }
        ),
        _log.end()
    );
}

void Game::block_arrest(Player* target) {
    if (target) {
        _arrest_blocked.insert(target);
    }
}

bool Game::is_arrest_blocked(Player* p) const {
    return _arrest_blocked.count(p) != 0;
}

void Game::block_sanction(Player* target) {
    if (target) {
        _sanction_blocked.insert(target);
    }
}

bool Game::is_sanctioned(Player* p) const {
    return _sanction_blocked.count(p) != 0;
}


void Game::cancel_coup(Player* target) {
    if (!target) {
        COUP_THROW("Null target for cancel_coup");
    }

    // Find most recent coup record for this target
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

    // Remove the record
    auto itDel = std::find_if(
        _log.begin(), _log.end(),
        [&](const ActionRecord& r) { return &r == rec; }
    );
    if (itDel != _log.end()) {
        _log.erase(itDel);
    }

    // Restore the player if they were removed
    if (std::find(_players.begin(), _players.end(), target) == _players.end()) {
        _players.insert(_players.begin() + _turn_idx, target);
    }
    }
    void Game::block_bribe(Player* target) {
    if (target) _bribe_blocked.insert(target);
    }
    bool Game::is_bribe_blocked(Player* p) const noexcept {
    return _bribe_blocked.count(p) != 0;
    }      
} // namespace coup