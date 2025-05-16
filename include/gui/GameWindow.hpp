// Email: realyoavperetz@gmail.com
// GameWindow.hpp — header for the GUI controller (patched version)
// * English-only comments
// * Adds support for target-selection actions (Arrest / Sanction / Coup)
// * Prevents duplicate roles

#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <functional>
#include <optional>

namespace coup {      class Game;  class Player; }

namespace coup_gui {

class GameWindow {
public:
    explicit GameWindow(coup::Game& game);
    void run();                                // main event/render loop

private:
    // helper struct for simple rectangle buttons
    struct Button {
        sf::RectangleShape shape;
        sf::Text           label;
        std::function<void()> onClick;
    };

    //----------------------- event / draw helpers -----------------------
    void handleMenuEvents();
    void drawMenu();

    void handlePlayEvents();
    void updatePlayLayout();
    void drawPlay();

    // action dispatch
    void onActionClicked(const std::string& act);

    //----------------------- target-dialog helpers ----------------------
    enum class PendingAct { None, Arrest, Sanction, Coup };
    void createTargetDialog(PendingAct act);           // build overlay
    void executePendingAction(coup::Player* target);   // run after click

    //----------------------- UI helpers --------------------------------
    Button makeButton(const std::string& txt,
                      float x, float y,
                      std::function<void()> cb);
    void showPopup(const std::string& msg);

    //----------------------- data --------------------------------------
    coup::Game& _game;

    sf::RenderWindow _window;
    sf::Font         _font;

    // --- MENU state ---
    std::vector<Button> _menuButtons;
    bool                _showAddDialog{false};
    sf::RectangleShape  _dialogBg;
    sf::Text            _dialogPrompt;
    std::string         _newPlayerName;

    // --- PLAY state ---
    enum class WindowState { Menu, Playing };
    WindowState         _state{WindowState::Menu};

    std::vector<Button> _buttons;              // action buttons
    sf::CircleShape     _bankCircle;
    sf::Text            _turnText;

    std::vector<sf::Text> _playerNameTexts;
    std::vector<sf::Text> _playerCoinTexts;
    std::vector<sf::Text> _playerRoleTexts;

    // --- Popup ---
    std::optional<std::string> _popupMessage;
    sf::Clock                  _popupClock;
    sf::Text                   _popupText;

    // --- Target-selection dialog ---
    bool                       _showTargetDialog{false};
    PendingAct                 _pending{PendingAct::None};
    sf::RectangleShape         _targetBg;
    std::vector<Button>        _targetButtons;
};

} // namespace coup_gui