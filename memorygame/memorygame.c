#include <msp430.h>
#include <libTimer.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "lcdutils.h"
#include "lcddraw.h"

// Game Constants
#define MAX_SEQUENCE_LENGTH 8
#define NUM_SHAPES 3

#define BTN_START BIT0  // Button 1 (P2.0)
#define BTN_SMALL BIT1  // Button 2 (P2.1)
#define BTN_BIG   BIT2  // Button 3 (P2.2)
#define BTN_TRI   BIT3  // Button 4 (P2.3)
#define SWITCHES (BTN_START | BTN_SMALL | BTN_BIG | BTN_TRI)

u_char sequence[MAX_SEQUENCE_LENGTH];  // stores correct sequence
u_char userInput[MAX_SEQUENCE_LENGTH]; // stores user input
u_char sequenceLength = 1;
u_char currentIndex = 0;
u_char inInputPhase = 0;
u_char gameActive = 0;
volatile u_char paletteIndex = 0; 
extern u_char inputChecker(u_char shapeId); 

int redrawScreen = 1;

// Game Menu States 
enum GameState {
  WAITING_TO_START,
  SHOWING_SEQUENCE,
  INPUT_PHASE,
  LEVEL_COMPLETE_MENU,
  SETTINGS_MENU
};
// -------- Color Pallette Functions -------
#define NUM_PALETTES 2

typedef struct {
  u_int small;
  u_int big;
  u_int tri;
} 
ColorPalette;

ColorPalette palettes[NUM_PALETTES] = {
  {COLOR_GREEN, COLOR_BLUE, COLOR_RED},         
  {COLOR_DARK_VIOLE, COLOR_DARK_ORANGE, COLOR_DARK_GREEN}  
          
};


enum GameState gameState = WAITING_TO_START;

//Level Counter 
u_char level = 1;

// Shape identifiers
enum Shape { SMALL_SQUARE, BIG_SQUARE, TRIANGLE };

// Display positions
short shape_col = screenWidth / 2;
short shape_row = screenHeight / 2;
void drawCenteredString5x7(u_char colCenter, u_char row, char *string, u_int color, u_int bgColor) {
  u_char len = strlen(string);
  u_char colStart = colCenter - (len * 5 / 2); // each char is 5px wide
  drawString5x7(colStart, row, string, color, bgColor);
}

// -------- Shape Drawing Functions --------
#define NUM_POSITIONS 5
typedef struct {
  short col, row;
} Pos;

const Pos shapePositions[NUM_POSITIONS] = {
  {30, 30},
  {30, screenHeight - 40},
  {screenWidth - 40, 30},
  {screenWidth - 40, screenHeight - 40},
  {screenWidth / 2, screenHeight / 2}
};

void drawSmallSquare() {
  fillRectangle(shape_col - 15, shape_row - 15, 20, 20, palettes[paletteIndex].small);
}

void drawBigSquare() {
  fillRectangle(shape_col - 20, shape_row - 20, 30, 30, palettes[paletteIndex].big);
}

void drawTriangle() {
  for (int i = 0; i < 20; i++) {
    int startCol = shape_col - i;
    int endCol = shape_col + i;
    for (int col = startCol; col <= endCol; col++) {
      drawPixel(col, shape_row + i, palettes[paletteIndex].tri);
    }
  }
}


void clearShape() {
  clearScreen(COLOR_BLACK);
}


// Draw shape based on ID
void drawShape(u_char shapeId) {
  clearShape();

  Pos pos = shapePositions[rand() % NUM_POSITIONS];
  shape_col = pos.col;
  shape_row = pos.row;

  switch (shapeId) {
    case SMALL_SQUARE: drawSmallSquare(); break;
    case BIG_SQUARE:   drawBigSquare();   break;
    case TRIANGLE:     drawTriangle();    break;
  }
}

void generatesequence(){
  for (int i = 0; i < sequenceLength; i++) {
    sequence[i] = rand() % NUM_SHAPES;
  }
}

