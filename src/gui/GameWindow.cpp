// Email: realyoavperetz@gmail.com
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

// ─────────────── Layout constants ───────────────

// Window dimensions
static constexpr unsigned WIN_W    = 1000;   // total window width
static constexpr unsigned WIN_H    = 800;   // total window height

// Log sub-window height (appears beneath the main window when playing)
static constexpr unsigned LOG_H    = 200;

// Header bar
static constexpr float MENU_H      = 60.f;  // height of the top menu/title bar

// Side panel (player list) dimensions
static constexpr float PANEL_W     = 300.f; // width of the left side panel
static constexpr float PANEL_PAD   = 15.f;  // padding inside panels and dialogs

// Action button dimensions & spacing
static constexpr float BUTTON_W    = 100.f; // width of each action button
static constexpr float BUTTON_H    = 40.f;  // height of each action button
static constexpr float BUTTON_SP   = 20.f;  // horizontal spacing between buttons

// Colors
static const sf::Color PANEL_BG    (120, 119, 119);   // side‐panel background
static const sf::Color BUTTON_BG   (80,  80, 200);   // action‐button background
static const sf::Color TEXT_COLOR  (230,230,230);   // general text color
static const sf::Color DIALOG_BG   (50,  50,  60, 230); // modal dialog background (semi‐transparent)
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
        // 1) Window close
        if (e.type == sf::Event::Closed) {
            _window.close();
            return;
        }

        // 2) “Change Role” buttons in Menu
        if (e.type == sf::Event::MouseButtonPressed) {
            sf::Vector2f m(e.mouseButton.x, e.mouseButton.y);
            for (size_t i = 0; i < _roleButtons.size(); ++i) {
                if (_roleButtons[i].shape.getGlobalBounds().contains(m)) {
                    _roleButtons[i].onClick();
                    return;  // consume click
                }
            }
        }

        // 3) Main menu buttons (“Add Player”, “Start Game”)
        if (!_showAddDialog && e.type == sf::Event::MouseButtonPressed) {
            sf::Vector2f m(e.mouseButton.x, e.mouseButton.y);
            for (auto& b : _menuButtons) {
                if (b.shape.getGlobalBounds().contains(m)) {
                    b.onClick();
                    return;
                }
            }
        }

        // 4) Text entry for the Add-Player dialog
        if (_showAddDialog && e.type == sf::Event::TextEntered) {
            char c = static_cast<char>(e.text.unicode);

            if (c == 8 && !_newPlayerName.empty()) {
                // backspace
                _newPlayerName.pop_back();
            }
            else if (c == '\r' || c == '\n') {
                // ENTER → attempt to add the player

                // a) duplicate‐name check
                for (auto& nm : _game.players()) {
                    if (nm == _newPlayerName) {
                        showPopup("Name is taken");
                        _showAddDialog = false;
                        return;
                    }
                }

                // b) pick a random role
                static const std::vector<std::string> ALL = {
                    "Governor","Spy","Baron","General","Judge","Merchant"
                };
                std::uniform_int_distribution<size_t> dist(0, ALL.size() - 1);
                const std::string role = ALL[dist(rng)];

                // c) try to construct the new player (catches >6 players)
                try {
                    if      (role == "Governor") new Governor(_game, _newPlayerName);
                    else if (role == "Spy")      new Spy(_game, _newPlayerName);
                    else if (role == "Baron")    new Baron(_game, _newPlayerName);
                    else if (role == "General")  new General(_game, _newPlayerName);
                    else if (role == "Judge")    new Judge(_game, _newPlayerName);
                    else                         new Merchant(_game, _newPlayerName);

                    showPopup("Added " + _newPlayerName + " as " + role);
                }
                catch (const CoupException& ex) {
                    // for example: “Game already has 6 players”
                    showPopup(ex.what());
                }

                // d) close the dialog so it disappears
                _showAddDialog = false;
                return;
            }
            else if (c >= 32 && c < 127) {
                // printable character
                _newPlayerName.push_back(c);
            }
        }
    }
}

