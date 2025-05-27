// tests.cpp — Complete doctest suite for Coup
// Build & run: make test

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "Game.hpp"
#include "Player.hpp"
#include "exceptions.hpp"
#include "roles/Governor.hpp"
#include "roles/Spy.hpp"
#include "roles/Baron.hpp"
#include "roles/General.hpp"
#include "roles/Judge.hpp"
#include "roles/Merchant.hpp"

using namespace coup;

// helper: spin the game until p’s turn, by doing always-legal gathers
static void advanceUntil(Game& g, coup::Player& p) {
    while (&p != g.current_player()) {
        g.current_player()->gather();
    }
}

//────────────────────────────────────────────────────────
// 1. Core Turn Mechanics
//────────────────────────────────────────────────────────

TEST_CASE("1.1 Add players & track order") {
    Game g;
    Governor a(g, "A");
    Spy      b(g, "B");
    Merchant m(g, "M");
    CHECK(g.players() == std::vector<std::string>{"A","B","M"});
    CHECK(g.current_player()->name() == "A");
}

TEST_CASE("1.2 validate_turn throws out-of-turn") {
    Game g;
    Spy s(g, "S");
    Baron v(g, "V");
    CHECK_THROWS_AS(v.gather(), CoupException);
}

//────────────────────────────────────────────────────────
// 2. Basic Economic Actions
//────────────────────────────────────────────────────────

TEST_CASE("2.1 gather adds one coin & advances turn") {
    Game g;
    Spy   s(g, "S");
    Spy   t(g, "T");
    advanceUntil(g, s);
    s.gather();
    CHECK(s.coins() == 1);
    CHECK(g.current_player() == &t);
}

TEST_CASE("2.2 Tax default +2, Governor +3") {
    Game g;
    Governor a(g, "A");
    Spy      b(g, "B");
    advanceUntil(g, a);
    a.tax();    // +3
    advanceUntil(g, b);
    b.tax();    // +2
    CHECK(a.coins() == 3);
    CHECK(b.coins() == 2);
}

TEST_CASE("2.3 Bribe costs 4 and leaves player on same turn") {
    Game g;
    Spy s(g, "S");
    advanceUntil(g, s);
    // fund to 4 via two default taxes
    for (int i = 0; i < 2; ++i) { s.tax(); advanceUntil(g, s); }
    int before = s.coins();
    s.bribe();
    CHECK(s.coins() == before - 4);
    CHECK(g.current_player() == &s);
}

TEST_CASE("2.4 Bribe without sufficient coins throws") {
    Game g;
    Spy s(g, "S");
    advanceUntil(g, s);
    CHECK_THROWS_AS(s.bribe(), CoupException);
}

//────────────────────────────────────────────────────────
// 3. Arrest & Sanction
//────────────────────────────────────────────────────────

TEST_CASE("3.1 Arrest cannot target same twice") {
    Game g;
    Spy   s(g, "S");
    Baron v(g, "V");
    // fund V to have at least one coin
    advanceUntil(g, v);
    v.gather();
    advanceUntil(g, s);
    s.arrest(v);
    advanceUntil(g, v);
    advanceUntil(g, s);
    CHECK_THROWS_AS(s.arrest(v), CoupException);
}

TEST_CASE("3.2 Sanction blocks gather and tax for one turn") {
    Game g;
    Governor a(g, "A");
    Spy      v(g, "V");
    advanceUntil(g, a);
    // fund A so he can pay the 3-coin sanction cost
    a.tax();               
    advanceUntil(g, a); // +3
    a.sanction(v);
    advanceUntil(g, v);
    CHECK_THROWS_AS(v.gather(), CoupException);
    CHECK_THROWS_AS(v.tax(),    CoupException);
    // next turn the block expires
    advanceUntil(g, a);
    advanceUntil(g, v);
    CHECK_NOTHROW(v.gather());
}

TEST_CASE("3.3 Baron gets +1 compensation on sanction") {
    Game g;
    Governor a(g, "A");
    Baron    b(g, "B");
    // fund A for sanction
    a.tax();               
    advanceUntil(g, a); // +1
    a.sanction(b); //+1
    advanceUntil(g, b);
    // the Baron should have exactly 2
    CHECK(b.coins() == 2);
}

//────────────────────────────────────────────────────────
// 4. Role-specific Abilities
//────────────────────────────────────────────────────────

TEST_CASE("4.1 Baron invest net +3") {
    Game g;
    Baron b(g, "B");
    advanceUntil(g, b);
    // fund via two taxes => +4
    b.tax(); advanceUntil(g, b);
    b.tax(); advanceUntil(g, b);
    b.invest();
    CHECK(b.coins() == 7);
}

