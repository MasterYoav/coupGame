// GameWindow.cpp (fixed)
#include <iostream>
#include <random>
#include <unordered_set>
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

// Layout constants
static constexpr unsigned WIN_W    = 800;
static constexpr unsigned WIN_H    = 600;
static constexpr unsigned LOG_H    = 200;
static constexpr float MENU_H      = 60.f;
static constexpr float PANEL_W     = 250.f;
static constexpr float PANEL_PAD   = 15.f;
static constexpr float BUTTON_W    = 100.f;
static constexpr float BUTTON_H    = 40.f;
static constexpr float BUTTON_SP   = 20.f;
static const    sf::Color PANEL_BG (40, 40, 40);
static const    sf::Color BUTTON_BG(80, 80, 200);
static const    sf::Color TEXT_COLOR(230,230,230);
static const    sf::Color DIALOG_BG(50,50,60,230);\

enum class PendingAct { None, Arrest, Sanction, Coup, BlockTax};

// ─────────────────── ctor ───────────────────
GameWindow::GameWindow(Game& game)
    : _game(game)
    , _window({WIN_W, WIN_H}, "Coup – Menu")
{
    _window.setFramerateLimit(30);
    if (!_font.loadFromFile("assets/sansation.ttf"))
        std::cerr << "[WARN] font assets/sansation.ttf not found\n";

    // --- Menu buttons ---
    float x = PANEL_PAD;
    _menuButtons.push_back(makeButton("Add Player", x, PANEL_PAD/2, [&]() {
        _newPlayerName.clear();
        _showAddDialog = true;
        _dialogBg.setSize(sf::Vector2f(300.f,80.f));
        _dialogBg.setFillColor(DIALOG_BG);
        _dialogBg.setPosition((WIN_W-300)/2.f,(WIN_H-80)/2.f);
        _dialogPrompt.setFont(_font);
        _dialogPrompt.setString("Enter name:");
        _dialogPrompt.setCharacterSize(18);
        _dialogPrompt.setFillColor(sf::Color::White);
        _dialogPrompt.setPosition(_dialogBg.getPosition()+sf::Vector2f(10.f,10.f));
    }));
    x += BUTTON_W + BUTTON_SP;
    _menuButtons.push_back(makeButton("Start Game", x, PANEL_PAD/2, [&]() {
        if (_game.players().size() < 2) {
            showPopup("Need at least 2 players");
            return;
        }
        _state = WindowState::Playing;
        _window.setTitle("Coup – Playing");
        // setup play UI
        _bankCircle.setRadius(40.f);
        _bankCircle.setFillColor(sf::Color(200,180,50));
        _bankCircle.setPosition((WIN_W+PANEL_W)/2.f - 40.f, MENU_H + PANEL_PAD);
        _turnText.setFont(_font);
        _turnText.setCharacterSize(20);
        _turnText.setFillColor(TEXT_COLOR);
        // action buttons
        _buttons.clear();
        const std::vector<std::string> acts={"Gather","Tax","Bribe","Arrest","Sanction","Coup"};
        float totalW=acts.size()*BUTTON_W + (acts.size()-1)*BUTTON_SP;
        float startX=(WIN_W-totalW)/2.f;
        for(size_t i=0;i<acts.size();++i){
            float bx=startX + i*(BUTTON_W+BUTTON_SP);
            std::string actName=acts[i];
            _buttons.push_back(makeButton(actName,bx,WIN_H-BUTTON_H-PANEL_PAD,
                [&,actName](){ onActionClicked(actName); }
            ));
        }
    }));
    // popup style
    _popupText.setFont(_font);
    _popupText.setCharacterSize(18);
    _popupText.setFillColor(sf::Color::White);
}

