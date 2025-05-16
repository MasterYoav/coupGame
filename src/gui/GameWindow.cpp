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

// ──────────────────── Layout constants ────────────────────
static constexpr unsigned WIN_W = 800, WIN_H = 600;
static constexpr float   MENU_H    = 60.f;
static constexpr float   PANEL_W   = 250.f;
static constexpr float   PANEL_PAD = 15.f;
static constexpr float   BUTTON_W  = 100.f;
static constexpr float   BUTTON_H  = 40.f;
static constexpr float   BUTTON_SP = 20.f;
static const    sf::Color PANEL_BG ( 40,  40,  40);
static const    sf::Color BUTTON_BG( 80,  80, 200);
static const    sf::Color TEXT_COLOR(230, 230, 230);
static const    sf::Color DIALOG_BG( 50,  50,  60, 230);

// ──────────────────── Helper enum ────────────────────
enum class PendingAct { None, Arrest, Sanction, Coup };

// ──────────────────── ctor ────────────────────
GameWindow::GameWindow(Game& game)
    : _game(game)
    , _window({WIN_W, WIN_H}, "Coup – Menu")
    , _showAddDialog(false)          // ① must come first
    , _state(WindowState::Menu)      // ② second
    , _showTargetDialog(false)
    , _pending(PendingAct::None)
{
    _window.setFramerateLimit(30);
    if (!_font.loadFromFile("assets/sansation.ttf"))
        std::cerr << "[WARN] font assets/sansation.ttf not found\n";

    // ───── menu buttons ─────
    float x = PANEL_PAD;

    _menuButtons.push_back(makeButton("Add Player", x, PANEL_PAD / 2, [&]() {
        // open modal text dialog
        _newPlayerName.clear();
        _showAddDialog = true;
        _dialogBg.setSize({300.f, 80.f});
        _dialogBg.setFillColor(DIALOG_BG);
        _dialogBg.setPosition((WIN_W - 300) / 2.f, (WIN_H - 80) / 2.f);
        _dialogPrompt.setFont(_font);
        _dialogPrompt.setString("Enter name:");
        _dialogPrompt.setCharacterSize(18);
        _dialogPrompt.setFillColor(sf::Color::White);
        _dialogPrompt.setPosition(_dialogBg.getPosition() + sf::Vector2f(10.f, 10.f));
    }));

    x += BUTTON_W + BUTTON_SP;

    _menuButtons.push_back(makeButton("Start Game", x, PANEL_PAD / 2, [&]() {
        if (_game.players().size() < 2) {
            showPopup("Need at least 2 players");
            return;
        }

        _state = WindowState::Playing;
        _window.setTitle("Coup – Playing");

        // widgets used only in playing mode
        _bankCircle.setRadius(40.f);
        _bankCircle.setFillColor(sf::Color(200,180,50));
        _bankCircle.setPosition((WIN_W + PANEL_W) / 2.f - 40.f, MENU_H + PANEL_PAD);

        _turnText.setFont(_font);
        _turnText.setCharacterSize(20);
        _turnText.setFillColor(TEXT_COLOR);

        // create bottom-center action buttons
        _buttons.clear();
        const std::vector<std::string> acts =
            {"Gather","Tax","Bribe","Arrest","Sanction","Coup"};

        const float totalW = acts.size()*BUTTON_W + (acts.size()-1)*BUTTON_SP;
        const float startX = (WIN_W - totalW) / 2.f;

        for (size_t i = 0; i < acts.size(); ++i) {
            const float bx = startX + i*(BUTTON_W + BUTTON_SP);
            const std::string actName = acts[i];      // copy to avoid dangling ref

            _buttons.push_back(makeButton(actName, bx,
                                          WIN_H - BUTTON_H - PANEL_PAD,
                                          [&, actName]() { onActionClicked(actName); }));
        }
    }));

    // popup styling
    _popupText.setFont(_font);
    _popupText.setCharacterSize(18);
    _popupText.setFillColor(sf::Color::White);
}

// ──────────────────── main loop ────────────────────
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

