// Email: realyoavperetz@gmail.com
// Implementation of GameWindow with Menu and Playing modes.
// Fixed random role assignment using C++ <random> for proper distribution.

#include <iostream>
#include <random>
#include "gui/GameWindow.hpp"
#include "Game.hpp"
#include "Player.hpp"
#include "exceptions.hpp"
#include "roles/Governor.hpp"
#include "roles/Spy.hpp"
#include "roles/Baron.hpp"
#include "roles/General.hpp"
#include "roles/Judge.hpp"
#include "roles/Merchant.hpp"

using namespace coup_gui;
using namespace coup;

static constexpr unsigned WIN_W = 800, WIN_H = 600;
static constexpr float MENU_H = 40.f;
static constexpr float PANEL_W = 200.f;
static constexpr float BUTTON_H = 40.f;
static const sf::Color PANEL_BG(40, 40, 40);
static const sf::Color BUTTON_BG(80, 80, 200);
static const sf::Color TEXT_COLOR(230, 230, 230);

GameWindow::GameWindow(Game& game)
    : _game(game),
      _window({WIN_W, WIN_H}, "Coup – Menu")
{
    _window.setFramerateLimit(30);
    if (!_font.loadFromFile("assets/sansation.ttf")) {
        std::cerr << "Warning: failed to load font assets/sansation.ttf" << std::endl;
    }
    // Menu buttons
    float x = 10.f;
    _menuButtons.push_back(makeButton("Add Player", x, 4.f, [&]() {
        _newPlayerName.clear();
        _showAddDialog = true;
        _dialogBg = sf::RectangleShape({300.f, 80.f});
        _dialogBg.setFillColor({50, 50, 60, 200});
        _dialogBg.setPosition((WIN_W - 300) / 2, (WIN_H - 80) / 2);
        _dialogPrompt.setFont(_font);
        _dialogPrompt.setString("Enter name:");
        _dialogPrompt.setCharacterSize(18);
        _dialogPrompt.setFillColor(sf::Color::White);
        _dialogPrompt.setPosition(_dialogBg.getPosition() + sf::Vector2f(10, 10));
    }));
    x += 120.f;
    _menuButtons.push_back(makeButton("Start Game", x, 4.f, [&]() {
        auto cnt = _game.players().size();
        if (cnt < 2) {
            showPopup("Need at least 2 players");
        } else {
            _state = WindowState::Playing;
            _window.setTitle("Coup – Playing");
            // Setup play widgets
            _bankCircle.setRadius(40.f);
            _bankCircle.setFillColor(sf::Color(200, 180, 50));
            _bankCircle.setPosition((WIN_W - 80) / 2, MENU_H + 10);
            _bankText.setFont(_font);
            _bankText.setCharacterSize(20);
            _bankText.setFillColor(TEXT_COLOR);
            _turnText.setFont(_font);
            _turnText.setCharacterSize(20);
            _turnText.setFillColor(TEXT_COLOR);
            // Action buttons
            _buttons.clear();
            static std::mt19937 rng(std::random_device{}());
            std::vector<std::string> actions = {"Gather", "Tax", "Bribe", "Arrest", "Sanction", "Coup"};
            float bx = PANEL_W + 20.f;
            for (auto &act : actions) {
                _buttons.push_back(makeButton(act, bx, WIN_H - BUTTON_H - 10, [&, act]() {
                    try {
                        coup::Player* cp = _game.current_player();
                        if (!cp) return;
                        if (act == "Gather") cp->gather();
                        else if (act == "Tax") cp->tax();
                        else if (act == "Bribe") cp->bribe();
                        else if (act == "Arrest") {
                            // TODO: prompt for target
                        }
                        else if (act == "Sanction") {
                            // TODO: prompt for target
                        }
                        else if (act == "Coup") {
                            // TODO: prompt for target
                        }
                    } catch (const CoupException& ex) {
                        showPopup(ex.what());
                    }
                }));
                bx += 120.f;
            }
        }
    }));
    _popupText.setFont(_font);
    _popupText.setCharacterSize(18);
    _popupText.setFillColor(sf::Color::White);
}

void GameWindow::run() {
    while (_window.isOpen()) {
        if (_state == WindowState::Menu) {
            handleMenuEvents();
            drawMenu();
        } else {
            handlePlayEvents();
            updatePlayLayout();
            drawPlay();
        }
    }
}

// Menu mode
void GameWindow::handleMenuEvents() {
    std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<int> dist(0, 5);
    sf::Event e;
    while (_window.pollEvent(e)) {
        if (e.type == sf::Event::Closed) _window.close();
        else if (!_showAddDialog && e.type == sf::Event::MouseButtonPressed) {
            sf::Vector2f m(e.mouseButton.x, e.mouseButton.y);
            for (auto &b : _menuButtons) if (b.shape.getGlobalBounds().contains(m)) b.onClick();
        } else if (_showAddDialog && e.type == sf::Event::TextEntered) {
            char c = static_cast<char>(e.text.unicode);
            if (c == 8 && !_newPlayerName.empty()) {
                _newPlayerName.pop_back();
            } else if (c == '\r' || c == '\n') {
                auto names = _game.players();
                if (std::find(names.begin(), names.end(), _newPlayerName) != names.end()) {
                    showPopup("Name is taken");
                    _showAddDialog = false;
                    continue;
                }
                try {
                    int idx = dist(rng);
                    std::string role;
                    switch (idx) {
                        case 0: role = "Governor";  new Governor(_game, _newPlayerName); break;
                        case 1: role = "Spy";       new Spy(_game, _newPlayerName);      break;
                        case 2: role = "Baron";     new Baron(_game, _newPlayerName);    break;
                        case 3: role = "General";   new General(_game, _newPlayerName);  break;
                        case 4: role = "Judge";     new Judge(_game, _newPlayerName);    break;
                        default:role = "Merchant";  new Merchant(_game, _newPlayerName); break;
                    }
                    showPopup("Added " + _newPlayerName + " as " + role);
                } catch (const CoupException&) {
                    showPopup("Can't add more players");
                }
                _showAddDialog = false;
            } else if (c >= 32 && c < 127) {
                _newPlayerName.push_back(c);
            }
        }
    }
}

