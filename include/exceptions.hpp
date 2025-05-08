// Email: realyoavperetz@gmail.com
// This header defines the generic exception class used by the Coup game engine.
// All rule‑breaking runtime errors should be reported via CoupException.

#pragma once

#include <stdexcept>
#include <string>

namespace coup {

/**
 * @brief Generic runtime error for illegal game actions.
 *
 * Throw this exception when a player performs an action that
 * breaks the game rules (for example: wrong turn, not enough coins,
 * or trying to block an impossible move).
 */
class CoupException : public std::logic_error {
public:
    /// Build a new exception with a short error message.
    explicit CoupException(const std::string &msg);
};

/**
 * @brief Helper macro for concise error throwing.
 *
 * Example usage:
 *     COUP_THROW("Not your turn");
 */
#define COUP_THROW(msg) throw coup::CoupException{msg}

}