// ──────────────────── MENU events ────────────────────
void GameWindow::handleMenuEvents() {
    static std::mt19937 rng(std::random_device{}());

    sf::Event e;
    while (_window.pollEvent(e)) {
        if (e.type == sf::Event::Closed) _window.close();

        else if (!_showAddDialog && e.type == sf::Event::MouseButtonPressed) {
            sf::Vector2f m(e.mouseButton.x, e.mouseButton.y);
            for (auto& b : _menuButtons)
                if (b.shape.getGlobalBounds().contains(m)) b.onClick();
        }

        else if (_showAddDialog && e.type == sf::Event::TextEntered) {
            char c = static_cast<char>(e.text.unicode);

            if (c == 8 && !_newPlayerName.empty()) {       // back-space
                _newPlayerName.pop_back();
            }
            else if (c == '\r' || c == '\n') {             // enter → finalize
                // duplicate name?
                for (const auto& nm : _game.players())
                    if (nm == _newPlayerName) {
                        showPopup("Name is taken");
                        _showAddDialog = false;
                        return;
                    }

                // collect used roles
                std::unordered_set<std::string> used;
                for (auto* p : _game.playerObjects()) used.insert(p->role());

                static const std::vector<std::string> ALL = {
                    "Governor","Spy","Baron","General","Judge","Merchant" };

                std::vector<std::string> free;
                for (const auto& r : ALL)
                    if (!used.count(r)) free.push_back(r);

                if (free.empty()) {
                    showPopup("All six roles are already taken");
                    _showAddDialog = false;
                    return;
                }

                std::uniform_int_distribution<size_t> dist(0, free.size()-1);
                const std::string role = free[dist(rng)];

                // instantiate correct role
                if      (role == "Governor") new Governor(_game, _newPlayerName);
                else if (role == "Spy")      new Spy(_game, _newPlayerName);
                else if (role == "Baron")    new Baron(_game, _newPlayerName);
                else if (role == "General")  new General(_game, _newPlayerName);
                else if (role == "Judge")    new Judge(_game, _newPlayerName);
                else                         new Merchant(_game, _newPlayerName);

                showPopup("Added " + _newPlayerName + " as " + role);
                _showAddDialog = false;
            }
            else if (c >= 32 && c < 127) {                 // printable char
                _newPlayerName.push_back(c);
            }
        }
    }
}

// ──────────────────── PLAY events ────────────────────
void GameWindow::handlePlayEvents() {
    sf::Event e;
    while (_window.pollEvent(e)) {
        if (e.type == sf::Event::Closed) _window.close();

        else if (_showTargetDialog && e.type == sf::Event::MouseButtonPressed) {
            sf::Vector2f m(e.mouseButton.x, e.mouseButton.y);
            for (auto& b : _targetButtons)
                if (b.shape.getGlobalBounds().contains(m)) { b.onClick(); break; }
        }

        else if (!_showTargetDialog && e.type == sf::Event::MouseButtonPressed) {
            sf::Vector2f m(e.mouseButton.x, e.mouseButton.y);
            for (auto& b : _buttons)
                if (b.shape.getGlobalBounds().contains(m)) { b.onClick(); break; }
        }
    }
}

// ──────────────────── onActionClicked ────────────────────
void GameWindow::onActionClicked(const std::string& act) {
    try {
        coup::Player* cp = _game.current_player();
        if (!cp) return;

        if (act == "Gather") {
            cp->gather();                                   // already advances turn
            showPopup(cp->name() + " gathered 1 coin");
        }
        else if (act == "Tax") {
            cp->tax();
            showPopup(cp->name() + " collected 2 coins");
        }
        else if (act == "Bribe") {
            cp->bribe();
            showPopup(cp->name() + " bribed for an extra action");
        }
        else if (act == "Arrest") {
            createTargetDialog(PendingAct::Arrest);
        }
        else if (act == "Sanction") {
            createTargetDialog(PendingAct::Sanction);
        }
        else if (act == "Coup") {
            createTargetDialog(PendingAct::Coup);
        }
    }
    catch (const CoupException& ex) {
        showPopup(ex.what());
    }
}

// ──────────────────── target-dialog helpers ────────────────────
void GameWindow::createTargetDialog(PendingAct act) {
    _pending = act;
    _showTargetDialog = true;
    _targetButtons.clear();

    const float dlgW   = 300.f;
    const float dlgPAD = 10.f;
    const float entryH = 35.f;
    const float dlgX   = (WIN_W - dlgW) / 2.f;
    const float dlgY   = (WIN_H - ((_game.playerObjects().size()-1)*entryH + dlgPAD*2)) / 2.f;

    _targetBg.setSize({dlgW, (_game.playerObjects().size()-1)*entryH + dlgPAD*2});
    _targetBg.setPosition(dlgX, dlgY);
    _targetBg.setFillColor(DIALOG_BG);

    coup::Player* current = _game.current_player();
    float y = dlgY + dlgPAD;

    for (coup::Player* p : _game.playerObjects()) {
        if (p == current) continue;               // cannot target self
        Button b = makeButton(p->name(), dlgX + dlgPAD, y, [this, p]() {
            executePendingAction(p);
        });
        b.shape.setSize({dlgW - dlgPAD*2, entryH - 5.f});
        b.label.setPosition(b.shape.getPosition() + sf::Vector2f(10.f, 6.f));
        _targetButtons.push_back(std::move(b));
        y += entryH;
    }
}