// ─────────────────── main loop ───────────────────
void GameWindow::run() {
    // Log window will be created once we enter Playing state
    sf::RenderWindow logWindow;
    bool logCreated = false;

    // Main loop
    while (_window.isOpen()) {
        // --- MENU STATE ---
        if (_state == WindowState::Menu) {
            handleMenuEvents();
            drawMenu();
            continue;
        }

        // --- PLAYING STATE ---
        // Lazily create the log window
        if (!logCreated) {
            logWindow.create({WIN_W, LOG_H}, "Coup – Log");
            logWindow.setFramerateLimit(30);
            logCreated = true;
        }

        // Check for winner
        if (!_showWinnerDialog && _game.playerObjects().size() == 1) {
            createWinnerDialog(_game.winner());
        }

        // Handle game‐window events (buttons, dialogs)
        handlePlayEvents();

        // Handle log‐window events
        sf::Event le;
        while (logCreated && logWindow.pollEvent(le)) {
            if (le.type == sf::Event::Closed) {
                logWindow.close();
                logCreated = false;
            }
        }

        // Update and draw the main game view
        updatePlayLayout();
        drawPlay();

        // Draw the log window if open
        if (logCreated) {
            logWindow.clear({20,20,20});
            auto logs = _game.getActionLog();
            float y = 10.f;
            size_t maxLines = (LOG_H - 20) / 20;
            size_t start = logs.size() > maxLines ? logs.size() - maxLines : 0;
            for (size_t i = start; i < logs.size(); ++i) {
                sf::Text entry(logs[i], _font, 14);
                entry.setFillColor(sf::Color::White);
                entry.setPosition(10.f, y);
                logWindow.draw(entry);
                y += 20.f;
            }
            logWindow.display();
        }

        // If the winner dialog is active, overlay it on top of the game
        if (_showWinnerDialog) {
            _window.draw(_winnerBg);
            _window.draw(_winnerText);
            for (auto& b : _winnerButtons) {
                _window.draw(b.shape);
                _window.draw(b.label);
            }
            // Only redraw once to avoid flicker
            _window.display();
        }
    }
}
// ─────────────────── Winner dialog builder ───────────────────
void GameWindow::createWinnerDialog(const std::string& winner) {
    _showWinnerDialog = true;
    _winnerBg.setSize(sf::Vector2f(400.f,200.f));
    _winnerBg.setFillColor(DIALOG_BG);
    _winnerBg.setPosition((WIN_W-400)/2.f,(WIN_H-200)/2.f);
    _winnerText.setFont(_font);
    _winnerText.setString(winner + " is the winner!");
    _winnerText.setCharacterSize(24);
    _winnerText.setFillColor(sf::Color::White);
    _winnerText.setPosition(_winnerBg.getPosition() + sf::Vector2f(20.f,20.f));
    // buttons
    _winnerButtons.clear();
    float bx = _winnerBg.getPosition().x + 20.f;
    float by = _winnerBg.getPosition().y + 100.f;
    _winnerButtons.push_back(makeButton("Play Again", bx, by, [&]() {
        // reset game state
        while (!_game.playerObjects().empty()) {
            _game.eliminate(_game.playerObjects().front());
        }
        _showWinnerDialog = false;
        _state = WindowState::Menu;
    }));
    _winnerButtons.push_back(makeButton("Quit", bx + BUTTON_W + BUTTON_SP, by, [&]() {
        _window.close();
    }));
}

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
            if (c == 8 && !_newPlayerName.empty()) _newPlayerName.pop_back();
            else if (c=='\r' || c=='\n') {
                for (auto& nm : _game.players())
                    if (nm == _newPlayerName) {
                        showPopup("Name is taken"); _showAddDialog=false; return;
                    }
                std::unordered_set<std::string> used;
                for (auto* p : _game.playerObjects()) used.insert(p->role());
                static const std::vector<std::string> ALL = {"Governor","Spy","Baron","General","Judge","Merchant"};
                std::vector<std::string> free;
                for (auto& r : ALL) if (!used.count(r)) free.push_back(r);
                if (free.empty()) { showPopup("All six roles are already taken"); _showAddDialog=false; return; }
                std::uniform_int_distribution<size_t> dist(0, free.size()-1);
                std::string role = free[dist(rng)];
                if      (role=="Governor") new Governor(_game, _newPlayerName);
                else if (role=="Spy")      new Spy(_game, _newPlayerName);
                else if (role=="Baron")    new Baron(_game, _newPlayerName);
                else if (role=="General")  new General(_game, _newPlayerName);
                else if (role=="Judge")    new Judge(_game, _newPlayerName);
                else                         new Merchant(_game, _newPlayerName);
                showPopup("Added " + _newPlayerName + " as " + role);
                _showAddDialog = false;
            } else if (c>=32 && c<127) _newPlayerName.push_back(c);
        }
    }
}

