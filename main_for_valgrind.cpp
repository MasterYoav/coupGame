// Email: realyoavperetz@gmail.com

#include "Game.hpp"
#include "roles/Spy.hpp"
#include "roles/Governor.hpp"

static void advanceUntil(coup::Game &g, coup::Player &p) {
    while (&p != g.current_player()) {
        g.current_player()->gather();
    }
}

int main() {
    coup::Game g;
    coup::Spy     s(g, "S");
    coup::Governor gv(g, "G");

    // 1) Fund Spy to 2 coins, Governor to 2 coins
    advanceUntil(g, s);
    s.gather();  // S = 1
    advanceUntil(g, gv);
    gv.gather(); // G = 1
    advanceUntil(g, s);
    s.gather();  // S = 2
    advanceUntil(g, gv);
    gv.gather(); // G = 2

    // 2) A few more gathers to exercise turn rotation
    for (int i = 0; i < 5; ++i) {
        advanceUntil(g, s);
        s.gather();
        advanceUntil(g, gv);
        gv.gather();
    }

    return 0;
}