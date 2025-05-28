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

TEST_CASE("3.2 Sanction blocks gather & tax for ONE FULL TURN only") {
    Game      g;
    Governor  a(g,"A");   // will sanction
    Spy       v(g,"V");   // victim of sanction
    Baron     x(g,"X");   // third player – provides an arrest target

    /* fund players -------------------------------------------------- */
    advanceUntil(g,x);            // X gathers 1 coin (needed for arrest)
    x.gather();                   // X=1

    advanceUntil(g,a);            // A gets 3 coins for sanction cost
    a.tax();                      // A coins = 3

    /* A sanctions V -------------------------------------------------- */
    advanceUntil(g,a);
    a.sanction(v);                // current player becomes V

    /* V’s *first* turn – tax/gather must be blocked ----------------- */
    REQUIRE(g.current_player() == &v);
    CHECK_THROWS_AS(v.gather(), CoupException);
    CHECK_THROWS_AS(v.tax   (), CoupException);

    /* V does a *legal* action (arrest) to finish the turn ------------ */
    REQUIRE_NOTHROW(v.arrest(x));       // steals the 1 coin from X

    /* now cycle back to V again -------------------------------------- */
    advanceUntil(g,v);            // A → X → back to V

    /* sanction should be cleared; gather/tax succeed ----------------- */
    CHECK_NOTHROW(v.gather());    // NO throw now
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

TEST_CASE("5.2 General can block a coup (pays 5, attacker still pays 7)") {
    Game      g;
    General   def(g,"D");   // defender / blocker
    Governor  atk(g,"A");   // attacker
    

    /* ---- Stage 1: fund defender enough to block & attacker enough to coup*/\
    def.tax(); atk.gather();  // def=2, atk=1

    /* ---- Stage 2: fund attacker enough to coup and KEEP turn --------- */
    def.gather(); // (def=3)
    atk.tax();   def.gather();   // (def=4 atk=4)
    atk.tax();   def.gather();   // (def=5 atk=7)
    REQUIRE(g.current_player() == &atk);
   
    /* ---- Stage 3: attempt coup – defender blocks ----------------- */
    
    CHECK(atk.coins() == 7);
    CHECK(def.coins() == 5);
    atk.coup(def); 
    CHECK(atk.coins() == 0);
    CHECK(g.current_player() == &atk);
    def.block_coup(def);

    /* check that defender still alive ------------------------------------------- */
    auto alive = g.playerObjects();
    CHECK(std::find(alive.begin(), alive.end(), &def) != alive.end());

    /* coin balances after payments ----------------------------------- */
    CHECK(atk.coins() == 0);   // paid 7
    CHECK(def.coins() == 0);   // 
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
/* 6.2 Spy blocks an Arrest for one turn -----------------------------*/
TEST_CASE("6.2 Spy can block an arrest against him") {
    Game g;
    Spy      spy(g,"S");
    Governor atk(g,"A");

    advanceUntil(g, atk);
    atk.gather();               // fund attacker 1
    advanceUntil(g, spy);
    spy.block_arrest(atk);      // reaction flag set on attacker

    advanceUntil(g, atk);
    REQUIRE_THROWS_AS(atk.arrest(spy), CoupException);   // blocked
    // next turn, arrest allowed again
    advanceUntil(g, spy);
    advanceUntil(g, atk);
    REQUIRE_NOTHROW(atk.arrest(spy));
}

/* 6.3 General refunded when arrested --------------------------------*/
TEST_CASE("6.3 General regains coin when arrested") {
    Game g;
    Spy      s(g,"S");
    General  gen(g,"G");
    advanceUntil(g, gen); //s=1
    gen.gather();               // gen= 1
    s.arrest(gen); advanceUntil(g,gen);            // steal 1 then General ,but- refund rule should still keep him 1
    CHECK(gen.coins() == 1);    // got it back
    CHECK(s.coins()  == 2);     // did NOT keep it
}

/* 6.4 Merchant pays 2 to bank when arrested -------------------------*/
TEST_CASE("6.4 Merchant loses 2 to bank when arrested") {
    Game g;
    Spy       s(g,"S");
    Merchant  m(g,"M");
    advanceUntil(g, m); 
    m.tax();                    // m +2
    advanceUntil(g, s);
    s.arrest(m);                // deduct 2 from Merchant 
    CHECK(m.coins() == 0);      //Merchant have 0
    CHECK(s.coins() == 1);      // Spy does not gain
}

/* 6.5 Judge extra-penalty when sanctioned ---------------------------*/
TEST_CASE("6.5 Sanctioning a Judge costs attacker 4") {
    Game g;
    Governor atk(g,"A");
    Judge    j(g,"J");
    advanceUntil(g, atk);
    atk.tax();  // +3 coins exactly
    CHECK_THROWS_AS(atk.sanction(j), CoupException); // needs 4
    advanceUntil(g, atk);
    atk.tax();  // now 5
    advanceUntil(g, atk);
    atk.sanction(j);   // pays 4 to bank
    CHECK(atk.coins() == 1);
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