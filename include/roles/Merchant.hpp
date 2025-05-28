
// Email: realyoavperetz@gmail.com
#pragma once
#include "Player.hpp"

namespace coup {

class Merchant : public Player {
public:
    Merchant(Game& game, const std::string& name);
    
    // +1 coin at start of turn if holding ≥3 coins
    void start_of_turn() override;

    // When arrested: pay 2 coins to bank instead of losing 1 to thief
    void on_arrested(Player& thief) override;
};

} // namespace coup