#pragma once
// Email: realyoavperetz@gmail.com
// Defines basic action types and an optional log record used by Game.

#include <cstddef>

namespace coup {

class Player; // forward declaration

/**
 * Enum describing every distinct action that can be performed during the game.
 */
enum class ActionType {
    Gather,
    Tax,
    Bribe,
    Arrest,
    Sanction,
    TaxCancel,
    Coup
};

/**
 * Simple log entry used by Game to allow deferred blocking of actions.
 */
struct ActionRecord {
    Player*    actor      = nullptr;   ///< player who initiated the action
    ActionType type       = ActionType::Gather;
    Player*    target     = nullptr;   ///< may be nullptr if no target
    std::size_t turn_idx  = 0;         ///< turn index when action happened
};

}
