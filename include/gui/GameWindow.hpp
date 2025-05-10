#pragma once
// Email: realyoavperetz@gmail.com
// Header for GameWindow: handles both Menu and Playing modes with SFML.

#include <SFML/Graphics.hpp>
#include <functional>
#include <vector>
#include <string>
#include <optional>
#include "Game.hpp"

namespace coup_gui {

enum class WindowState { Menu, Playing };

class GameWindow {
public:
    explicit GameWindow(coup::Game& game);
    void run();

private:
    // Core game model
    coup::Game& _game;
    // SFML window and assets
    sf::RenderWindow _window;
    sf::Font         _font;

    WindowState      _state = WindowState::Menu;

    // ─── Menu mode members ───
    struct Button {
        sf::RectangleShape shape;
        sf::Text           label;
        std::function<void()> onClick;
    };
    std::vector<Button>        _menuButtons;
    bool                       _showAddDialog = false;
    std::string                _newPlayerName;
    sf::Text                   _dialogPrompt;
    sf::RectangleShape         _dialogBg;
    std::optional<std::string> _popupMessage;
    sf::Text                   _popupText;
    sf::Clock                  _popupClock;

    // ─── Playing mode members ───
    std::vector<sf::Text> _playerNameTexts;   // Name labels (in red)
    std::vector<sf::Text> _playerCoinTexts;   // Coin labels (in gold)
    std::vector<sf::Text> _playerRoleTexts;   // Role labels (in blue)

    sf::Text            _turnText;
    sf::CircleShape     _bankCircle;
    sf::Text            _bankText;
    std::vector<Button> _buttons;

    // ─── Core methods ───
    void handleMenuEvents();
    void drawMenu();

    void handlePlayEvents();
    void updatePlayLayout();
    void drawPlay();

    void showPopup(const std::string& msg);
    Button makeButton(const std::string& text, float x, float y,
                      std::function<void()> cb);
};

} // namespace coup_gui