void GameWindow::handlePlayEvents() {
    sf::Event e;
    while (_window.pollEvent(e)) {
        if (e.type == sf::Event::Closed) _window.close();
        else if (_showTargetDialog && e.type==sf::Event::MouseButtonPressed) {
            sf::Vector2f m(e.mouseButton.x, e.mouseButton.y);
            for (auto& b : _targetButtons)
                if (b.shape.getGlobalBounds().contains(m)) { b.onClick(); break; }
        }
        else if (!_showTargetDialog && e.type==sf::Event::MouseButtonPressed) {
            sf::Vector2f m(e.mouseButton.x, e.mouseButton.y);
            for (auto& b : _buttons)
                if (b.shape.getGlobalBounds().contains(m)) { b.onClick(); break; }
        }
    }
}

void GameWindow::onActionClicked(const std::string& act) {
    try {
        coup::Player* cp = _game.current_player();
        if (!cp) return;

        if (act == "Gather")          { cp->gather(); }
        else if (act == "Tax")        { cp->tax(); }
        else if (act == "Bribe")      { cp->bribe(); }
        else if (act == "Arrest")     { createTargetDialog(PendingAct::Arrest); return; }
        else if (act == "Sanction")   { createTargetDialog(PendingAct::Sanction); return; }
        else if (act == "Coup")       { createTargetDialog(PendingAct::Coup); return; }
        else if (act == "Block Tax") { createTargetDialog(PendingAct::BlockTax); return; }

        // after any non-target action, show popup and advance turn
        showPopup(cp->name() + " performed " + act);
    }
    catch (const CoupException& ex) {
        showPopup(ex.what());
    }
}

void GameWindow::createTargetDialog(PendingAct act) {
    _pending = act;
    _showTargetDialog = true;
    _targetButtons.clear();

    float dlgW = 300.f, dlgPAD = 10.f, entryH = 35.f;
    float dlgX = (WIN_W - dlgW) / 2.f;
    float dlgY = (WIN_H - ((int)_game.playerObjects().size()-1)*entryH - dlgPAD*2)/2.f;

    _targetBg.setSize(sf::Vector2f(dlgW,
        (_game.playerObjects().size() - 1) * entryH + dlgPAD*2));
    _targetBg.setPosition(dlgX, dlgY);
    _targetBg.setFillColor(DIALOG_BG);

    coup::Player* current = _game.current_player();
    float y = dlgY + dlgPAD;
    for (auto* p : _game.playerObjects()) {
        if (p == current) continue;
        auto b = makeButton(p->name(), dlgX + dlgPAD, y, [this,p]() {
            executePendingAction(p);
        });
        b.shape.setSize({dlgW - dlgPAD*2, entryH - 5.f});
        b.label.setPosition(b.shape.getPosition() + sf::Vector2f(10.f,6.f));
        _targetButtons.push_back(std::move(b));
        y += entryH;
    }
}
void GameWindow::executePendingAction(coup::Player* target) {
    try {
        auto* cp = _game.current_player(); if (!cp) return;
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
            case PendingAct::BlockTax:
            // Governor-specific tax block
            static_cast<Governor*>(cp)->block_tax(*target);
            showPopup(cp->name() + " blocked Tax for " + target->name());
            break;
            default:
                break;
        }
    } catch (const CoupException& ex) {
        showPopup(ex.what());
    }
    _pending = PendingAct::None;
    _showTargetDialog = false;
}


