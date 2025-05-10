#pragma once
// Email: realyoavperetz@gmail.com
#include "Player.hpp"

namespace coup {

class Judge : public Player {
public:
    Judge(Game& game, const std::string& name);
    std::string role() const override { return "Judge"; }
    void undo(Player& action_owner) override;      // cancel Bribe
    void on_sanction(Player& attacker) override;   // attacker pays +1
};

} // namespace coup