void GameWindow::executePendingAction(coup::Player* target) {
    try {
        coup::Player* cp = _game.current_player();
        if (!cp) return;

        switch (_pending) {
            case PendingAct::Arrest:
                cp->arrest(*target);
                showPopup(cp->name() + " arrested " + target->name());
                break;
            case PendingAct::Sanction:
                cp->sanction(*target);
                showPopup(cp->name() + " sanctioned " + target->name());
                break;
            case PendingAct::Coup:
                cp->coup(*target);
                showPopup(cp->name() + " performed a coup on " + target->name());
                break;
            default: break;
        }
    }
    catch (const CoupException& ex) {
        showPopup(ex.what());
    }
    _pending = PendingAct::None;
    _showTargetDialog = false;
}

// ──────────────────── updatePlayLayout ────────────────────
void GameWindow::updatePlayLayout() {
    _playerNameTexts.clear();
    _playerCoinTexts.clear();
    _playerRoleTexts.clear();

    float y = MENU_H + PANEL_PAD;
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

        sf::Text roleTxt(isCurrent ? p->role() : "", _font, 18);
        roleTxt.setFillColor(sf::Color::Blue);
        roleTxt.setPosition(PANEL_PAD + 200.f, y);
        _playerRoleTexts.push_back(roleTxt);

        y += 50.f;
    }

    sf::Vector2u ws = _window.getSize();
    _turnText.setString("Turn: " + _game.turn());
    _turnText.setPosition(ws.x - _turnText.getLocalBounds().width - PANEL_PAD,
                          PANEL_PAD);
}

// ──────────────────── drawPlay ────────────────────
void GameWindow::drawPlay() {
    _window.clear();

    // side panel
    sf::RectangleShape panel({PANEL_W, static_cast<float>(WIN_H)});
    panel.setFillColor(PANEL_BG);
    panel.setOutlineColor(sf::Color(100,100,100));
    panel.setOutlineThickness(2.f);
    _window.draw(panel);

    // player list
    for (const auto& t : _playerNameTexts)  _window.draw(t);
    for (const auto& t : _playerCoinTexts)  _window.draw(t);
    for (const auto& t : _playerRoleTexts)  _window.draw(t);

    // other widgets
    _window.draw(_turnText);
    _window.draw(_bankCircle);

    for (const auto& b : _buttons) {
        _window.draw(b.shape);
        _window.draw(b.label);
    }

    // target dialog
    if (_showTargetDialog) {
        _window.draw(_targetBg);
        for (const auto& b : _targetButtons) {
            _window.draw(b.shape);
            _window.draw(b.label);
        }
    }

    _window.display();
}

// ──────────────────── drawMenu (unchanged) ────────────────────
void GameWindow::drawMenu() {
    _window.clear({20,20,20});

    sf::RectangleShape bar({static_cast<float>(WIN_W), MENU_H});
    bar.setFillColor({40,40,40});
    _window.draw(bar);

    for (auto& b : _menuButtons) {
        _window.draw(b.shape);
        _window.draw(b.label);
    }

    if (_showAddDialog) {
        _window.draw(_dialogBg);
        _window.draw(_dialogPrompt);
        sf::Text in(_newPlayerName, _font, 20);
        in.setFillColor(sf::Color::White);
        in.setPosition(_dialogBg.getPosition() + sf::Vector2f(10.f, 40.f));
        _window.draw(in);
    }

    if (_popupMessage) {
        if (_popupClock.getElapsedTime().asSeconds() < 2.f) {
            _popupText.setString(*_popupMessage);
            _popupText.setPosition(
                (WIN_W - _popupText.getLocalBounds().width) / 2.f,
                MENU_H + PANEL_PAD);
            _window.draw(_popupText);
        } else {
            _popupMessage.reset();
        }
    }

    _window.display();
}

// ──────────────────── helper: makeButton ────────────────────
GameWindow::Button GameWindow::makeButton(const std::string& txt,
                                          float x, float y,
                                          std::function<void()> cb) {
    Button b;
    b.shape.setSize({BUTTON_W, BUTTON_H});
    b.shape.setPosition(x, y);
    b.shape.setFillColor(BUTTON_BG);

    b.label.setFont(_font);
    b.label.setString(txt);
    b.label.setCharacterSize(16);
    b.label.setFillColor(sf::Color::White);
    b.label.setPosition(x + 10.f, y + 6.f);

    b.onClick = std::move(cb);
    return b;
}

// ──────────────────── helper: popup ────────────────────
void GameWindow::showPopup(const std::string& msg) {
    _popupMessage = msg;
    _popupClock.restart();
}