TEST_CASE("4.2 Merchant start-of-turn bonus +1 when >=3 coins") {
    Game g;
    Merchant m(g, "M");
    Spy filler(g, "X");
    advanceUntil(g, m);
    // fund via two taxes => +4
    m.tax(); advanceUntil(g, m);
    m.tax(); advanceUntil(g, m);
    // now start-of-turn bonus
    advanceUntil(g, m);
    CHECK(m.coins() == 5);
}

TEST_CASE("4.3 Spy inspect sees coin count") {
    Game g;
    Spy      s(g, "S");
    Governor v(g, "V");
    advanceUntil(g, v);
    v.tax(); advanceUntil(g, v);
    advanceUntil(g, s);
    CHECK_NOTHROW(s.inspect(v));
}

TEST_CASE("4.4 Governor can block tax for one turn") {
    Game g;
    Governor gov(g, "G");
    Spy      s(g, "S");
    advanceUntil(g, gov);
    gov.block_tax(s);
    advanceUntil(g, s);
    CHECK_THROWS_AS(s.tax(), CoupException);
    // block expires after S’s turn
    advanceUntil(g, gov);
    advanceUntil(g, s);
    CHECK_NOTHROW(s.tax());
}

//────────────────────────────────────────────────────────
// 5. Coup Mechanics & General block
//────────────────────────────────────────────────────────

TEST_CASE("5.1 Coup costs 7 and eliminates player") {
    Game g;
    Governor a(g, "A");
    Spy      b(g, "B");
    advanceUntil(g, a);
    // fund to at least 7 via two taxes (+6) and one gather (+1)
    a.tax();    advanceUntil(g, a);
    a.tax();    advanceUntil(g, a);
    a.gather(); advanceUntil(g, a);
    // should now have 7
    a.coup(b);
    CHECK(a.coins() == 0);
    CHECK(g.playerObjects().size() == 1);
}

TEST_CASE("5.2 General can block a coup and gets no refund") {
    Game g;
    Governor atk(g, "A");
    General  def(g, "D");
    advanceUntil(g, atk);
    // fund A to exactly 7: two taxes (+6) and one gather (+1)
    atk.tax();    advanceUntil(g, atk);
    atk.tax();    advanceUntil(g, atk);
    atk.gather(); advanceUntil(g, atk);
    // fund def so he has >=5
    advanceUntil(g, def);
    for(int i=0;i<3;++i){ def.tax(); advanceUntil(g, def); }
    // back to attacker’s turn
    advanceUntil(g, atk);
    atk.coup(def);
    // D should still be in the game (block succeeded)
    auto objs = g.playerObjects();
    CHECK(std::find(objs.begin(), objs.end(), &def) != objs.end());
}

TEST_CASE("5.3 must coup at 10+ coins") {
    Game g;
    Governor a(g, "A");
    Spy      b(g, "B");
    advanceUntil(g, a);
    // fund well above 10: four taxes (+12)
    for(int i=0;i<4;++i){ a.tax(); advanceUntil(g, a); }
    // any further tax should be disallowed (forced coup)
    CHECK_THROWS_AS(a.tax(), CoupException);
}

//────────────────────────────────────────────────────────
// 6. Judge cancel-bribe
//────────────────────────────────────────────────────────

TEST_CASE("6.1 Judge cancels bribe and other player still lose 4 coins") {
    Game g;
    Spy   s(g, "S");
    Judge j(g, "J");
    advanceUntil(g, s);
    // fund to 4
    for (int i = 0; i < 2; ++i) { s.tax(); advanceUntil(g, s); }
    s.bribe(); 
    advanceUntil(g, j);
    j.cancel_bribe(s);
    // S spent 4 and does not get it back
    CHECK(s.coins() == 2);  //6-4 =2
}

//────────────────────────────────────────────────────────
// 7. Winner & Action Log
//────────────────────────────────────────────────────────

TEST_CASE("7.1 Winner is last player") {
    Game g;
    Governor a(g, "A");
    Spy      b(g, "B");
    advanceUntil(g, a);
    // fund to 7 quickly
    for(int i=0;i<4;++i){ a.tax(); advanceUntil(g,a); }
    a.coup(b);
    CHECK(g.winner() == "A");
}

TEST_CASE("7.2 Action log contains gather event") {
    Game g;
    Spy s(g, "S");
    advanceUntil(g, s);
    s.gather();
    auto logs = g.getActionLog();
    REQUIRE(!logs.empty());
    CHECK(logs.back().rfind("S,Gather,Succeeded", 0) == 0);
}