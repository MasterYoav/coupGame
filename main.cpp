// Email: realyoavperetz@gmail.com
#include "Game.hpp"
#include "roles/Governor.hpp"
#include "roles/Spy.hpp"
#include "roles/Baron.hpp"
#include "roles/General.hpp"
#include "roles/Judge.hpp"

#include "gui/GameWindow.hpp"

int main() {
    coup::Game game;
    coup_gui::GameWindow win(game);
    win.run();
    return 0;
}