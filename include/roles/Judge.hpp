#pragma once
// Email: realyoavperetz@gmail.com
#include "Player.hpp"

namespace coup {

class Judge : public Player {
public:
    Judge(Game& game, const std::string& name);
    
    void cancel_bribe(Player& target);      // cancel Bribe
    void on_sanction(Player& attacker) override;   // attacker pays +1
};

} // namespace coup