void showSequence() {
  for (int i = 0; i < sequenceLength; i++) {
    drawShape(sequence[i]);
    __delay_cycles(5000000); // show shape for ~1.5 seconds
    clearShape();
    __delay_cycles(5000000); // pause for ~1 second between shapes

  }
  drawString5x7(10, 10, "Your Turn!", COLOR_WHITE, COLOR_BLACK);
}

static char switch_update_interrupt_sense() {
  char p2val = P2IN;
  P2IES |= (p2val & SWITCHES);
  P2IES &= (p2val | ~SWITCHES);
  return p2val;
}

void switch_init() {
  P2REN |= SWITCHES;
  P2IE |= SWITCHES;
  P2OUT |= SWITCHES;
  P2DIR &= ~SWITCHES;
  switch_update_interrupt_sense();
}
// GAME LOGIC FUNCTIONS 
void switch_interrupt_handler() {
  char p2val = switch_update_interrupt_sense();
  char switches = ~p2val & SWITCHES;

  if (gameState == WAITING_TO_START && (switches & BTN_START)) {
    level = 1;
    sequenceLength = level;
    clearScreen(COLOR_BLACK);
    srand(TAR);
    generatesequence();
    showSequence();
    __delay_cycles(8000000);
    currentIndex = 0;
    gameState = INPUT_PHASE;
    inInputPhase = 1;
    gameActive = 1;
    return;
  }

  if (gameState == LEVEL_COMPLETE_MENU) {
    if (switches & BTN_START) {
      level = (level < MAX_SEQUENCE_LENGTH) ? level + 1 : level;
      sequenceLength = level;
      clearScreen(COLOR_BLACK);
      srand(TAR); 
      generatesequence();
      showSequence();
      __delay_cycles(8000000); 
      currentIndex = 0;
      gameState = INPUT_PHASE;
      inInputPhase = 1;
      gameActive = 1;
    } else if (switches & BTN_SMALL) {
      clearScreen(COLOR_BLACK);
      drawString5x7(10, 10, "Settings Menu", COLOR_WHITE, COLOR_BLACK);
      drawString5x7(10, 30, "Btn1: Continue Game", COLOR_GREEN, COLOR_BLACK);
      drawString5x7(10, 40, "Btn2: Change Color", COLOR_YELLOW, COLOR_BLACK);
      drawString5x7(10, 60, "Btn4: Restart", COLOR_RED, COLOR_BLACK);
      gameState = SETTINGS_MENU;
    } else if (switches & BTN_TRI) {
      level = 1;
      sequenceLength = level;
      currentIndex = 0;
      inInputPhase = 0;
      gameActive = 0;
      gameState = WAITING_TO_START;

      clearScreen(COLOR_BLACK);
      drawMainMenu();

    }
    return;
  }

  if (gameState == INPUT_PHASE && gameActive) {
    u_char shapePressed = 255;
    if (switches & BTN_SMALL) {
      shapePressed = SMALL_SQUARE;
    } else if (switches & BTN_BIG) {
      shapePressed = BIG_SQUARE;
    } else if (switches & BTN_TRI) {
      shapePressed = TRIANGLE;
    }

    if (shapePressed != 255) {
      if (inputChecker(shapePressed)) {
        // ❌ Wrong input
        clearScreen(COLOR_BLACK);
        drawString5x7(10, 10, "Try Again! :(", COLOR_RED, COLOR_BLACK);
        __delay_cycles(4000000);

        level = 1;
        sequenceLength = level;
        currentIndex = 0;
        inInputPhase = 0;
        gameActive = 0;
        gameState = WAITING_TO_START;

        clearScreen(COLOR_BLACK);
        
        drawMainMenu();

      } else {
        // ✅ Correct input
        drawShape(shapePressed);
        __delay_cycles(2500000);
        clearShape();
        currentIndex++;

        if (currentIndex == sequenceLength) {
          clearScreen(COLOR_BLACK);
          char msg[20];
          sprintf(msg, "Level %d Complete!", level);
          drawString5x7(10, 10, msg, COLOR_GREEN, COLOR_BLACK);
          drawString5x7(10, 30, "Btn1: Next", COLOR_WHITE, COLOR_BLACK);
          drawString5x7(10, 40, "Btn2: Change Palette", COLOR_WHITE, COLOR_BLACK);
          drawString5x7(10, 60, "Btn4: Restart", COLOR_WHITE, COLOR_BLACK);
          gameState = LEVEL_COMPLETE_MENU;
          inInputPhase = 0;
          gameActive = 0;

          if (sequenceLength == MAX_SEQUENCE_LENGTH) {
            drawString5x7(10, 20, "Max Level Reached!", COLOR_YELLOW, COLOR_BLACK);
          }
        }
      }
    }
  }

  if (gameState == SETTINGS_MENU) {
    if (switches & BTN_START) {
      level = (level < MAX_SEQUENCE_LENGTH) ? level + 1 : level;
      sequenceLength = level;
      clearScreen(COLOR_BLACK);
      srand(TAR); 
      generatesequence();
      showSequence();
      currentIndex = 0;
      gameState = INPUT_PHASE;
      inInputPhase = 1;
      gameActive = 1;
    } else if (switches & BTN_SMALL) {
      paletteIndex = ((paletteIndex + 1) % NUM_PALETTES);
      clearScreen(COLOR_BLACK);
      drawString5x7(10, 10, "Palette Changed!", COLOR_YELLOW, COLOR_BLACK);

      shape_row = 110;
      shape_col = 24;
      drawSmallSquare();

      shape_col = 61;
      drawBigSquare();

      shape_row = 100;
      shape_col = 100;
      drawTriangle();

      drawString5x7(10, 25, "Btn1: Continue", COLOR_WHITE, COLOR_BLACK);
      drawString5x7(10, 40, "Btn2: Next Palette", COLOR_WHITE, COLOR_BLACK);
    } else if (switches & BTN_TRI) {
      level = 1;
      sequenceLength = level;
      srand(TAR);               
      generatesequence();
      gameState = WAITING_TO_START;
      clearScreen(COLOR_BLACK);
      drawString5x7(10, 10, "Game Restarted!", COLOR_WHITE, COLOR_BLACK);
      drawMainMenu();

    }
    return;
  }
}

