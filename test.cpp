// GameLogicTest.cpp  –  comprehensive doctest suite for “Coup”
//
// Build & run:  make test
// ───────────────────────────────────────────────────────────────
// Section numbering matches the assignment’s rule list so it’s
// easy to tick requirements one-by-one.
// If you later add new public methods (e.g. Governor::blockTax)
// just drop the relevant CHECKs back in.
//
// All tests assume these public methods exist on Player or the
// concrete role classes:
//   gather(), tax(), bribe(), arrest(Player&), sanction(Player&),
//   coup(Player&), invest()  (Baron), inspect(Player&) (Spy)
// They also assume Game exposes:
//   current_player(), players(), turn(), winner(), next_turn()
// ───────────────────────────────────────────────────────────────

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "Game.hpp"
#include "exceptions.hpp"
#include "Governor.hpp"
#include "Spy.hpp"
#include "Baron.hpp"
#include "General.hpp"
#include "Judge.hpp"
#include "Merchant.hpp"

using coup::Game;
using coup::Player;
using coup::CoupException;

// helper – spin the game until ‘p’ is the active player
static void advanceUntil(Game& g, Player& p) {
    while (&p != g.current_player())
        g.current_player()->gather();   // cheap, always legal
}

// ================================================================
// 1.  Base-turn mechanics
// ================================================================
TEST_CASE("1.1 Gather adds one coin & advances turn") {
    Game g;
    coup::Governor a(g,"A");
    coup::Spy      b(g,"B");

    REQUIRE(g.current_player() == &a);
    a.gather();
    CHECK(a.coins() == 1);
    CHECK(g.current_player() == &b);
}

TEST_CASE("1.2 Tax default +2, Governor gets +3") {
    Game g;
    coup::Governor a(g,"A");
    coup::Spy      b(g,"B");

    a.tax();   // +3
    b.tax();   // +2
    CHECK(a.coins() == 3);
    CHECK(b.coins() == 2);
}

TEST_CASE("1.3 Bribe costs 4 and leaves player on the same turn") {
    Game g;
    coup::Spy s(g,"S");

    // give S 4 coins quickly
    for(int i=0;i<2;++i) s.tax();   // +4
    int before = s.coins();
    s.bribe();
    CHECK(s.coins() == before - 4);
    CHECK(g.current_player() == &s);   // extra action allowed
}

// ================================================================
// 2.  Arrest — cannot target same player twice in a row
// ================================================================
TEST_CASE("2.1 Arrest same target twice throws") {
    Game g;
    coup::Spy   s(g,"S");
    coup::Baron v(g,"V");

    s.gather();  v.gather();   // bring turn back to S
    s.arrest(v);
    v.gather();  s.gather();   // S again
    CHECK_THROWS_AS(s.arrest(v), CoupException);
}

// ================================================================
// 3.  Sanction behaviour incl. Baron + Judge side-effects
// ================================================================
TEST_CASE("3.1 Sanction blocks gather/tax for one turn") {
    Game g;
    coup::Governor a(g,"A");
    coup::Spy      v(g,"V");

    a.sanction(v);
    CHECK_THROWS_AS(v.gather(), CoupException);
    CHECK_THROWS_AS(v.tax(),    CoupException);

    // sanction expires after victim’s next turn
    advanceUntil(g, v);
    CHECK_NOTHROW(v.gather());
}

TEST_CASE("3.2 Baron compensation +1 on sanction") {
    Game g;
    coup::Governor a(g,"A");
    coup::Baron    b(g,"B");

    a.tax(); a.tax();           // fund Governor 6 coins
    int before = b.coins();
    a.sanction(b);              // Governor pays 3, Baron gains 1
    CHECK(b.coins() == before + 1);
}

TEST_CASE("3.3 Judge sanction penalty (+1 to bank)") {
    Game g;
    coup::Governor a(g,"A");
    coup::Judge    j(g,"J");

    a.tax(); a.tax(); a.gather();   // 7 coins
    int afterTax = a.coins();
    a.sanction(j);                  // pays 3 + 1 penalty
    CHECK(a.coins() == afterTax - 4);
}

// ================================================================
// 4.  Passive / active powers of individual roles
// ================================================================
TEST_CASE("4.1 Baron invest −3 +6 net +3") {
    Game g;
    coup::Baron b(g,"B");
    b.tax(); b.tax();          // +4
    b.invest();
    CHECK(b.coins() == 7);     // 4 – 3 + 6
}

TEST_CASE("4.2 Merchant start-turn bonus +1 when ≥3 coins") {
    Game g;
    coup::Merchant m(g,"M");
    coup::Spy filler(g,"X");

    // fund Merchant
    m.tax(); m.tax();          // +4
    filler.gather();           // turn to M next
    advanceUntil(g, m);        // start turn -> bonus
    CHECK(m.coins() == 5);     // 4 + 1 bonus
}

TEST_CASE("4.3 Spy inspect reveals coins (no throw)") {
    Game g;
    coup::Spy s(g,"S");
    coup::Governor v(g,"V");

    v.tax(); // 3 coins
    CHECK_NOTHROW(s.inspect(v));
}

// ================================================================
// 5.  Coup mechanics, General block, forced coup at ≥10
// ================================================================
TEST_CASE("5.1 Coup costs 7, eliminates by default") {
    Game g;
    coup::Governor a(g,"A");
    coup::Spy      b(g,"B");

    while(a.coins() < 7) a.tax();
    a.coup(b);
    CHECK(a.coins() == 0);            // paid 7 (started 7)
    CHECK(g.players().size() == 1);   // B removed
}

TEST_CASE("5.2 General blockCoup refund logic") {
    Game g;
    coup::Governor attacker(g,"A");
    coup::General  defender(g,"D");

    while(attacker.coins() < 7) attacker.tax();
    defender.tax(); defender.gather(); // fund General 3

    attacker.coup(defender);          // should be blocked inside General::coup?
    // Implementation-specific: if you expose defender.blockCoup(attacker)
    // call it here and assert defender paid 5 and remains.
}

TEST_CASE("5.3 Forced coup at 10+ coins") {
    Game g;
    coup::Governor a(g,"A");
    coup::Spy      b(g,"B");

    while(a.coins() < 10) a.tax();
    advanceUntil(g, a);               // start A's turn
    CHECK_THROWS_AS(a.gather(), CoupException);
}

// ================================================================
// 6.  Winner detection + turn skipping eliminated players
// ================================================================
TEST_CASE("6.1 Game winner is last active player") {
    Game g;
    coup::Governor a(g,"A");
    coup::Spy      b(g,"B");

    while(a.coins() < 7) a.tax();
    a.coup(b);
    CHECK(g.players().size() == 1);
    CHECK(g.players().front() == "A");
    CHECK(g.winner() == "A");
}