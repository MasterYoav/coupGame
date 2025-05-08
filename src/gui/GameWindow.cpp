// Email: realyoavperetz@gmail.com
// Minimal SFML GUI implementation for the Coup game.
//  ─────────  draws current turn + bank; no interactive buttons yet ─────────

#include "gui/GameWindow.hpp"
#include <sstream>

using namespace coup_gui;
using namespace coup;

GameWindow::GameWindow(Game& game)
    : _game(game),
      _window(sf::VideoMode(800, 600), "Coup GUI")
{
    _window.setFramerateLimit(60);

    // You can replace this path with any .ttf on your system
#if defined(__APPLE__)
    _font.loadFromFile("/System/Library/Fonts/SFNSDisplay.ttf");
#else
    _font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");
#endif

    _turnText.setFont(_font);
    _turnText.setCharacterSize(24);
    _turnText.setPosition(20.f, 10.f);

    _bankText.setFont(_font);
    _bankText.setCharacterSize(24);
    _bankText.setPosition(20.f, 40.f);
}

void GameWindow::run()
{
    while (_window.isOpen()) {
        handleEvents(); 
        updateTexts();
        draw();
    }
}

void GameWindow::handleEvents()
{
    sf::Event e;
    while (_window.pollEvent(e)) {
        if (e.type == sf::Event::Closed)
            _window.close();
    }
}

void GameWindow::updateTexts()
{
    _turnText.setString("Turn: " + _game.turn());
    _bankText.setString("Bank: " + std::to_string(_game.bank()));
}

void GameWindow::draw()
{
    _window.clear(sf::Color(30, 30, 30));

    _window.draw(_turnText);
    _window.draw(_bankText);

    // (No interactive controls yet)

    _window.display();
}