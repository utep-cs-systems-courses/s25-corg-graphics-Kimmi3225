# MSP430 Memory Game

## Project Overview

This project is a memory game implemented on the MSP430 microcontroller using an LCD screen and input buttons. Players must watch a sequence of shapes displayed on the screen and then reproduce the sequence correctly using buttons. As the game progresses the sequence grows longer challenging the player's memory.

The game includes colorful shape displays, multiple levels, a settings menu with palette customization, and dynamic game states.

---

## How the Game Works

- **Start the Game**: Press **Button 1 (P2.0)** to begin.
- **Watch the Sequence**: A random sequence of shapes is shown (small square, big square, triangle).
- **Repeat the Sequence**: Press the correct shape buttons in order:
  - **Button 2 (P2.1)** → Small Square
  - **Button 3 (P2.2)** → Big Square
  - **Button 4 (P2.3)** → Triangle
- **Pass**: Successfully repeating the sequence progresses to the next level (sequence grows).
- **Fail**: A wrong input ends the round and returns to the main menu with a new sequence.

---

## Features

- Randomized shape sequence using hardware timer (`TAR`)
- **Customizable color palettes** via the settings menu
- Main menu with title and shape previews
- Input validation using an assembly routine for efficient comparison
- Uses a finite state machine:
  - `WAITING_TO_START`
  - `SHOWING_SEQUENCE`
  - `INPUT_PHASE`
  - `LEVEL_COMPLETE_MENU`
  - `SETTINGS_MENU`


## File Structure

- `memorygame.c` - Main game logic, drawing, and state transitions
- `test1.s` - Assembly function that checks player input against the correct sequence
- `wdt_handler.c` - Optional watchdog timer handler
- `README.md` - Project documentation

---

## How to Build

```bash
msp430-elf-gcc -mmcu=msp430g2553 -Os -I../h -L../lib -o memorygame.elf memorygame.o wdt_handler.o test1.o -lTimer -lLcd
