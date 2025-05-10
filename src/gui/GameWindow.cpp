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
static constexpr float PANEL_W = 250.f;
static constexpr float PANEL_PAD = 15.f;
static constexpr float BUTTON_W = 100.f;
static constexpr float BUTTON_H = 40.f;
static constexpr float BUTTON_SP = 20.f;
static const sf::Color PANEL_BG(40,40,40);
static const sf::Color BUTTON_BG(80,80,200);
static const sf::Color TEXT_COLOR(230,230,230);

GameWindow::GameWindow(Game& game)
    : _game(game), _window({WIN_W, WIN_H}, "Coup – Menu")
{
    std::cerr << "[DEBUG] GameWindow constructor" << std::endl;

    _window.setFramerateLimit(30);
    if (!_font.loadFromFile("assets/sansation.ttf"))
        std::cerr << "Warning: failed to load font assets/sansation.ttf\n";

    // Menu: Add Player and Start Game
    float x = PANEL_PAD;
    _menuButtons.push_back(makeButton("Add Player", x, PANEL_PAD / 2, [&]() {
        _newPlayerName.clear();
        _showAddDialog = true;
        _dialogBg.setSize({300.f, 80.f});
        _dialogBg.setFillColor({50,50,60,200});
        _dialogBg.setPosition((WIN_W - 300) / 2, (WIN_H - 80) / 2);
        _dialogPrompt.setFont(_font);
        _dialogPrompt.setString("Enter name:");
        _dialogPrompt.setCharacterSize(18);
        _dialogPrompt.setFillColor(sf::Color::White);
        _dialogPrompt.setPosition(_dialogBg.getPosition() + sf::Vector2f(10,10));
    }));
    x += BUTTON_W + BUTTON_SP;
    _menuButtons.push_back(makeButton("Start Game", x, PANEL_PAD / 2, [&]() {
        if (_game.players().size() < 2) {
            showPopup("Need at least 2 players");
        } else {
            _state = WindowState::Playing;
            _window.setTitle("Coup – Playing");
            // Setup play widgets
            _bankCircle.setRadius(40.f);
            _bankCircle.setFillColor(sf::Color(200,180,50));
            _bankCircle.setPosition((WIN_W + PANEL_W) / 2 - 40.f, MENU_H + PANEL_PAD);
            _turnText.setFont(_font);
            _turnText.setCharacterSize(20);
            _turnText.setFillColor(TEXT_COLOR);
            // Create action buttons
            _buttons.clear();
            std::vector<std::string> acts = {"Gather","Tax","Bribe","Arrest","Sanction","Coup"};
            float totalW = acts.size() * BUTTON_W + (acts.size() - 1) * BUTTON_SP;
            float startX = (WIN_W - totalW) / 2;
            for (size_t i = 0; i < acts.size(); ++i) {
                float bx = startX + i * (BUTTON_W + BUTTON_SP);
                _buttons.push_back(makeButton(acts[i], bx, WIN_H - BUTTON_H - PANEL_PAD, [&, i]() {
                    try {
                        coup::Player* cp = _game.current_player();
                        if (!cp) return;
                        const std::string& act = acts[i];
                        if(act == "Gather") {
    // DEBUG: Gather
    coup::Player* cp = _game.current_player();
    std::cerr << "[DEBUG] Gather clicked. cp ptr=" << cp << std::endl;
    if(!cp) {
        showPopup("DEBUG: no current player");
    } else {
        cp->gather();
        std::cerr << "[DEBUG] Gather succeeded, new coins=" << cp->coins() << std::endl;
        showPopup(cp->name() + " gathered 1 coin");
        _game.next_turn();
    }
} else if (act == "Tax") {
                            cp->tax();
                            showPopup(cp->name() + " collected 2 coins");
                            _game.next_turn();
                        } else if (act == "Bribe") {
                            cp->bribe();
                            showPopup(cp->name() + " bribed for an extra action");
                            _game.next_turn();
                        } else if (act == "Arrest") {
                            // TODO: target prompt
                        } else if (act == "Sanction") {
                            // TODO: target prompt
                        } else if (act == "Coup") {
                            // TODO: target prompt
                        }
                    } catch (const CoupException& ex) {
                        showPopup(ex.what());
                    }
                }));
            }
        }
    }));

    // Popup text
    _popupText.setFont(_font);
    _popupText.setCharacterSize(18);
    _popupText.setFillColor(sf::Color::White);
}