void wdt_c_handler() {
  // You can add blinking, countdowns, or animations here if needed
}
  void drawMainMenu() {
    clearScreen(COLOR_BLACK);
    drawCenteredString5x7(screenWidth / 2, 20, "---- Memory Game ------", COLOR_WHITE, COLOR_BLACK);

    shape_row = 80;
    shape_col = 24;
    drawSmallSquare();

    shape_col = 61;
    drawBigSquare();

    shape_row = 70;
    shape_col = 100;
    drawTriangle();

    drawCenteredString5x7(screenWidth / 2 - 6, screenHeight - 30, "Press Btn1 to Start", COLOR_WHITE, COLOR_BLACK);
  }

void main() {

  WDTCTL = WDTPW | WDTHOLD; // Stop watchdog for manual config
  P1DIR |= BIT6;
  P1OUT |= BIT6;

  configureClocks();
  TACTL = TASSEL_2 + MC_2; // Use SMCLK, continuous mode

  lcd_init();
  switch_init();

  paletteIndex = 1;

  enableWDTInterrupts();
  or_sr(0x8); // GIE - enable interrupts

  clearScreen(COLOR_BLACK);
  srand(TAR);
  generatesequence();
  drawMainMenu();

  while (1) {
    P1OUT &= ~BIT6;
    or_sr(0x10); // CPU off
    P1OUT |= BIT6;
  }
}
__attribute__((interrupt(PORT2_VECTOR)))
void Port_2(void) {
  if (P2IFG & SWITCHES) {
    P2IFG &= ~SWITCHES;
    switch_interrupt_handler();
  }
}

