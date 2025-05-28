// Email: realyoavperetz@gmail.com

#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <functional>
#include <optional>

namespace coup { class Game; class Player; }

namespace coup_gui {

class GameWindow {
public:
    explicit GameWindow(coup::Game& game);
    void run();  // main loop

private:
    struct Button {
        sf::RectangleShape shape;
        sf::Text           label;
        std::function<void()> onClick;
    };

    // Event & draw
    void handleMenuEvents();
    void drawMenu();
    void handlePlayEvents();
    void updatePlayLayout();
    void drawPlay();

    // Action dispatch
    void onActionClicked(const std::string& act);

    // Target dialog
    enum class PendingAct { None,Bribe, Arrest, Sanction, Coup, BlockTax, BlockArrest,BlockCoup,BlockBribe};
    void createTargetDialog(PendingAct act);
    void executePendingAction(coup::Player* target);
    
    // Buttons for “Change Role” in the Menu screen
    std::vector<Button>       _roleButtons;
    std::vector<coup::Player*> _roleButtonTargets;

    // Winner dialog
    void createWinnerDialog(const std::string& winner);

    // UI helpers
    Button makeButton(const std::string& txt, float x, float y, std::function<void()> cb);
    void showPopup(const std::string& msg);

    // Data
    coup::Game&               _game;
    sf::RenderWindow          _window;
    sf::Font                  _font;

    // Menu
    std::vector<Button>       _menuButtons;
    bool                      _showAddDialog{false};
    sf::RectangleShape        _dialogBg;
    sf::Text                  _dialogPrompt;
    std::string               _newPlayerName;

    // Play
    enum class WindowState { Menu, Playing };
    WindowState               _state{WindowState::Menu};
    std::vector<Button>       _buttons;
    sf::CircleShape           _bankCircle;
    sf::Text                  _turnText;
    std::vector<sf::Text>     _playerNameTexts;
    std::vector<sf::Text>     _playerCoinTexts;
    std::vector<sf::Text>     _playerRoleTexts;

    // Popup
    std::optional<std::string> _popupMessage;
    sf::Clock                  _popupClock;
    sf::Text                   _popupText;

    // --- Target-selection dialog ---
    bool    _showTargetDialog{false};
    bool    _showSpyBalances{false};    // Spy special: when true, show every player’s coin count
    PendingAct _pending{PendingAct::None};
    sf::RectangleShape _targetBg;
    std::vector<Button> _targetButtons;

    // Winner-dialog
    bool                       _showWinnerDialog{false};
    sf::RectangleShape         _winnerBg;
    sf::Text                   _winnerText;
    std::vector<Button>        _winnerButtons;

   // General’s real-time block-coup prompt
    coup::Player*      _pendingCoupAttacker = nullptr;
    coup::Player*      _pendingCoupTarget   = nullptr;
    bool               _showBlockCoupDialog = false;
    std::vector<Button> _blockCoupButtons; 
    
    void createBlockCoupDialog(coup::Player* general);

    // Judge’s cancel-bribe prompt 
    coup::Player*      _pendingBribeAttacker  = nullptr;
    coup::Player*      _pendingBribeTarget    = nullptr;
    bool               _showBlockBribeDialog  = false;
    std::vector<Button> _blockBribeButtons;
    void createBlockBribeDialog(coup::Player* judge);
};

} // namespace coup_gui
