// Email: realyoavperetz@gmail.com
#include "Game.hpp"
#include "roles/Governor.hpp"
#include "roles/Spy.hpp"
#include "roles/Baron.hpp"
#include "roles/General.hpp"
#include "roles/Judge.hpp"

#include "gui/GameWindow.hpp"

int main() {
    using namespace coup;

    Game game;

    Governor governor(game, "Moshe");
    Spy      spy     (game, "Yossi");
    Baron    baron   (game, "Meirav");
    General  general (game, "Reut");
    Judge    judge   (game, "Gilad");

    // Launch the SFML window (blocks until closed)
    coup_gui::GameWindow win(game);
    win.run();

    return 0;
}