void GameWindow::run() {
    std::cerr << "[DEBUG] run() start" << std::endl;
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

void GameWindow::handleMenuEvents() {
    static std::mt19937 rng(std::random_device{}());
    static std::uniform_int_distribution<int> dist(0,5);
    sf::Event e;
    while (_window.pollEvent(e)) {
        if (e.type == sf::Event::Closed) _window.close();
        else if (!_showAddDialog && e.type == sf::Event::MouseButtonPressed) {
            sf::Vector2f m(e.mouseButton.x, e.mouseButton.y);
            for (auto& b : _menuButtons) if (b.shape.getGlobalBounds().contains(m)) b.onClick();
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
                        case 0: role = "Governor"; new Governor(_game, _newPlayerName); break;
                        case 1: role = "Spy"; new Spy(_game, _newPlayerName); break;
                        case 2: role = "Baron"; new Baron(_game, _newPlayerName); break;
                        case 3: role = "General"; new General(_game, _newPlayerName); break;
                        case 4: role = "Judge"; new Judge(_game, _newPlayerName); break;
                        default: role = "Merchant"; new Merchant(_game, _newPlayerName); break;
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
    _window.clear({20,20,20});
    // Menu bar
    sf::RectangleShape bar({(float)WIN_W, MENU_H});
    bar.setFillColor({40,40,40});
    _window.draw(bar);
    // Buttons
    for (auto& b : _menuButtons) {
        _window.draw(b.shape);
        _window.draw(b.label);
    }
    // Add dialog
    if (_showAddDialog) {
        _window.draw(_dialogBg);
        _window.draw(_dialogPrompt);
        sf::Text in(_newPlayerName, _font, 20);
        in.setFillColor(sf::Color::White);
        in.setPosition(_dialogBg.getPosition() + sf::Vector2f(10,40));
        _window.draw(in);
    }
    // Popup
    if (_popupMessage) {
        if (_popupClock.getElapsedTime().asSeconds() < 2.f) {
            _popupText.setString(*_popupMessage);
            _popupText.setPosition((WIN_W - _popupText.getLocalBounds().width) / 2, MENU_H + PANEL_PAD);
            _window.draw(_popupText);
        } else {
            _popupMessage.reset();
        }
    }
    _window.display();
}

void GameWindow::handlePlayEvents() {
    sf::Event e;
    while (_window.pollEvent(e)) {
        std::cerr << "[DEBUG] play-event type=" << e.type << std::endl;
        if (e.type == sf::Event::Closed) {
            _window.close();
        }
        else if (e.type == sf::Event::MouseButtonPressed) {
            std::cerr << "[DEBUG] play-click at (" 
                      << e.mouseButton.x << "," << e.mouseButton.y << ")" 
                      << std::endl;

            for (auto& b : _buttons) {
                if (b.shape.getGlobalBounds().contains(e.mouseButton.x, e.mouseButton.y)) {
                    std::cerr << "[DEBUG] button '" 
                              << b.label.getString().toAnsiString()
                              << "' clicked" << std::endl;
                    b.onClick();
                }
            }
        }
    }
}

void GameWindow::updatePlayLayout() {
    _playerNameTexts.clear();
    _playerCoinTexts.clear();
    _playerRoleTexts.clear();
    float y = PANEL_PAD + MENU_H;
    coup::Player* current = _game.current_player();
    for (coup::Player* p : _game.playerObjects()) {
        bool isCurrent = (p == current);
        sf::Text nameTxt(p->name(), _font, 18);
        nameTxt.setFillColor(isCurrent ? sf::Color::Yellow : sf::Color::White);
        nameTxt.setStyle(isCurrent ? sf::Text::Bold : sf::Text::Regular);
        nameTxt.setPosition(PANEL_PAD, y);
        _playerNameTexts.push_back(nameTxt);

        sf::Text coinTxt(std::to_string(p->coins()), _font, 18);
        coinTxt.setFillColor(sf::Color(212,175,55));
        coinTxt.setPosition(PANEL_PAD + 120.f, y);
        _playerCoinTexts.push_back(coinTxt);

        std::string roleStr = isCurrent ? p->role() : "";
        sf::Text roleTxt(roleStr, _font, 18);
        roleTxt.setFillColor(sf::Color::Blue);
        roleTxt.setPosition(PANEL_PAD + 200.f, y);
        _playerRoleTexts.push_back(roleTxt);

        y += 50.f;
    }
    // Turn label top-right
    sf::Vector2u ws = _window.getSize();
    _turnText.setString("Turn: " + _game.turn());
    _turnText.setPosition(ws.x - _turnText.getLocalBounds().width - PANEL_PAD,
                           PANEL_PAD);
}

void GameWindow::drawPlay() {
    _window.clear();
    // Side panel
    sf::RectangleShape panel({PANEL_W, (float)WIN_H});
    panel.setFillColor(PANEL_BG);
    panel.setOutlineColor(sf::Color(100,100,100));
    panel.setOutlineThickness(2.f);
    _window.draw(panel);
    // Player list
    for (auto& t : _playerNameTexts) _window.draw(t);
    for (auto& t : _playerCoinTexts) _window.draw(t);
    for (auto& t : _playerRoleTexts) _window.draw(t);
    // Turn label
    _window.draw(_turnText);
    // Bank circle only
    _window.draw(_bankCircle);
    // Action buttons
    for (auto& b : _buttons) {
        _window.draw(b.shape);
        _window.draw(b.label);
    }
    _window.display();
}

void GameWindow::showPopup(const std::string& msg) {
    _popupMessage = msg;
    _popupClock.restart();
}

GameWindow::Button GameWindow::makeButton(const std::string& text, float x, float y, std::function<void()> cb) {
    Button b;
    b.shape.setSize({BUTTON_W, BUTTON_H});
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


