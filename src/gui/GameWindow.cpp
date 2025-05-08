// Email: realyoavperetz@gmail.com
// Implementation of GameWindow: renders left panel, center bank, and action buttons.

#include "gui/GameWindow.hpp"
#include "Game.hpp"
#include "Player.hpp"
#include <sstream>

using namespace coup_gui;
using namespace coup;

static constexpr unsigned WINDOW_W = 800;
static constexpr unsigned WINDOW_H = 600;
static constexpr float PANEL_W    = 200.f;
static constexpr float BUTTON_H   = 50.f;
static constexpr float BUTTON_Y   = WINDOW_H - BUTTON_H - 10.f;
static const sf::Color PANEL_BG(40, 40, 40);
static const sf::Color BUTTON_BG(80, 80, 200);
static const sf::Color TEXT_COLOR(230, 230, 230);

GameWindow::GameWindow(Game& game)
    : _game(game),
      _window({WINDOW_W, WINDOW_H}, "Coup GUI")
{
    _window.setFramerateLimit(30);

    #if defined(__APPLE__)
    _font.loadFromFile("/System/Library/Fonts/SFNSDisplay.ttf");
    #else
    _font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");
    #endif

    // Turn text (top right)
    _turnText.setFont(_font);
    _turnText.setCharacterSize(20);
    _turnText.setFillColor(TEXT_COLOR);
    _turnText.setPosition(PANEL_W + 20.f, 10.f);

    // Bank circle (center)
    _bankCircle.setRadius(50.f);
    _bankCircle.setFillColor(sf::Color(200, 180, 50));
    _bankCircle.setPosition(
        (WINDOW_W - _bankCircle.getRadius()*2)/2,
        (WINDOW_H - BUTTON_H)/2 - _bankCircle.getRadius()
    );
    _bankText.setFont(_font);
    _bankText.setCharacterSize(24);
    _bankText.setFillColor(TEXT_COLOR);

    // Action buttons
    float x = PANEL_W + 20.f;
    _buttons.push_back(makeButton("Gather", x, BUTTON_Y, [&]() {
        _game.players(); // placeholder
    }));
    x += 120.f;
    _buttons.push_back(makeButton("Tax", x, BUTTON_Y, [&]() {
        _game.players();
    }));
    x += 80.f;
    _buttons.push_back(makeButton("Coup", x, BUTTON_Y, [&]() {
        _game.players();
    }));
}

void GameWindow::run() {
    while (_window.isOpen()) {
        handleEvents();
        updateLayout();
        draw();
    }
}

void GameWindow::handleEvents() {
    sf::Event ev;
    while (_window.pollEvent(ev)) {
        if (ev.type == sf::Event::Closed) {
            _window.close();
        }
        if (ev.type == sf::Event::MouseButtonPressed) {
            sf::Vector2f mp(ev.mouseButton.x, ev.mouseButton.y);
            for (auto& btn : _buttons) {
                if (btn.shape.getGlobalBounds().contains(mp)) {
                    btn.onClick();
                }
            }
        }
    }
}

void GameWindow::updateLayout() {
    // Build vertical list of player names
    _playerTexts.clear();
    auto names = _game.players();
    float y = 10.f;
    for (const auto& nm : names) {
        sf::Text t;
        t.setFont(_font);
        t.setCharacterSize(18);
        t.setFillColor(TEXT_COLOR);
        t.setString(nm);
        t.setPosition(10.f, y);
        _playerTexts.push_back(t);
        y += 30.f;
    }

    // Update turn text
    _turnText.setString("Turn: " + _game.turn());

    // Update bank text inside circle
    _bankText.setString(std::to_string(_game.bank()));
    auto bb = _bankText.getLocalBounds();
    _bankText.setPosition(
        _bankCircle.getPosition().x + _bankCircle.getRadius() - bb.width/2,
        _bankCircle.getPosition().y + _bankCircle.getRadius() - bb.height/2 - 5.f
    );
}

void GameWindow::draw() {
    _window.clear();

    // Left panel background
    sf::RectangleShape panelBg({PANEL_W, float(WINDOW_H)});
    panelBg.setFillColor(PANEL_BG);
    _window.draw(panelBg);

    // Player list
    for (auto& t : _playerTexts) {
        _window.draw(t);
    }

    // Turn text
    _window.draw(_turnText);

    // Bank
    _window.draw(_bankCircle);
    _window.draw(_bankText);

    // Buttons
    for (auto& btn : _buttons) {
        _window.draw(btn.shape);
        _window.draw(btn.label);
    }

    _window.display();
}

GameWindow::Button GameWindow::makeButton(
    const std::string& text, float x, float y, std::function<void()> cb)
{
    Button b;
    b.shape.setSize({100.f, BUTTON_H});
    b.shape.setPosition(x, y);
    b.shape.setFillColor(BUTTON_BG);

    b.label.setFont(_font);
    b.label.setString(text);
    b.label.setCharacterSize(18);
    b.label.setFillColor(TEXT_COLOR);
    b.label.setPosition(x + 10.f, y + 10.f);

    b.onClick = std::move(cb);
    return b;
}