void GameWindow::handlePlayEvents() {
    sf::Event e;
    while (_window.pollEvent(e)) {
        // Coup‐block dialog (General)
        if (_showBlockCoupDialog) {
            if (e.type == sf::Event::MouseButtonPressed) {
                sf::Vector2f m(e.mouseButton.x, e.mouseButton.y);
                for (auto& b : _blockCoupButtons) {
                    if (b.shape.getGlobalBounds().contains(m)) {
                        b.onClick();
                        break;
                    }
                }
            }
            continue;  // swallow all other clicks until choice made
        }

         // Judge's Cancel Bribe dialog
         if (_showBlockBribeDialog) {
            if (e.type == sf::Event::MouseButtonPressed) {
                sf::Vector2f m(e.mouseButton.x, e.mouseButton.y);
                for (auto& b : _blockBribeButtons) {
                    if (b.shape.getGlobalBounds().contains(m)) {
                        b.onClick();
                        break;
                    }
                }
            }
            continue;
        }

        // 3) Normal event flow
        if (e.type == sf::Event::Closed) {
            _window.close();
        }
        else if (_showTargetDialog && e.type == sf::Event::MouseButtonPressed) {
            sf::Vector2f m(e.mouseButton.x, e.mouseButton.y);
            for (auto& b : _targetButtons) {
                if (b.shape.getGlobalBounds().contains(m)) {
                    b.onClick();
                    break;
                }
            }
        }
        else if (!_showTargetDialog && e.type == sf::Event::MouseButtonPressed) {
            sf::Vector2f m(e.mouseButton.x, e.mouseButton.y);
            for (auto& b : _buttons) {
                if (b.shape.getGlobalBounds().contains(m)) {
                    b.onClick();
                    break;
                }
            }
        }
    }
}

