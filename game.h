#ifndef _GAMEH
#define _GAMEH

#include "gameStructs.h"
#include "keyboard.h"
#include "lcom/lcf.h"
#include "player.h"
#include "rtc.h"
#include "serial.h"
#include "stdbool.h"
#include "stdint.h"
#include "stdlib.h"

/** @defgroup game game
 * @{
 *
 */

#define TIME_OUT_TIME                                                       \
  20 /**< Time in seconds of each turn in case the player takes too long to \
play*/

typedef enum {
  PVP,
  PVE
} GameType; /**< Game type, PVP - Player vs Player, PVE - Player vs Bot */

/**
 * @brief Coordinates of the chosen cell by the bot
 *
 */
typedef struct {
  int x, y;
} shotCoords;

/**
 * @brief State of the game
 *
 */
typedef enum {
  mainMenu,
  botMenu,
  building,
  playerChoosingTransition,
  playerChoosing,
  afterShotWaiting,
  oponentChoosingTransition,
  oponentChoosing,
  waiting,
  victory,
  loss
} gameState;

/**
 * @brief Type of bot, the absolute bot is the hardest, the dum is the easiest
 *
 */
typedef enum {
  dumb,
  // slight,
  absolute
} botop;

typedef enum { vs_bot, vs_player } mainoption;

// Utilities

/**
 * @brief Returns how many cells the ship occupies
 *
 * @param st Ship type
 * @return uint Number of cells
 */
uint ship_size(shiptype st);

/**
 * @brief Generates a random board for the specified player.
 *
 * @param p Player
 */
void game_generate_board(Player* p);

/**
 * @brief Returns booelan that tells if the game is still running.
 *
 * Returns true if the game still supposed to be running and false otherwise.
 * Used by the main game loop in proj.c as a run condition.
 *
 */
bool isGameRunning();
bool game_button_check(Button* b, uint32_t mouseX, uint32_t mouseY);

// Game State Handlers

/**
 * @brief Changes the state of the game
 *
 * @param state New state of the game
 */
void game_change_state(gameState state);

/**
 * @brief Initializes all the variables, loads all the sprites so the game can
 * begin
 *
 */
void game_init();

/**
 * @brief Advances a tick in all anisprites
 *
 */
void game_advance_tick();

/**
 * @brief Sets the handler for each peripheral given the state of the game
 *
 */
void game_set_handlers();

/**
 * @brief Toggles a button
 *
 * @param btn Button to be activated
 */
void game_activate_building_button(int btn);

/**
 * @brief Checks if the player has placed all the 5 battleships and changes the
 * state to playing mode
 *
 */
void game_finished_building();

/**
 * @brief Initializes all the builing buttons
 *
 */
void game_init_build_buttons();

// Bots
/**
 * @brief Simple bot that randomly chooses a spot to shoot
 *
 * @param lastX Last shot x coordinate
 * @param lastY Last shot y coordinate
 * @return shotCoords Next shot coordinates
 */
shotCoords default_bot_get_coords(int lastX, int lastY);

/**
 * @brief An immproved version of the default bot that takes into account the
 * position of the last shot and if it hit or not
 *
 * @param lastX Last shot x coordinate
 * @param lastY Last shot y coordinate
 * @return shotCoords Next shot coordinates
 */
shotCoords improved_bot_get_coords(int lastX, int lastY);

// Bot handler

/**
 * @brief Bot handler, generates the seed and uses one of the bots to shoot a
 * cell
 *
 */
void game_bot_shot();

// RTC Handler

void playerChoosing_rtc_handler(struct rtc_event event);
void playerChoosingTransition_rtc_handler(struct rtc_event event);
void oponentChoosingTransition_rtc_handler(struct rtc_event event);

// Serial Handler

void building_serial_handler(struct serial_event event);
void afterShot_serial_handler(struct serial_event event);
void waiting_serial_handler(struct serial_event event);

void oponentChoosing_serial_handler(struct serial_event event);
void playerChoosing_serial_handler(struct serial_event event);

// Mouse Handlers
/**
 * @brief Mouse handler for the Building phase
 *
 * @param event Mouse event
 */
void building_mouse_handler(struct mouse_ev event);

/**
 * @brief Mouse handler for the Playing phase
 *
 * @param event Mouse event
 */
void playerChoosing_mouse_handler(struct mouse_ev event);

// Keyboard handlers

/**
 * @brief Keyboard handler for the bot menu
 *
 * @param event Keyboard event
 */
void botmenu_kbd_handler(struct kbd_event event);

/**
 * @brief Keyboard handler for the waiting screen between plays
 *
 * @param event Keyboard event
 */
void waiting_kbd_handler(struct kbd_event event);

/**
 * @brief Keyboard handler for the victory screen
 *
 * @param event Keyboard event
 */
void victory_kbd_handler(struct kbd_event event);

/**
 * @brief Keyboard handler for the lose screen
 *
 * @param event Keyboard event
 */
void loss_kbd_handler(struct kbd_event event);

/**
 * @brief Keyboard handler for the building phase
 *
 * @param event Keyboard event
 */
void building_kbd_handler(struct kbd_event event);

/**
 * @brief Keyboard handler for the opponent turn
 *
 * @param event Keyboard event
 */
void oponentChoosing_kbd_handler(struct kbd_event event);

/**
 * @brief Keyboard handler for the main menu
 *
 * @param event Keyboard event
 */
void mainmenu_kbd_handler(struct kbd_event event);

/**
 * @brief Keyboard handler for the player turn
 *
 * @param event Keyboard event
 */
void playerChoosing_kbd_handler(struct kbd_event event);

// XPM DRAWERS

/**
 * @brief Draws the help in the build phase
 *
 */
void game_draw_buildhelp();

/**
 * @brief Draws the help when displaying opponent's board
 *
 */
void game_draw_shootophelp();

/**
 * @brief Draws the help when displaying player's board
 *
 */
void game_draw_shootplayer();

/**
 * @brief Draws the losing screen
 *
 */
void game_draw_losescreen();

/**
 * @brief Draws the victory screen
 *
 */
void game_draw_winscreen();

/**
 * @brief Draws the banner on top to display whom's turn is it
 *
 * @param yturn True if player's turn
 */
void game_draw_turn(bool yturn);

/**
 * @brief Draws all ships on the screen
 *
 */
void game_ship_draw();

/**
 * @brief Advances animation frame
 *
 */
void game_advance_animations();

/**
 * @brief Draws the things necessary on the screen given the stage of the game
 *
 */
void game_render_current_state();

/**
 * @brief Draws all animations in the animation list
 *
 */
void game_render_animations();

// XPM Loaders

/**
 * @brief Loads all xpm
 *
 */
void game_load_all_xpm();

/**
 * @brief Loads the xpm fo the main menu
 *
 */
void game_mainmenu_load();

/**
 * @brief Loads all the ships sprites
 *
 */
void game_ship_load();

/**
 * @brief Loads all the frames of the explosion
 *
 */
void game_explosions_load();

/**
 * @brief Loads all the buttons
 *
 */
void game_buttons_load();

/**
 * @brief Loads all the frames of the flame
 *
 */
void game_load_flame();

/**@}*/

#endif
