// Email: realyoavperetz@gmail.com
#pragma once


#include <string>
#include "exceptions.hpp"
#include "Action.hpp"

namespace coup {

class Game;  // forward declaration

class Player {
public:
    Player(Game& game, const std::string& name);
    virtual ~Player() = default;
    // Actions
    virtual void gather();
    virtual void tax();
    virtual void bribe();
    virtual void arrest(Player& target);
    virtual void sanction(Player& target);
    virtual void coup(Player& target);
    virtual void undo(Player& action_owner);

    // Reaction hooks (public so Game can call them)
    virtual void on_sanction([[maybe_unused]] Player& attacker) {}
    virtual void on_arrested([[maybe_unused]] Player& thief)   {}
    virtual void start_of_turn()                               {}

    // Info
    const std::string& name()  const noexcept { return _name; }
    int                 coins() const noexcept { return _coins; }

    // Utility
    void spend(int amount);
    void gain(int amount) noexcept;

    // override your existing role() to return this
    virtual std::string role() const { return _roleName;}

    void change_role(const std::string& newRole) {
        _roleName = newRole;
    }
    
protected:
    std::string _name;
    int         _coins = 0;
    Game&       _game;
    std::string _roleName;    // store the current role name
    bool    _extraActionAllowed = false;
    Player* _lastArrestTarget   = nullptr;

    static constexpr int MANDATORY_COUP_LIMIT = 10;
};

} // namespace coup