void GameWindow::onActionClicked(const std::string& act) {
    try {
        coup::Player* cp = _game.current_player();
        if (!cp) return;
        // Mandatory coup: if you have ≥10 coins, you must coup
        // mandatory coup if ≥10 coins, but allow Spy peek
        if (cp->coins() >= 10 && act != "Coup" && !(cp->role() == "Spy" && (act == "Show Coins" || act == "Hide Coins"))){
            showPopup("Must coup when holding 10 or more coins");return;}
        if (act == "Gather")          { cp->gather(); }
        else if (act == "Tax")        { cp->tax(); }
        else if (act == "Bribe") {_pending = PendingAct::Bribe;executePendingAction(nullptr);return;}
        else if (act == "Arrest")     { createTargetDialog(PendingAct::Arrest); return; }
        else if (act == "Sanction")   { createTargetDialog(PendingAct::Sanction); return; }
        else if (act == "Coup")       { if (cp->coins() < 7) {
                        showPopup("Not enough coins to coup");
                    } else {
                        _pending = PendingAct::Coup;
                        createTargetDialog(PendingAct::Coup);
                    } return; }
        else if (act == "Block Tax") { createTargetDialog(PendingAct::BlockTax); return; }
        else if (act == "Block Arrest") {createTargetDialog(PendingAct::BlockArrest);return;}
        else if (act == "Show Coins") {_showSpyBalances = true;return;}
        else if (act == "Hide Coins") {_showSpyBalances = false;return;}
        else if (act == "Invest") {static_cast<Baron*>(cp)->invest();showPopup(cp->name() + " invested 3 coins for 6");return;}
        else if (act == "BlockCoup"){createTargetDialog(PendingAct::BlockCoup);return;}
        
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
void GameWindow::createBlockCoupDialog(coup::Player* general) {
    _blockCoupButtons.clear();

    // compute y position of that general in the side‐panel
    int idx = 0;
    for (auto* p : _game.playerObjects()) {
        if (p == general) break;
        ++idx;
    }
    float y = MENU_H + PANEL_PAD + idx * 50.f;

    // “Block Coup” (red button)
    {
        auto btn = makeButton(
            "Block Coup",
            PANEL_PAD + PANEL_W + 20.f,
            y,
            [this, general]() {
                // General pays 5
                static_cast<General*>(general)->spend(5);
                _game.bank() += 5;
                // Attacker still loses 7
                _pendingCoupAttacker->spend(7);
                _game.bank() += 7;

                showPopup(general->name() + " blocked the coup");
                _showBlockCoupDialog = false;
                _pending = PendingAct::None;
            }
        );
        btn.shape.setFillColor(sf::Color(200,  0,  0));
        _blockCoupButtons.push_back(std::move(btn));
    }

    // “Continue” (green button)
    {
        auto btn = makeButton(
            "Continue",
            PANEL_PAD + PANEL_W + 20.f + BUTTON_W + BUTTON_SP,
            y,
            [this]() {
                _pendingCoupAttacker->coup(*_pendingCoupTarget);
                showPopup(_pendingCoupAttacker->name()
                          + " performed a coup on "
                          + _pendingCoupTarget->name());
                _showBlockCoupDialog = false;
                _pending = PendingAct::None;
            }
        );
        btn.shape.setFillColor(sf::Color(  0,200,  0));
        _blockCoupButtons.push_back(std::move(btn));
    }
}
void GameWindow::createBlockBribeDialog(coup::Player* judge) {
    _blockBribeButtons.clear();

    // find the judge’s Y position in the side panel
    int idx = 0;
    for (auto* p : _game.playerObjects()) {
        if (p == judge) break;
        ++idx;
    }
    float y = MENU_H + PANEL_PAD + idx * 50.f;

    // RED “Cancel Bribe”: attacker loses their 4 coins (bank gains them)
    {
        auto btn = makeButton(
            "Cancel Bribe",
            PANEL_PAD + PANEL_W + 20.f, y,
            [this]() {
                _pendingBribeAttacker->spend(4);
                _game.bank() += 4;
                showPopup(
                    _pendingBribeAttacker->name() + "'s bribe canceled"
                );
                _showBlockBribeDialog = false;
            }
        );
        btn.shape.setFillColor(sf::Color(200,  0,  0)); // RED
        _blockBribeButtons.push_back(std::move(btn));
    }

    // GREEN “Continue”: let the bribe stand
    {
        auto btn = makeButton(
            "Continue",
            PANEL_PAD + PANEL_W + 20.f + BUTTON_W + BUTTON_SP, y,
            [this]() {
                _pendingBribeAttacker->bribe();
                showPopup(
                    _pendingBribeAttacker->name() + " bribed for an extra action"
                );
                _showBlockBribeDialog = false;
            }
        );
        btn.shape.setFillColor(sf::Color(  0,200,  0)); // GREEN
        _blockBribeButtons.push_back(std::move(btn));
    }
}
void GameWindow::executePendingAction(coup::Player* target) {
    try {
        auto* cp = _game.current_player();
        if (!cp) return;

        switch (_pending) {
            case PendingAct::None:
                // Nothing to do
                _pending = PendingAct::None;
                return;

            case PendingAct::Arrest: {
                _showTargetDialog = false;
                cp->arrest(*target);
                showPopup(cp->name() + " arrested " + target->name());
                _pending = PendingAct::None;
                return;
            }

            case PendingAct::Sanction: {
                _showTargetDialog = false;
                cp->sanction(*target);
                showPopup(cp->name() + " sanctioned " + target->name());
                _pending = PendingAct::None;
                return;
            }

            case PendingAct::Bribe: {
                // validate before prompting Judge
                try {
                    _game.validate_turn(cp);
                    if (_game.is_bribe_blocked(cp)) throw CoupException("Can't bribe – you are blocked");
                    if (cp->coins() < 4)               throw CoupException("Not enough coins to bribe");
                } catch (const CoupException& ex) {
                    showPopup(ex.what());
                    _pending = PendingAct::None;
                    return;
                }
                // store attacker and clear pending
                _pendingBribeAttacker = cp;
                _pending = PendingAct::None;
                // see if an *other* Judge can cancel
                for (auto* p : _game.playerObjects()) {
                    if (p->role() == "Judge" && p != cp) {
                        createBlockBribeDialog(p);
                        _showBlockBribeDialog = true;
                        return;
                    }
                }
                // no Judge → execute bribe now
                cp->bribe();
                showPopup(cp->name() + " bribed for an extra action");
                return;
            }

            case PendingAct::Coup: {
                // store attacker/defender, clear pending & dialog
                auto* attacker = cp;
                auto* defender = target;
                _pendingCoupAttacker = attacker;
                _pendingCoupTarget = defender;
                _showTargetDialog = false;
                _pending          = PendingAct::None;

                // see if an *other* General can block
                for (auto* p : _game.playerObjects()) {
                    if (p != attacker && p->role() == "General" && p->coins() >= 5) {
                        createBlockCoupDialog(p);
                        _showBlockCoupDialog = true;
                        return;
                    }
                }
                // no block → manual coup
                attacker->spend(7);
                _game.bank() += 7;
                _game.register_action(attacker, ActionType::Coup, defender, true);
                _game.eliminate(defender);
                _game.next_turn();
                showPopup(attacker->name() + " performed a coup on " + defender->name());
                return;
            }

            case PendingAct::BlockTax: {
                _showTargetDialog = false;
                static_cast<Governor*>(cp)->block_tax(*target);
                showPopup(cp->name() + " blocked Tax for " + target->name());
                _pending = PendingAct::None;
                return;
            }

            case PendingAct::BlockArrest: {
                _showTargetDialog = false;
                static_cast<Spy*>(cp)->block_arrest(*target);
                showPopup(cp->name() + " blocked Arrest for " + target->name());
                _pending = PendingAct::None;
                return;
            }

            case PendingAct::BlockCoup:
            case PendingAct::BlockBribe: {
                // These are handled entirely in their create*Dialog() lambdas,
                // so we just reset the pending state here.
                _pending = PendingAct::None;
                return;
            }

            // No default: we’ve covered every value above
        }
    } catch (const CoupException& ex) {
        // If anything throws unexpectedly, clear all dialogs & show the error
        _showTargetDialog      = false;
        _showBlockCoupDialog   = false;
        _showBlockBribeDialog  = false;
        showPopup(ex.what());
        _pending = PendingAct::None;
    }
}


// Rebuild buttons dynamically each turn based on role
void GameWindow::updatePlayLayout() {
    _playerNameTexts.clear();
    _playerCoinTexts.clear();
    _playerRoleTexts.clear();

    // Side-panel texts (unchanged)
    float y = MENU_H + PANEL_PAD;
    coup::Player* current = _game.current_player();
    for (auto* p : _game.playerObjects()) {
    bool isCurrent = (p == current);

    // Player name
    sf::Text name(p->name(), _font, 18);
    name.setFillColor(isCurrent ? sf::Color::Yellow : sf::Color::White);
    name.setStyle(isCurrent ? sf::Text::Bold : sf::Text::Regular);
    name.setPosition(PANEL_PAD, y);
    _playerNameTexts.push_back(name);

    // Player role
    sf::Text role(p->role(), _font, 18);
    role.setFillColor(sf::Color::Blue);
    role.setPosition(PANEL_PAD + 200.f, y);
    _playerRoleTexts.push_back(role);


    // Player coins: show only for current player, or when Spy has revealed
    std::string coinStr;
    if (isCurrent || (current->role() == "Spy" && _showSpyBalances)) {
        coinStr = std::to_string(p->coins());
    }
    sf::Text coins(coinStr, _font, 18);
    coins.setFillColor(sf::Color(212,175,55));
    coins.setPosition(PANEL_PAD + 120.f, y);
    _playerCoinTexts.push_back(coins);

    
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
        if (current->role() == "Spy"){acts.push_back("Block Arrest");acts.push_back(_showSpyBalances ? "Hide Coins" : "Show Coins");}
        if (current->role() == "Baron")    acts.push_back("Invest");
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


// ──────────────────── Patched drawPlay ────────────────────
void GameWindow::drawPlay() {
    _window.clear();

    // Side panel
    sf::RectangleShape panel({PANEL_W, float(WIN_H)});
    panel.setFillColor(PANEL_BG);
    panel.setOutlineColor(sf::Color(100,100,100));
    panel.setOutlineThickness(2.f);
    _window.draw(panel);

    // Player stats
    for (auto& t : _playerNameTexts)  _window.draw(t);
    for (auto& t : _playerCoinTexts)  _window.draw(t);
    for (auto& t : _playerRoleTexts)  _window.draw(t);

    // Turn text & bank
    _window.draw(_turnText);
    _window.draw(_bankCircle);

    // Action buttons
    for (auto& b : _buttons) {
        _window.draw(b.shape);
        _window.draw(b.label);
    }

    // Target-select dialog
    if (_showTargetDialog) {
        _window.draw(_targetBg);
        for (auto& b : _targetButtons) {
            _window.draw(b.shape);
            _window.draw(b.label);
        }
    }

    // Block-Coup or block-Bribe overlays
    if (_showBlockCoupDialog) {
        for (auto& b : _blockCoupButtons) {
            _window.draw(b.shape);
            _window.draw(b.label);
        }
        _window.display();
        return;
    }
    if (_showBlockBribeDialog) {
        for (auto& b : _blockBribeButtons) {
            _window.draw(b.shape);
            _window.draw(b.label);
        }
        _window.display();
        return;
    }

    // Draw in-game pop-ups *under* the bank coin:
    if (_popupMessage) {
        if (_popupClock.getElapsedTime().asSeconds() < 2.f) {
            _popupText.setString(*_popupMessage);
            // position directly below bankCircle
            sf::Vector2f bankPos = _bankCircle.getPosition();
            float radius = _bankCircle.getRadius();
            float px = bankPos.x;                          // left edge of coin
            float py = bankPos.y + 2*radius + PANEL_PAD;   // just below
            _popupText.setPosition(px, py);
            _window.draw(_popupText);
        } else {
            _popupMessage.reset();
        }
    }

    _window.display();
}

// Patched drawMenu() with pop-ups above the player list and dynamic role updates
void GameWindow::drawMenu() {
    _window.clear({20,20,20});

    // ── Top bar & main buttons ──
    sf::RectangleShape bar({static_cast<float>(WIN_W), MENU_H});
    bar.setFillColor(PANEL_BG);
    _window.draw(bar);
    for (auto& b : _menuButtons) {
        _window.draw(b.shape);
        _window.draw(b.label);
    }

    // ── Pop-up messages (above the list) ──
    if (_popupMessage) {
        if (_popupClock.getElapsedTime().asSeconds() < 2.f) {
            _popupText.setString(*_popupMessage);
            float px = PANEL_PAD;
            float py = MENU_H + PANEL_PAD;  // just below the menu bar
            _popupText.setPosition(px, py);
            _window.draw(_popupText);
        } else {
            _popupMessage.reset();
        }
    }

    // ── Player list with live role & Change Role buttons ──
    float y = MENU_H + PANEL_PAD * 2;  // leave space for pop-up
    _roleButtons.clear();
    for (coup::Player* p : _game.playerObjects()) {
        // Name
        sf::Text nameTxt(p->name(), _font, 18);
        nameTxt.setFillColor(TEXT_COLOR);
        nameTxt.setPosition(PANEL_PAD, y);
        _window.draw(nameTxt);

        // Live Role
        sf::Text roleTxt(p->role(), _font, 18);
        roleTxt.setFillColor(sf::Color::Blue);
        roleTxt.setPosition(PANEL_PAD + 150.f, y);
        _window.draw(roleTxt);

        // Change Role button
        Button btn = makeButton(
            "Change Role", PANEL_PAD + 300.f, y - 4.f,
            [this, p]() {
                static const std::vector<std::string> ALL = {
                    "Governor","Spy","Baron","General","Judge","Merchant"
                };
                static std::mt19937 rng(std::random_device{}());
                std::uniform_int_distribution<size_t> dist(0, ALL.size()-1);
                std::string newRole = ALL[dist(rng)];
                p->change_role(newRole);
                showPopup(p->name() + " is now a " + newRole);
            }
        );
        _roleButtons.push_back(btn);
        _window.draw(btn.shape);
        _window.draw(btn.label);

        y += 30.f;
    }

    // ── Add-Player dialog ──
    if (_showAddDialog) {
        _window.draw(_dialogBg);
        _window.draw(_dialogPrompt);
        sf::Text in(_newPlayerName, _font, 20);
        in.setFillColor(sf::Color::White);
        in.setPosition(_dialogBg.getPosition() + sf::Vector2f(10.f, 40.f));
        _window.draw(in);
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