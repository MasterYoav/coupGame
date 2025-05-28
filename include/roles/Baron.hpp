// Email: realyoavperetz@gmail.com
#pragma once

#include "Player.hpp"

namespace coup {

class Baron : public Player {
public:
    Baron(Game& game, const std::string& name);
    
    void invest();                            // spend 3, gain 6
    void on_sanction(Player& attacker) override; // compensation +1
};

} // namespace coup