void GameWindow::updatePlayLayout() {
    _playerNameTexts.clear();
    _playerCoinTexts.clear();
    _playerRoleTexts.clear();

    // Side-panel texts (unchanged)
    float y = MENU_H + PANEL_PAD;
    coup::Player* current = _game.current_player();
    for (auto* p : _game.playerObjects()) {
        bool isCurrent = (p == current);
        sf::Text name(p->name(), _font, 18);
        name.setFillColor(isCurrent ? sf::Color::Yellow : sf::Color::White);
        name.setStyle(isCurrent ? sf::Text::Bold : sf::Text::Regular);
        name.setPosition(PANEL_PAD, y);
        _playerNameTexts.push_back(name);

        sf::Text coins(std::to_string(p->coins()), _font, 18);
        coins.setFillColor(sf::Color(212,175,55));
        coins.setPosition(PANEL_PAD+120.f, y);
        _playerCoinTexts.push_back(coins);

        sf::Text role(isCurrent ? p->role() : "", _font, 18);
        role.setFillColor(sf::Color::Blue);
        role.setPosition(PANEL_PAD+200.f, y);
        _playerRoleTexts.push_back(role);

        y += 50.f;
    }
    _turnText.setFont(_font);
    _turnText.setString("Turn: " + _game.turn());
    _turnText.setPosition(
        _window.getSize().x - _turnText.getLocalBounds().width - PANEL_PAD,
        PANEL_PAD
    );

    // Rebuild action buttons
    _buttons.clear();
    if (current) {
        std::vector<std::string> acts = {"Gather","Tax","Bribe","Arrest","Sanction","Coup"};
        if (current->role() == "Governor") acts.push_back("Block Tax");

        float totalW = acts.size()*BUTTON_W + (acts.size()-1)*BUTTON_SP;
        float startX = (WIN_W - totalW) / 2.f;
        for (size_t i = 0; i < acts.size(); ++i) {
            float bx = startX + i*(BUTTON_W + BUTTON_SP);
            const std::string actName = acts[i];
            _buttons.push_back(
                makeButton(actName, bx, WIN_H - BUTTON_H - PANEL_PAD,
                    [&, actName]() { onActionClicked(actName); })
            );
        }
    }
}


void GameWindow::drawPlay() {
    _window.clear();
    sf::RectangleShape panel({PANEL_W,(float)WIN_H});
    panel.setFillColor(PANEL_BG); panel.setOutlineColor(sf::Color(100,100,100)); panel.setOutlineThickness(2.f);
    _window.draw(panel);
    for (auto& t:_playerNameTexts) _window.draw(t);
    for (auto& t:_playerCoinTexts) _window.draw(t);
    for (auto& t:_playerRoleTexts) _window.draw(t);
    _window.draw(_turnText); _window.draw(_bankCircle);
    for (auto& b:_buttons) { _window.draw(b.shape); _window.draw(b.label); }
    if (_showTargetDialog) {
        _window.draw(_targetBg);
        for (auto& b:_targetButtons) { _window.draw(b.shape); _window.draw(b.label); }
    }
    _window.display();
}

void GameWindow::drawMenu() {
    _window.clear({20,20,20});
    sf::RectangleShape bar({(float)WIN_W,MENU_H}); bar.setFillColor({40,40,40});
    _window.draw(bar);
    for (auto& b:_menuButtons) { _window.draw(b.shape); _window.draw(b.label); }
    if (_showAddDialog) {
        _window.draw(_dialogBg); _window.draw(_dialogPrompt);
        sf::Text in(_newPlayerName,_font,20);
        in.setFillColor(sf::Color::White);
        in.setPosition(_dialogBg.getPosition()+sf::Vector2f(10,40));
        _window.draw(in);
    }
    if (_popupMessage) {
        if (_popupClock.getElapsedTime().asSeconds()<2.f) {
            _popupText.setString(*_popupMessage);
            _popupText.setPosition((WIN_W-_popupText.getLocalBounds().width)/2.f, MENU_H+PANEL_PAD);
            _window.draw(_popupText);
        } else _popupMessage.reset();
    }
    _window.display();
}

GameWindow::Button GameWindow::makeButton(const std::string& txt, float x, float y, std::function<void()> cb) {
    Button b;
    b.shape.setSize({BUTTON_W,BUTTON_H}); b.shape.setPosition(x,y); b.shape.setFillColor(BUTTON_BG);
    b.label.setFont(_font); b.label.setString(txt); b.label.setCharacterSize(16); b.label.setFillColor(sf::Color::White);
    b.label.setPosition(x+10,y+6);
    b.onClick = std::move(cb);
    return b;
}

void GameWindow::showPopup(const std::string& msg) {
    _popupMessage = msg;
    _popupClock.restart();
}