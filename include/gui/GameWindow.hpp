#pragma once
// Email: realyoavperetz@gmail.com
// Simple SFML window that visualises the Coup game and lets the current
// player click basic action buttons (gather / tax / coup).
// This is a minimal skeleton – extend as needed.

#include <SFML/Graphics.hpp>
#include <memory>
#include "Game.hpp"

namespace coup_gui {

class GameWindow {
public:
    explicit GameWindow(coup::Game& game);
    void run();

private:
    coup::Game&        _game;
    sf::RenderWindow   _window;

    // UI helpers
    sf::Font           _font;
    sf::Text           _turnText;
    sf::Text           _bankText;

    // Basic buttons
    struct Button {
        sf::RectangleShape shape;
        sf::Text           label;
        std::function<void()> onClick;
    };
    std::vector<Button> _buttons;

    void handleEvents();
    void updateTexts();
    void draw();

    // factory helpers
    Button makeButton(const std::string& text, float y, std::function<void()> cb);
};

} // namespace coup_gui