void GameWindow::drawMenu() {
    _window.clear({20, 20, 20});
    sf::RectangleShape bar({static_cast<float>(WIN_W), MENU_H}); bar.setFillColor({40,40,40});
    _window.draw(bar);
    for (auto &b : _menuButtons) { _window.draw(b.shape); _window.draw(b.label); }
    float y = MENU_H + 10.f;
    for (auto &n : _game.players()) {
        sf::Text t(n, _font, 18); t.setFillColor(sf::Color::White); t.setPosition(10.f, y); _window.draw(t); y += 25.f;
    }
    if (_showAddDialog) {
        _window.draw(_dialogBg);
        _window.draw(_dialogPrompt);
        sf::Text in(_newPlayerName, _font, 20); in.setFillColor(sf::Color::White);
        in.setPosition(_dialogBg.getPosition() + sf::Vector2f(10, 40)); _window.draw(in);
    }
    if (_popupMessage) {
        if (_popupClock.getElapsedTime().asSeconds() < 2.f) {
            _popupText.setString(*_popupMessage);
            _popupText.setPosition((WIN_W - _popupText.getLocalBounds().width) / 2, MENU_H + 5);
            _window.draw(_popupText);
        } else _popupMessage.reset();
    }
    _window.display();
}

// Playing mode
void GameWindow::handlePlayEvents() {
    sf::Event e;
    while (_window.pollEvent(e)) {
        if (e.type == sf::Event::Closed) _window.close();
        else if (e.type == sf::Event::MouseButtonPressed) {
            sf::Vector2f mp(e.mouseButton.x, e.mouseButton.y);
            for (auto &btn : _buttons) if (btn.shape.getGlobalBounds().contains(mp)) btn.onClick();
        }
    }
}

void GameWindow::updatePlayLayout() {
    _playerNameTexts.clear();
    _playerCoinTexts.clear();
    _playerRoleTexts.clear();

    float y = 10.f;
    for (coup::Player* p : _game.players()) {
        // Name (in red)
        sf::Text nameTxt(p->name(), _font, 18);
        nameTxt.setFillColor(sf::Color::Red);
        nameTxt.setPosition(10.f, y);
        _playerNameTexts.push_back(nameTxt);

        // Coins (in gold)
        sf::Text coinTxt(std::to_string(p->coins()), _font, 18);
        coinTxt.setFillColor(sf::Color(212,175,55));  // gold-ish
        coinTxt.setPosition(120.f, y);
        _playerCoinTexts.push_back(coinTxt);

        // Role (in blue)
        sf::Text roleTxt(p->role(), _font, 18);
        roleTxt.setFillColor(sf::Color::Blue);
        roleTxt.setPosition(200.f, y);
        _playerRoleTexts.push_back(roleTxt);

        y += 30.f;
    }

    // Turn and bank text positioning unchanged...
    _turnText.setString("Turn: " + _game.turn());
    _bankText.setString(std::to_string(_game.bank()));
    auto bb = _bankText.getLocalBounds();
    _bankText.setPosition(
        _bankCircle.getPosition().x + _bankCircle.getRadius() - bb.width/2,
        _bankCircle.getPosition().y + _bankCircle.getRadius() - bb.height/2 - 5.f
    );
}

void GameWindow::drawPlay() {
    _window.clear();

    // Side panel background
    sf::RectangleShape panel({200.f, (float)WIN_H});
    panel.setFillColor({40,40,40});
    _window.draw(panel);

    // Draw name, coins, role lists
    for (auto& t : _playerNameTexts) _window.draw(t);
    for (auto& t : _playerCoinTexts) _window.draw(t);
    for (auto& t : _playerRoleTexts) _window.draw(t);

    // Draw turn and bank
    _window.draw(_turnText);
    _window.draw(_bankCircle);
    _window.draw(_bankText);

    // Draw action buttons
    for (auto& b : _buttons) {
        _window.draw(b.shape);
        _window.draw(b.label);
    }

    _window.display();
}

void GameWindow::showPopup(const std::string &msg) {
    _popupMessage = msg;
    _popupClock.restart();
}

GameWindow::Button GameWindow::makeButton(const std::string &text, float x, float y, std::function<void()> cb) {
    Button b;
    b.shape.setSize({100.f, BUTTON_H});
    b.shape.setPosition(x, y);
    b.shape.setFillColor(BUTTON_BG);
    b.label.setFont(_font);
    b.label.setString(text);
    b.label.setCharacterSize(16);
    b.label.setFillColor(sf::Color::White);
    b.label.setPosition(x + 10.f, y + 6.f);
    b.onClick = std::move(cb);
    return b;
}

