#pragma once
// Email: realyoavperetz@gmail.com
// SFML window for the Coup game – left panel of player stats,
// center bank circle, bottom row of action buttons.

#include <SFML/Graphics.hpp>
#include <functional>
#include <vector>
#include <string>
#include "Game.hpp"

namespace coup_gui {

class GameWindow {
public:
    explicit GameWindow(coup::Game& game);
    void run();

private:
    // Core game model
    coup::Game&        _game;
    // SFML window
    sf::RenderWindow   _window;

    // Shared font
    sf::Font           _font;

    // Left panel: one text entry per player
    std::vector<sf::Text> _playerTexts;
    // Center: bank circle + text
    sf::CircleShape    _bankCircle;
    sf::Text           _bankText;
    // Top-right: current turn
    sf::Text           _turnText;

    // Bottom: action buttons
    struct Button {
        sf::RectangleShape shape;
        sf::Text           label;
        std::function<void()> onClick;
    };
    std::vector<Button> _buttons;

    // Helpers
    void handleEvents();
    void updateLayout();
    void draw();

    // Button factory
    Button makeButton(const std::string& text, float x, float y,
                      std::function<void()> cb);
};

} // namespace coup_gui