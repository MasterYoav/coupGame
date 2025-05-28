# Coup Game Simulation

A C++ implementation of the popular bluffing card game **Coup**, featuring:

* **Core game mechanics** and **role-specific abilities**
* **Action logging** and **exception handling** for illegal moves
* **Unit tests** with [doctest](https://github.com/doctest/doctest)
* **Memory leak checks** via Valgrind
* **Optional GUI** version (using SFML)
* **Headless simulation** for automated validation (no GUI)

---

## Table of Contents

1. [Game Rules](#game-rules)
2. [Project Structure](#project-structure)
3. [Building](#building)
4. [Running Unit Tests](#running-unit-tests)
5. [Memory Leak Checking](#memory-leak-checking)
6. [Headless Simulation](#headless-simulation)
7. [GUI](#gui)
8. [Cleaning Up](#cleaning-up)

---

## Game Rules

Players take turns performing one of several actions, aiming to be the last player remaining. Each player holds a secret role card granting special abilities.

### Common Actions (available to all roles)

* **Gather**: Take 1 coin from the treasury (can be blocked by *Sanction*).
* **Tax**: Take 2 coins (can be blocked by Governor).
* **Bribe**: Spend 4 coins to take an extra action this turn (can be canceled by Judge).
* **Arrest**: Steal 1 coin from another player (cannot target the same player twice in a row; can be blocked by Spy).
* **Sanction**: Pay 3 coins to block a target’s *Gather* and *Tax* until their next turn (Baron gets 1 compensation; Judge costs attacker +1 more).
* **Coup**: Pay 7 coins to eliminate another player (mandatory if holding 10+ coins; can be blocked by General).

### Roles & Abilities

| Role     | Special Power                                                                                         |
| -------- | ----------------------------------------------------------------------------------------------------- |
| Governor | +3 coins on *Tax*; can block another player’s *Tax*.                                                  |
| Spy      | View any player’s coins; can block one *Arrest* against any target.                                   |
| Baron    | Invest: pay 3 coins to gain 6 coins; if sanctioned, gain +1 compensation.                             |
| General  | Pay 5 coins to block a coup against any target; if arrested, regain the stolen coin.                  |
| Judge    | Cancel another player’s *Bribe*, causing them to lose the 4 coins; if sanctioned, attacker pays +1.   |
| Merchant | At the start of turn, if holding ≥3 coins, gain +1 free coin; if arrested, pay 2 to treasury instead. |

---

## Project Structure

```
CoupGame/
├── include/
│   ├── Game.hpp            # Core game engine interface
│   ├── Player.hpp          # Base player class
│   ├── exceptions.hpp      # Custom exception types
│   ├── Action.hpp          # Action type enum
│   └── roles/              # Role-specific headers
│       ├── Governor.hpp
│       ├── Spy.hpp
│       ├── Baron.hpp
│       ├── General.hpp
│       ├── Judge.hpp
│       └── Merchant.hpp
├── src/
│   ├── Game.cpp            # Game logic implementation
│   ├── Player.cpp          # Player base implementation
│   ├── exceptions.cpp      # Exception implementations
│   ├── roles/              # Role-specific implementations
│   │   ├── Governor.cpp
│   │   ├── Spy.cpp
│   │   ├── Baron.cpp
│   │   ├── General.cpp
│   │   ├── Judge.cpp
│   │   └── Merchant.cpp
│   └── gui/                # Optional GUI components (SFML)
│       ├── GameWindow.cpp
│       └── GameWindow.hpp
├── tests.cpp               # Complete doctest suite
├── main.cpp                # Entry point for GUI-enabled version
├── main_for_valgrind.cpp   # Headless simulation for Valgrind
├── Makefile                # Build & test targets
└── README.md               # This file
```

---

## Building

Ensure you have a C++20 compiler (e.g., `g++`) and [SFML](https://www.sfml-dev.org/) if you plan to use the GUI.

```bash
# Build everything (tests, GUI-executable, valgrind runner):
make all
```

---

## Running Unit Tests

```bash
make test
```

* Compiles `tests.cpp` with all core sources
* Runs the full doctest suite

---

## Memory Leak Checking

```bash
make valgrind
```

* Builds the tests runner and `main_for_valgrind.cpp` simulation
* Runs both under Valgrind with `--leak-check=full`

---

## Headless Simulation

To manually run the non-GUI simulation:

```bash
./game_val
```

It exercises every action and block path, then runs until a winner is declared.

---

## GUI

If SFML is installed, you can build and run the GUI version:

```bash
# Assuming SFML library paths are set in the Makefile
make gui
./CoupGameGUI
```

---

## Cleaning Up

```bash
make clean
```

Removes all build artifacts (`tests_val`, `game_val`, GUI binaries, object files).

---

Enjoy playing and experimenting with **Coup**! Contributions and improvements are welcome. 🚀
