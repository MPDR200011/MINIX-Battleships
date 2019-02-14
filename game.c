#include "game.h"
#include "List.h"
#include "event.h"
#include "graphics.h"
#include "mouse.h"
#include "player.h"
#include "sprites.h"
#include "stdlib.h"
#include "unistd.h"

// Settings

bool gameRunning = true;  /**<This bools keeps the program running */
uint32_t squareSide = 35; /**<Size of each cell on the screen in pixels */
uint32_t tableGap = 1; /**<Size of the gap between each board cell in pixels */

uint32_t tableXStart; /**<X coordinate of the upper left corner of the table.*/
uint32_t tableYStart; /**<Y coordinate of the upper left corner of the table.*/

// Entities
Player oponent; /**< Holds the info of the opponent */
Player player;  /**< Holds the info of the player */

// Build
List* build_buttons;    /**< List with the buttons that choose which boat the
                           player wants to place in the building phase*/
Sprite* buttons_sprite; /**< Sprite of the buttons in the build phase, 0,1
                           square with circle corners, 2,3 circle button */
static int activeButton = -1;      /**< Index of the pressed button */
static Button* button_pressed = 0; /**< Pointer to the pressed button */
bool orientationVertical =
    false; /**< Orientation of the to-be-placed ship in the building phase */

// Current State control
static GameType gameType =
    PVP; /**< Game type, PVP - Player vs Player, PVE - Player vs Bot */
static gameState currentState = mainMenu; /**< Current State of the game*/
static bool watchingPlayerBoard =
    true; /**< Displayed board during the player's turn */
static bool oponentReady =
    false; /**< Checks if the opponent has finished building */
shotCoords (*bot_coords)(int, int) =
    &improved_bot_get_coords; /**< Pointer to the bot the player is going to
                                 play against */

// Sprites
mainoption mainmenu_selec = vs_bot; /**< Option selected in the main menu */
botop botop_selec = dumb;           /**< Option selected in the bot menu*/
Sprite
    test_ships[5]; /**< Array containing the ship sprites, ordered bby size */
Sprite background_mainmenu; /**< Sprite if the main menu background */
Sprite deletemode;          /**<Delete mode font*/

Sprite mainmenu_vsbot;    /**< Sprite of the main menu Vs Bot Button. */
Sprite mainmenu_vsplayer; /**< Sprite of the main menu Vs Player Button. */
Sprite mainmenu_cursor;   /**< Sprite of menu cursor that indicates current
                             selection. */

Sprite buildhelp;       /**< Sprite of the building phase instructions. */
Sprite shootophelp;     /**< Sprite of the oponent vision instructions. */
Sprite shootplayerhelp; /**< Sprite of the player board vision instructions. */

Sprite losescreen; /**< Background of defeat screen. */
Sprite winscreen;  /**< Background of the victory screen. */

Sprite yourturn; /**< "Your turn:" text above the board. */
Sprite opturn;   /**< "Oponent's turn:" text above the board. */

Sprite bdumb;     /**< Sprite for dumb bot button. */
Sprite babsolute; /**< Sprite for absolute bot button. */
Sprite botback;   /**< Bot menu background*/

Sprite waitscreen; /**< Wait screen that appears when one player is waiting for
                      the other to build the board*/

AniSprite explosion; /**< Explosion animated sprite */
AniSprite flame;     /**< Flame animated sprite */

List currentAnimations; /**< Queue with the animations that are ready to be
                           renderes*/

int lastShotX = 0; /**< Variable to store last shot X the player did. */
int lastShotY = 0; /**< Variable to store last shot Y the player did. */

static gameState nextState = playerChoosing; /**< State of the game*/

////////////////////////////////////////////////////////////////
// Utilities
////////////////////////////////////////////////////////////////

uint ship_size(shiptype st) {
  switch (st) {
    case destroyer:
      return 2;
    case submarine:
    case cruiser:
      return 3;
    case battleship:
      return 4;
    case carrier:
      return 5;
  }
}

void game_generate_board(Player* p) {
  for (int i = 0; i < p->battleships_size; i++) {
    ship* s = &p->battleships[i];
    if (!s->exists) {
      continue;
    }
    if (s->vertical) {
      for (uint i = 0; i < s->size; i++) {
        p->player_board[s->y + i][s->x] = OCEAN;
      }
    } else {
      for (uint i = 0; i < s->size; i++) {
        p->player_board[s->y][s->x + i] = OCEAN;
      }
    }

    s->exists = false;
    s->x = -1;
    s->y = -1;
    s->vertical = false;
  }

  static uint seedAdd = 0;
  srand(time(NULL) + seedAdd++);
  for (int i = 0; i < p->battleships_size; i++) {
    ship s;
    do {
      bool vertical = rand() % 2 - 1;
      int x = rand() % 10;
      int y = rand() % 10;
      s.exists = true;
      s.st = i % 5;
      s.vertical = vertical;
      s.x = x;
      s.y = y;
      s.hits = malloc(sizeof(bool) * ship_size(i % 5));
      s.size = ship_size(i % 5);
    } while (player_setPlayerShip(p, s) != OK);
  }
}

bool isGameRunning() {
  return gameRunning;
}

bool game_button_check(Button* b, uint32_t mouseX, uint32_t mouseY) {
  return (mouseY < b->y + b->height && mouseX < b->x + b->width &&
          mouseY > b->y && mouseX > b->x);
}

////////////////////////////////////////////////////////////////
// Game State Handlers
////////////////////////////////////////////////////////////////

void game_change_state(gameState state) {
  currentState = state;
  if (state == building) {
    watchingPlayerBoard = true;
    oponentReady = false;
    for (int i = 0; i < player.battleships_size; i++) {
      for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
          player.player_board[i][j] = OCEAN;
          player.oponent_board[i][j] = OPONENT_UNKNOWN;
          if (gameType == PVE) {
            oponent.player_board[i][j] = OCEAN;
            oponent.oponent_board[i][j] = OPONENT_UNKNOWN;
          }
        }
      }
      player.battleships[i].exists = false;
    }
  }
  if (state == oponentChoosing) {
    watchingPlayerBoard = true;
  }
  if (state == playerChoosing) {
    rtc_set_timeout(TIME_OUT_TIME);
    watchingPlayerBoard = false;
  }
  if (state == mainMenu) {
    for (size_t i = 0; i < currentAnimations.size; i++) {
      free(((AniSprite*)list_get(&currentAnimations, i))->frames);
    }
    list_clear(&currentAnimations);
  }
  game_set_handlers();
}

void game_init() {
  game_load_all_xpm();
  game_set_handlers();
  player = player_init();
  oponent = player_init();
  mouse_reset();

  tableXStart = (getHres() / 2) - ((10 * squareSide + 9 * tableGap) / 2);
  tableYStart = (getVres() / 2) - ((10 * squareSide + 9 * tableGap) / 2);
  game_init_build_buttons();

  currentAnimations = list_create();
}

void game_advance_tick() {
  game_advance_animations();
  switch (currentState) {
    case oponentChoosing:
      if (gameType == PVE) {
        static uint botWait = 60;
        botWait--;
        if (botWait == 0) {
          game_bot_shot();
          botWait = 60;
        }
      }
      break;
    default:
      break;
  }
}

void game_set_handlers() {
  reset_handlers();
  switch (currentState) {
    case mainMenu:
      set_keyboard_handler(&mainmenu_kbd_handler);
      break;
    case botMenu:
      set_keyboard_handler(&botmenu_kbd_handler);
      break;
    case building:
      set_mouse_btn_handler(&building_mouse_handler);
      set_keyboard_handler(&building_kbd_handler);
      set_serial_handler(&building_serial_handler);
      break;
    case playerChoosingTransition:
      set_rtc_handler(&playerChoosingTransition_rtc_handler);
      break;
    case playerChoosing:
      set_mouse_btn_handler(&playerChoosing_mouse_handler);
      set_keyboard_handler(&playerChoosing_kbd_handler);
      set_serial_handler(&playerChoosing_serial_handler);
      set_rtc_handler(&playerChoosing_rtc_handler);
      break;
    case afterShotWaiting:
      set_serial_handler(&afterShot_serial_handler);
      break;
    case oponentChoosingTransition:
      set_rtc_handler(&oponentChoosingTransition_rtc_handler);
      break;
    case oponentChoosing:
      set_serial_handler(&oponentChoosing_serial_handler);
      set_keyboard_handler(&oponentChoosing_kbd_handler);
      break;
    case waiting:
      set_serial_handler(&waiting_serial_handler);
      set_keyboard_handler(&waiting_kbd_handler);
      break;
    case victory:
      set_keyboard_handler(&victory_kbd_handler);
      break;
    case loss:
      set_keyboard_handler(&loss_kbd_handler);
      break;
    default:
      break;
  }
}

void game_activate_building_button(int btnIndex) {
  if (btnIndex < 0 || btnIndex > 5) {
    return;
  }

  if (activeButton == 5) {
    if (btnIndex == 5) {
      button_pressed->pressed = false;
      activeButton = -1;
      button_pressed = 0;
    } else {
      player_removeShip(&player, btnIndex);
    }
    return;
  }
  activeButton = btnIndex;

  for (uint i = 0; i < build_buttons->size; i++) {
    Button* btn = list_get(build_buttons, i);
    if ((int)i == btnIndex) {
      btn->pressed = true;
      button_pressed = btn;
    } else {
      btn->pressed = false;
    }
  }
}

void game_finished_building() {
  for (int i = 0; i < player.battleships_size; i++) {
    if (!player.battleships[i].exists) {
      return;
    }
  }

  if (gameType == PVE) {
    game_generate_board(&oponent);
    game_change_state(playerChoosing);
  } else {
    serial_addByteToSend(READY_MSG);
    if (oponentReady) {
      game_change_state(oponentChoosing);
    } else {
      game_change_state(waiting);
    }
  }
}

void game_init_build_buttons() {
  uint32_t TRCx = tableXStart + (10 * squareSide + 9 * tableGap);
  uint32_t TRCy = tableYStart;
  Button tmp = {.x = TRCx + 20,
                .y = TRCy - (2 * tableGap + 2 * squareSide),
                .width = squareSide,
                .height = squareSide,
                .pressed = false};
  Button* tmp_ptr;

  for (size_t i = 0; i < 5; i++) {
    tmp_ptr = malloc(sizeof(Button));
    tmp.y += (2 * tableGap + 2 * squareSide);
    *tmp_ptr = tmp;
    list_add(build_buttons, tmp_ptr);
  }

  // delete button
  tmp_ptr = malloc(sizeof(Button));
  tmp.x = tableXStart + (2 * squareSide + 2 * tableGap);
  tmp.y = tableYStart + (11 * squareSide + 11 * tableGap);
  tmp.pressed = false;
  *tmp_ptr = tmp;

  list_add(build_buttons, tmp_ptr);
}

////////////////////////////////////////////////////////////////
// Bot Handlers
////////////////////////////////////////////////////////////////

void game_bot_shot() {
  static uint32_t seedAdd = 0;
  srand(time(NULL) + seedAdd++);

  static int lastHitX = -1;
  static int lastHitY = -1;

  shotCoords shot = (*bot_coords)(lastHitX, lastHitY);
  uint32_t x = shot.x;
  uint32_t y = shot.y;

  if (!player_shootPlayer(&player, x, y)) {
    oponent.oponent_board[y][x] = OCEAN;
    game_change_state(oponentChoosingTransition);
    nextState = playerChoosing;
    rtc_set_timeout(2);
    lastHitX = -1;
    lastHitY = -1;
  } else {
    oponent.oponent_board[y][x] = OPONENT_SHIP_HIT;
    // Flame
    AniSprite* asp_fl = malloc(sizeof(AniSprite));
    *asp_fl = flame;
    asp_fl->x = tableXStart + x * (squareSide + tableGap);
    asp_fl->y = tableYStart + y * (squareSide + tableGap);
    list_add(&currentAnimations, asp_fl);

    // Explosion
    AniSprite* asp = malloc(sizeof(AniSprite));
    *asp = explosion;
    asp->x = tableXStart + x * (squareSide + tableGap);
    asp->y = tableYStart + y * (squareSide + tableGap);
    list_add(&currentAnimations, asp);
    lastHitX = x;
    lastHitY = y;
    if (player_checkIfFleetDestroyed(&player)) {
      game_change_state(oponentChoosingTransition);
      lastHitX = -1;
      lastHitY = -1;
      nextState = loss;
      rtc_set_timeout(2);
    }
  }
}

bool check_valid_choice(Player* p, int x, int y) {
  return p->oponent_board[y][x] == OPONENT_UNKNOWN;
}

bool checkMiss(int x, int y) {
  return ((x == -1) && (y == -1));
}

////////////////////////////////////////////////////////////////
// Bots
////////////////////////////////////////////////////////////////

shotCoords default_bot_get_coords(int lastX, int lastY) {
  shotCoords shot = {.x = lastX, .y = lastY};

  do {
    shot.x = rand() % 10;
    shot.y = rand() % 10;
  } while (!check_valid_choice(&oponent, shot.x, shot.y));

  return shot;
}

shotCoords improved_bot_get_coords(int lastX, int lastY) {
  enum botPlayState {
    searchingShip,
    searchingDirection,
    foundDirection,
    reversedDirection
  };
  enum directions { triedRight, triedUp, triedLeft, triedDown };
  static enum botPlayState currShotState = searchingShip;
  static enum directions currDirection = triedRight;
  bool validChoice = false;
  static shotCoords initialHit = {.x = 0, .y = 0};
  static shotCoords lastHit = {.x = 0, .y = 0};
  shotCoords shot = {.x = 0, .y = 0};
  printf("in state %d, %d with shot : %d - %d\n", currShotState, currDirection,
         lastX, lastY);
  do {
    validChoice = false;
    switch (currShotState) {
      case searchingShip:
        if (checkMiss(lastX, lastY)) {
          shot.x = rand() % 10;
          shot.y = rand() % 10;
          validChoice = check_valid_choice(&oponent, shot.x, shot.y);
        } else {
          initialHit.x = lastX;
          initialHit.y = lastY;
          lastHit.x = lastX;
          lastHit.y = lastY;
          currShotState = searchingDirection;
          shot.x = lastX + 1;
          shot.y = lastY;
          if (shot.x < 10) {
            if (check_valid_choice(&oponent, shot.x, shot.y)) {
              validChoice = true;
            } else {
              validChoice = false;
              lastX = -1;
              lastY = -1;
            }
          } else {
            lastX = -1;
            lastY = -1;
          }
          currDirection = triedRight;
        }
        break;
      case searchingDirection:
        switch (currDirection) {
          case triedRight:
            if (checkMiss(lastX, lastY)) {
              shot.x = lastHit.x;
              shot.y = lastHit.y - 1;
              if (shot.y >= 0) {
                if (check_valid_choice(&oponent, shot.x, shot.y)) {
                  validChoice = true;
                } else {
                  validChoice = false;
                  lastX = -1;
                  lastY = -1;
                }
              } else {
                lastX = -1;
                lastY = -1;
              }
              currDirection = triedUp;
            } else {
              lastHit.x = lastX;
              lastHit.y = lastY;
              currShotState = foundDirection;
            }
            break;
          case triedUp:
            if (checkMiss(lastX, lastY)) {
              shot.x = lastHit.x - 1;
              shot.y = lastHit.y;
              if (shot.x >= 0) {
                if (check_valid_choice(&oponent, shot.x, shot.y)) {
                  validChoice = true;
                } else {
                  validChoice = false;
                  lastX = -1;
                  lastY = -1;
                }
              } else {
                lastX = -1;
                lastY = -1;
              }
              currDirection = triedLeft;
            } else {
              lastHit.x = lastX;
              lastHit.y = lastY;
              currShotState = foundDirection;
            }
            break;
          case triedLeft:
            if (checkMiss(lastX, lastY)) {
              shot.x = lastHit.x;
              shot.y = lastHit.y + 1;
              if (shot.y < 10) {
                if (check_valid_choice(&oponent, shot.x, shot.y)) {
                  validChoice = true;
                } else {
                  validChoice = false;
                  lastX = -1;
                  lastY = -1;
                }
              } else {
                lastX = -1;
                lastY = -1;
              }
              currDirection = triedDown;
            } else {
              lastHit.x = lastX;
              lastHit.y = lastY;
              currShotState = foundDirection;
            }
            break;
          case triedDown:
            if (checkMiss(lastX, lastY)) {
              currShotState = searchingShip;
            } else {
              lastHit.x = lastX;
              lastHit.y = lastY;
              currShotState = foundDirection;
            }
            break;
        }
        break;
      case foundDirection:
        switch (currDirection) {
          case triedRight:
            if (checkMiss(lastX, lastY)) {
              validChoice = false;
              currShotState = reversedDirection;
              lastX = initialHit.x;
              lastY = initialHit.y;
            } else {
              shot.x = lastX + 1;
              shot.y = lastY;
              if (shot.x < 10) {
                if (check_valid_choice(&oponent, shot.x, shot.y)) {
                  validChoice = true;
                } else {
                  validChoice = false;
                  lastX = initialHit.x;
                  lastY = initialHit.y;
                  currShotState = reversedDirection;
                }
              } else {
                validChoice = false;
                lastX = initialHit.x;
                lastY = initialHit.y;
                currShotState = reversedDirection;
              }
            }
            break;
          case triedUp:
            if (checkMiss(lastX, lastY)) {
              validChoice = false;
              currShotState = reversedDirection;
              lastX = initialHit.x;
              lastY = initialHit.y;
            } else {
              shot.x = lastX;
              shot.y = lastY - 1;
              if (shot.y >= 0) {
                if (check_valid_choice(&oponent, shot.x, shot.y)) {
                  validChoice = true;
                } else {
                  validChoice = false;
                  lastX = initialHit.x;
                  lastY = initialHit.y;
                  currShotState = reversedDirection;
                }
              } else {
                validChoice = false;
                lastX = initialHit.x;
                lastY = initialHit.y;
                currShotState = reversedDirection;
              }
            }
            break;
          case triedLeft:
            if (checkMiss(lastX, lastY)) {
              validChoice = false;
              currShotState = reversedDirection;
              lastX = initialHit.x;
              lastY = initialHit.y;
            } else {
              shot.x = lastX - 1;
              shot.y = lastY;
              if (shot.x >= 0) {
                if (check_valid_choice(&oponent, shot.x, shot.y)) {
                  validChoice = true;
                } else {
                  validChoice = false;
                  lastX = initialHit.x;
                  lastY = initialHit.y;
                  currShotState = reversedDirection;
                }
              } else {
                validChoice = false;
                lastX = initialHit.x;
                lastY = initialHit.y;
                currShotState = reversedDirection;
              }
            }
            break;
          case triedDown:
            if (checkMiss(lastX, lastY)) {
              validChoice = false;
              currShotState = reversedDirection;
              lastX = initialHit.x;
              lastY = initialHit.y;
            } else {
              shot.x = lastX;
              shot.y = lastY + 1;
              if (shot.y < 10) {
                if (check_valid_choice(&oponent, shot.x, shot.y)) {
                  validChoice = true;
                } else {
                  validChoice = false;
                  lastX = initialHit.x;
                  lastY = initialHit.y;
                  currShotState = reversedDirection;
                }
              } else {
                validChoice = false;
                lastX = initialHit.x;
                lastY = initialHit.y;
                currShotState = reversedDirection;
              }
            }
            break;
        }
        break;
      case reversedDirection:
        switch (currDirection) {
          case triedRight:
            if (checkMiss(lastX, lastY)) {
              currShotState = searchingShip;
              lastX = -1;
              lastY = -1;
              currDirection = triedRight;
            } else {
              shot.x = lastX - 1;
              shot.y = lastY;
              if (shot.x >= 0) {
                if (check_valid_choice(&oponent, shot.x, shot.y)) {
                  validChoice = true;
                } else {
                  validChoice = false;
                  lastX = -1;
                  lastY = -1;
                  currShotState = searchingShip;
                  currDirection = triedRight;
                }
              } else {
                validChoice = false;
                lastX = -1;
                lastY = -1;
                currShotState = searchingShip;
                currDirection = triedRight;
              }
            }
            break;
          case triedUp:
            if (checkMiss(lastX, lastY)) {
              currShotState = searchingShip;
              lastX = -1;
              lastY = -1;
              currDirection = triedRight;
            } else {
              shot.x = lastX;
              shot.y = lastY + 1;
              if (shot.y < 10) {
                if (check_valid_choice(&oponent, shot.x, shot.y)) {
                  validChoice = true;
                } else {
                  validChoice = false;
                  lastX = -1;
                  lastY = -1;
                  currShotState = searchingShip;
                  currDirection = triedRight;
                }
              } else {
                validChoice = false;
                lastX = -1;
                lastY = -1;
                currShotState = searchingShip;
                currDirection = triedRight;
              }
            }
            break;
          case triedLeft:
            if (checkMiss(lastX, lastY)) {
              currShotState = searchingShip;
              lastX = -1;
              lastY = -1;
              currDirection = triedRight;
            } else {
              shot.x = lastX + 1;
              shot.y = lastY;
              if (shot.x < 10) {
                if (check_valid_choice(&oponent, shot.x, shot.y)) {
                  validChoice = true;
                } else {
                  validChoice = false;
                  lastX = -1;
                  lastY = -1;
                  currShotState = searchingShip;
                  currDirection = triedRight;
                }
              } else {
                validChoice = false;
                lastX = -1;
                lastY = -1;
                currShotState = searchingShip;
                currDirection = triedRight;
              }
            }
            break;
          case triedDown:
            if (checkMiss(lastX, lastY)) {
              currShotState = searchingShip;
              lastX = -1;
              lastY = -1;
              currDirection = triedRight;
            } else {
              shot.x = lastX;
              shot.y = lastY - 1;
              if (shot.y >= 0) {
                if (check_valid_choice(&oponent, shot.x, shot.y)) {
                  validChoice = true;
                } else {
                  validChoice = false;
                  lastX = -1;
                  lastY = -1;
                  currShotState = searchingShip;
                  currDirection = triedRight;
                }
              } else {
                validChoice = false;
                lastX = -1;
                lastY = -1;
                currShotState = searchingShip;
                currDirection = triedRight;
              }
            }
            break;
        }
        break;
    }
  } while (!validChoice);
  printf("states %d, %d  - trying %d - %d\n", currShotState, currDirection,
         shot.x, shot.y);
  return shot;
}

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
// RTC Handler
////////////////////////////////////////////////////////////////

void playerChoosing_rtc_handler(struct rtc_event event) {
  if (event.type == timeoutAlarm) {
    game_change_state(oponentChoosing);
    serial_addByteToSend(MISS_MSG);
  }
}

void playerChoosingTransition_rtc_handler(struct rtc_event event) {
  if (event.type == timeoutAlarm) {
    game_change_state(nextState);
  }
}

void oponentChoosingTransition_rtc_handler(struct rtc_event event) {
  if (event.type == timeoutAlarm) {
    game_change_state(nextState);
  }
}

////////////////////////////////////////////////////////////////
// Serial Handler
////////////////////////////////////////////////////////////////

void playerChoosing_serial_handler(struct serial_event event) {
  if (event.MSG == VICT_MSG) {
    game_change_state(victory);
  }
}

void oponentChoosing_serial_handler(struct serial_event event) {
  if (event.MSG == SHOT_HEADER) {
    if (player_shootPlayer(&player, event.X, event.Y)) {  // Hit player ship
      AniSprite* asp_fl = malloc(sizeof(AniSprite));
      AniSprite* asp_exp = malloc(sizeof(AniSprite));
      // Flame
      *asp_fl = flame;
      asp_fl->x = tableXStart + event.X * (squareSide + tableGap);
      asp_fl->y = tableYStart + event.Y * (squareSide + tableGap);
      list_add(&currentAnimations, asp_fl);

      // Explosion
      *asp_exp = explosion;
      asp_exp->x = tableXStart + event.X * (squareSide + tableGap);
      asp_exp->y = tableYStart + event.Y * (squareSide + tableGap);
      list_add(&currentAnimations, asp_exp);

      if (player_checkIfFleetDestroyed(&player)) {
        game_change_state(oponentChoosingTransition);
        nextState = loss;
        rtc_set_timeout(2);
        serial_addByteToSend(VICT_MSG);
      } else {
        serial_addByteToSend(HIT_MSG);
      }
    } else {
      serial_addByteToSend(MISS_MSG);
      mouse_reset();
      game_change_state(oponentChoosingTransition);
      nextState = playerChoosing;
      rtc_set_timeout(2);
    }
  }

  if (event.MSG == VICT_MSG) {
    game_change_state(victory);
  }

  if (event.MSG == MISS_MSG) {
    game_change_state(playerChoosing);
  }
}

void building_serial_handler(struct serial_event event) {
  if (event.MSG == READY_MSG) {
    oponentReady = true;
  }
}

void afterShot_serial_handler(struct serial_event event) {
  if (event.MSG == HIT_MSG) {
    player.oponent_board[lastShotY][lastShotX] = OPONENT_SHIP_HIT;
    game_change_state(playerChoosing);
  }
  if (event.MSG == MISS_MSG) {
    player.oponent_board[lastShotY][lastShotX] = OCEAN;
    game_change_state(playerChoosingTransition);
    nextState = oponentChoosing;
    rtc_set_timeout(2);
  }
  if (event.MSG == VICT_MSG) {
    game_change_state(playerChoosingTransition);
    nextState = victory;
    rtc_set_timeout(2);
  }
}

void waiting_serial_handler(struct serial_event event) {
  if (event.MSG == READY_MSG) {
    serial_addByteToSend(LATE_MSG);
    game_change_state(playerChoosing);
  }
  if (event.MSG == LATE_MSG) {
    game_change_state(oponentChoosing);
  }
}

////////////////////////////////////////////////////////////////
// Mouse handlers
////////////////////////////////////////////////////////////////

void playerChoosing_mouse_handler(struct mouse_ev event) {
  if (watchingPlayerBoard) {
    return;
  }
  if (event.type == LB_PRESSED) {
    if (mouse_getX() < 0 || mouse_getY() < 0) {
      return;
    }

    uint32_t mouseX = mouse_getX();
    uint32_t mouseY = mouse_getY();

    if (mouseX < tableXStart || mouseY < tableYStart) {
      return;
    }

    int cellY = -1;
    int cellX = -1;
    for (int y = 1; y <= 10; y++) {  // Checks which row was clicked
      if (mouseY < tableYStart + y * (tableGap + squareSide)) {
        cellY = y - 1;
        break;
      }
    }

    if (cellY == -1) {
      return;
    }

    for (int x = 1; x <= 10; x++) {  // Checks which col was clicked
      if (mouseX < tableXStart + x * (tableGap + squareSide)) {
        cellX = x - 1;
        break;
      }
    }

    if (cellX == -1) {
      return;
    }

    if (player.oponent_board[cellY][cellX] != OPONENT_UNKNOWN) {
      return;
    }

    if (gameType == PVE) {
      if (!player_shootPlayer(&oponent, cellX, cellY)) {
        game_change_state(playerChoosingTransition);
        nextState = oponentChoosing;
        rtc_set_timeout(2);
        player.oponent_board[cellY][cellX] = OCEAN;
        return;
      } else {
        player.oponent_board[cellY][cellX] = OPONENT_SHIP_HIT;
        if (player_checkIfFleetDestroyed(&oponent)) {
          game_change_state(playerChoosingTransition);
          nextState = victory;
          rtc_set_timeout(2);
        }
      }
    } else {
      lastShotX = cellX;
      lastShotY = cellY;
      game_change_state(afterShotWaiting);
      serial_addByteToSend(SHOT_HEADER);
      serial_addByteToSend(cellX);
      serial_addByteToSend(cellY);
    }
  }
  if (event.type == BUTTON_EV) {
    mouse_reset();
  }
}

void building_mouse_handler(struct mouse_ev event) {
  uint32_t mouseX = mouse_getX();
  uint32_t mouseY = mouse_getY();
  // static int cells_pressed = 0;

  switch (event.type) {
    case LB_PRESSED: {
      Button* deleteBtn = list_get(build_buttons, 5);
      if (game_button_check(deleteBtn, mouseX, mouseY)) {
        activeButton = 5;
        deleteBtn->pressed = !deleteBtn->pressed;
        if (button_pressed != deleteBtn) {
          if (button_pressed != 0) {
            button_pressed->pressed = false;
          }
          button_pressed = deleteBtn;
        } else {
          button_pressed->pressed = false;
          button_pressed = 0;
        }
        return;
      }

      if (button_pressed == 0) {
        for (uint i = 0; i < build_buttons->size; i++) {
          Button* tmp = list_get(build_buttons, i);
          if (game_button_check(tmp, mouseX, mouseY)) {
            if (!tmp->pressed) {
              tmp->pressed = true;
              button_pressed = tmp;
            }
            activeButton = i;
            return;
          }
        }
      } else if (button_pressed == deleteBtn) {
        for (uint i = 0; i < build_buttons->size; i++) {
          Button* tmp = list_get(build_buttons, i);
          if (game_button_check(tmp, mouseX, mouseY)) {
            player_removeShip(&player, i);
            return;
          }
        }
      } else {
        if (game_button_check(button_pressed, mouseX, mouseY)) {
          button_pressed->pressed = !button_pressed->pressed;
          button_pressed = 0;
          activeButton = -1;
        }
      }

      if (mouseX < tableXStart) {
        return;
      }

      if (mouseY < tableYStart) {
        return;
      }

      int cellY = -1;
      int cellX = -1;
      for (int y = 1; y <= 10; y++) {  // Checks which row was clicked
        if (mouseY < tableYStart + y * (tableGap + squareSide)) {
          cellY = y - 1;
          break;
        }
      }

      if (cellY == -1) {
        return;
      }

      for (int x = 1; x <= 10; x++) {  // Checks which col was clicked
        if (mouseX < tableXStart + x * (tableGap + squareSide)) {
          cellX = x - 1;
          break;
        }
      }

      if (cellX == -1) {
        return;
      }
      if (activeButton != 5) {
        ship s = {.exists = true,
                  .st = activeButton,
                  .vertical = orientationVertical,
                  .x = cellX,
                  .y = cellY,
                  .hits = malloc(sizeof(bool) * ship_size(activeButton)),
                  .size = ship_size(activeButton)};
        player_setPlayerShip(&player, s);
      }
      break;
    }
    case BUTTON_EV: {
      mouse_reset();
      break;
    }
    case RB_PRESSED:
    case LB_RELEASED:
    case RB_RELEASED:
    case MOUSE_MOV:
      break;
  }
}

////////////////////////////////////////////////////////////////
// Keyboard handlers
////////////////////////////////////////////////////////////////

void waiting_kbd_handler(struct kbd_event event) {
  if (!event.pressed && !event.twoBytes && event.codes[1] == ESC_BREAK) {
    game_change_state(mainMenu);
  }
}

void victory_kbd_handler(struct kbd_event event) {
  if (!event.pressed && !event.twoBytes && event.codes[1] == ESC_BREAK) {
    game_change_state(mainMenu);
  }
}

void loss_kbd_handler(struct kbd_event event) {
  if (!event.pressed && !event.twoBytes && event.codes[1] == ESC_BREAK) {
    game_change_state(mainMenu);
  }
}

void oponentChoosing_kbd_handler(struct kbd_event event) {
  if (!event.pressed && !event.twoBytes && event.codes[1] == ESC_BREAK) {
    serial_addByteToSend(VICT_MSG);
    game_change_state(loss);
  }
}

void botmenu_kbd_handler(struct kbd_event event) {
  if (event.pressed && !event.twoBytes && event.codes[1] == ENTER_MAKE_CODE) {
    switch (botop_selec) {
      case dumb:
        game_change_state(building);
        gameType = PVE;
        bot_coords = &default_bot_get_coords;
        return;
      case absolute:
        game_change_state(building);
        gameType = PVE;
        bot_coords = &improved_bot_get_coords;
        return;
    }
  }
  if (event.pressed && event.twoBytes && event.codes[1] == UP_ARROW_MAKE_CODE &&
      botop_selec != dumb) {
    botop_selec--;
    return;
  }
  if (event.pressed && event.twoBytes &&
      event.codes[1] == DOWN_ARROW_MAKE_CODE && botop_selec != absolute) {
    botop_selec++;
    return;
  }
  if (!event.pressed && !event.twoBytes && event.codes[1] == ESC_BREAK) {
    game_change_state(mainMenu);
    botop_selec = dumb;
    return;
  }
}

void mainmenu_kbd_handler(struct kbd_event event) {
  if (event.pressed && !event.twoBytes && event.codes[1] == ENTER_MAKE_CODE) {
    switch (mainmenu_selec) {
      case vs_bot:
        // game_change_state(building);
        game_change_state(botMenu);
        gameType = PVE;
        return;
      case vs_player:
        game_change_state(building);
        gameType = PVP;
        return;
    }
  }
  if (event.pressed && event.twoBytes && event.codes[1] == UP_ARROW_MAKE_CODE &&
      mainmenu_selec != vs_bot) {
    mainmenu_selec = vs_bot;
    return;
  }
  if (event.pressed && event.twoBytes &&
      event.codes[1] == DOWN_ARROW_MAKE_CODE && mainmenu_selec != vs_player) {
    mainmenu_selec = vs_player;
  }
  if (!event.pressed && !event.twoBytes && event.codes[1] == ESC_BREAK) {
    gameRunning = false;
  }
}

void building_kbd_handler(struct kbd_event event) {
  if (event.pressed && !event.twoBytes && event.codes[1] == R_MAKE_CODE) {
    orientationVertical = !orientationVertical;
  }

  if (event.pressed && !event.twoBytes && event.codes[1] == P_MAKE_CODE) {
    game_finished_building();
  }

  if (event.pressed && !event.twoBytes && event.codes[1] == G_MAKE_CODE) {
    game_generate_board(&player);
  }
  if (!event.pressed && !event.twoBytes && event.codes[1] == ESC_BREAK) {
    game_change_state(mainMenu);
  }
  if (event.pressed && !event.twoBytes && event.codes[1] == ONE_MAKE_CODE) {
    game_activate_building_button(0);
  }
  if (event.pressed && !event.twoBytes && event.codes[1] == TWO_MAKE_CODE) {
    game_activate_building_button(1);
  }
  if (event.pressed && !event.twoBytes && event.codes[1] == THREE_MAKE_CODE) {
    game_activate_building_button(2);
  }
  if (event.pressed && !event.twoBytes && event.codes[1] == FOUR_MAKE_CODE) {
    game_activate_building_button(3);
  }
  if (event.pressed && !event.twoBytes && event.codes[1] == FIVE_MAKE_CODE) {
    game_activate_building_button(4);
  }
  if (event.pressed && !event.twoBytes && event.codes[1] == D_MAKE_CODE) {
    game_activate_building_button(5);
  }
}

void playerChoosing_kbd_handler(struct kbd_event event) {
  if (event.pressed && !event.twoBytes && event.codes[1] == S_MAKE_CODE) {
    watchingPlayerBoard = !watchingPlayerBoard;
  }
  if (!event.pressed && !event.twoBytes && event.codes[1] == ESC_BREAK) {
    serial_addByteToSend(VICT_MSG);
    game_change_state(loss);
  }
}

////////////////////////////////////////////////////////////////
// XPM DRAWERS
////////////////////////////////////////////////////////////////

void game_draw_buildhelp() {
  draw_sprite(&buildhelp, 10, tableYStart);
}

void game_draw_shootophelp() {
  draw_sprite(&shootophelp, 10, tableYStart);
}

void game_draw_shootplayer() {
  draw_sprite(&shootplayerhelp, 10, tableYStart);
}

void game_draw_losescreen() {
  draw_sprite(&losescreen, 0, 0);
}

void game_draw_winscreen() {
  draw_sprite(&winscreen, 0, 0);
}

void game_draw_turn(bool yturn) {
  yturn ? draw_sprite(&yourturn, 0, 0) : draw_sprite(&opturn, 0, 0);
}

void game_ship_draw() {
  if (!watchingPlayerBoard) {
    return;
  }
  int cellStep = tableGap + squareSide;
  if (currentState == building && activeButton != -1 && activeButton < 5) {
    uint mouseX = mouse_getX();
    uint mouseY = mouse_getY();
    if (mouseX < tableXStart) {
      goto afterPreview;
    }

    if (mouseY < tableYStart) {
      goto afterPreview;
    }

    int cellY = -1;
    int cellX = -1;
    for (int y = 1; y <= 10; y++) {  // Checks which row was clicked
      if (mouseY < tableYStart + y * (tableGap + squareSide)) {
        cellY = y - 1;
        break;
      }
    }

    if (cellY == -1) {
      goto afterPreview;
    }

    for (int x = 1; x <= 10; x++) {  // Checks which col was clicked
      if (mouseX < tableXStart + x * (tableGap + squareSide)) {
        cellX = x - 1;
        break;
      }
    }

    if (cellX != -1) {
      if (orientationVertical && cellY + ship_size(activeButton) > 10) {
        goto afterPreview;
      } else if (!orientationVertical && cellX + ship_size(activeButton) > 10) {
        goto afterPreview;
      }
      draw_sprite_ship(&test_ships[activeButton],
                       tableXStart + cellStep * cellX,
                       tableYStart + cellStep * cellY, orientationVertical);
    }
  }
afterPreview:
  for (int i = 0; i < player.battleships_size; i++) {
    ship* s = &player.battleships[i];
    if (s->exists) {
      draw_sprite_ship(&test_ships[s->st], tableXStart + cellStep * s->x,
                       tableYStart + cellStep * s->y, s->vertical);
    }
  }
}

void game_advance_animations() {
  bool removed = false;
  for (uint i = 0; i < currentAnimations.size;) {
    removed = false;
    AniSprite* asp = list_get(&currentAnimations, i);

    asp->ticks_to_next_frame++;
    if (asp->ticks_to_next_frame % (uint)(60 / asp->fps) == 0) {
      asp->ticks_to_next_frame = 0;
      asp->currentframe++;
      if (asp->currentframe >= asp->size) {
        if (!asp->looping) {
          removed = true;
          free(asp->frames);
          list_remove(&currentAnimations, i);
        } else {
          asp->currentframe %= asp->size;
        }
      }
    }

    if (!removed) {
      i++;
    }
  }
}

void game_render_current_state() {
  switch (currentState) {
    case mainMenu: {
      int buttonx = getHres() / 4 - mainmenu_vsbot.info.width / 2;
      int cursor_x = buttonx - mainmenu_cursor.info.width;
      int cursor_y;
      draw_sprite(&background_mainmenu, 0, 0);
      draw_sprite(&mainmenu_vsbot, buttonx, getVres() / 2);
      draw_sprite(&mainmenu_vsplayer, buttonx, getVres() / 2 + 100);
      switch (mainmenu_selec) {
        case vs_bot:
          cursor_y = getVres() / 2 + mainmenu_vsbot.info.height / 2 -
                     mainmenu_cursor.info.height / 2;
          break;
        case vs_player:
          cursor_y = getVres() / 2 + 100 + mainmenu_vsplayer.info.height / 2 -
                     mainmenu_cursor.info.height / 2;
          break;
      }
      draw_sprite(&mainmenu_cursor, cursor_x, cursor_y);
      break;
    }
    ///////////////////////////////////////////////
    case botMenu: {
      int startx = getHres() / 2 - bdumb.info.width / 2;
      int starty = getVres() / 3;
      int cursor_x = startx - mainmenu_cursor.info.width;
      int cursor_y;
      draw_sprite(&botback, 0, 0);
      draw_sprite(&bdumb, startx, starty);
      draw_sprite(&babsolute, startx, starty + (bdumb.info.height + 100));
      // draw_sprite(&babsolute, startx, starty + 2 * (bdumb.info.height +
      // 100));

      switch (botop_selec) {
        case dumb:
          cursor_y =
              starty + bdumb.info.height / 2 - mainmenu_cursor.info.height / 2;
          break;
        case absolute:
          cursor_y = starty + (bdumb.info.height + 100) +
                     bdumb.info.height / 2 - mainmenu_cursor.info.height / 2;
          break;
          // case absolute:
          //   cursor_y = starty + 2 * (bdumb.info.height + 100) +
          //              bdumb.info.height / 2 - mainmenu_cursor.info.height /
          //              2;
          //   break;
      }

      draw_sprite(&mainmenu_cursor, cursor_x, cursor_y);
      break;
    }
    //////////////////////////////////////////////7
    case building: {
      Sprite* tmp_sprite;
      draw_table(player.player_board);

      for (int i = 0; i < 6; i++) {
        Button* tmp = list_get(build_buttons, i);
        if (tmp == 0) {
          continue;
        }
        if (tmp->pressed) {
          if (i == 5) {
            tmp_sprite = &buttons_sprite[3];
          } else {
            tmp_sprite = &buttons_sprite[1];
          }
        } else {
          if (i == 5) {
            tmp_sprite = &buttons_sprite[2];
          } else {
            tmp_sprite = &buttons_sprite[0];
          }
        }

        draw_sprite(tmp_sprite, tmp->x, tmp->y);
        if (i == 5) {
          draw_sprite(&deletemode, tmp->x + tmp_sprite->info.width, tmp->y);
        } else {
          draw_sprite(&test_ships[i], tmp->x + squareSide, tmp->y);
        }
      }
      game_ship_draw();
      game_draw_buildhelp();
      mouse_draw_mouse();
      break;
    }
    /////////////////////////////////////////
    case afterShotWaiting: {
      // render message
      game_draw_turn(true);
      draw_table(player.oponent_board);
      game_draw_shootophelp();
      mouse_draw_mouse();
      break;
    }
    ///////////////////////////////////////////////////////
    case playerChoosingTransition:
    case playerChoosing: {
      game_draw_turn(true);
      if (watchingPlayerBoard) {
        game_draw_shootplayer();
        draw_table(player.player_board);
        game_ship_draw();
        game_render_animations();
      } else {
        game_draw_shootophelp();
        draw_table(player.oponent_board);
        game_ship_draw();
      }
      mouse_draw_mouse();
      break;
    }
    /////////////////////////////////
    case oponentChoosingTransition:
    case oponentChoosing: {
      game_draw_turn(false);
      draw_table(player.player_board);
      game_ship_draw();
      game_draw_shootplayer();
      game_render_animations();
      break;
    }
    ///////////////////////////////////////
    case waiting: {
      // vg_draw_rectangle(0, 0, 100, 100, 0xff00ff);
      draw_sprite(&waitscreen, 0, 0);
      break;
    }
    ////////////////////////////////
    case victory: {
      // vg_draw_rectangle(0, 0, 100, 100, 0x0000ff00);
      game_draw_winscreen();
      break;
    }
    /////////////////////////////////////////
    case loss: {
      // vg_draw_rectangle(0, 0, 100, 100, 0x00ff0000);
      game_draw_losescreen();
      break;
    }
    /////////////////////////////////////
    default:
      break;
  }
}

void game_render_animations() {
  for (uint i = 0; i < currentAnimations.size; i++) {
    AniSprite* asp = list_get(&currentAnimations, i);
    draw_sprite(&asp->frames[asp->currentframe], asp->x, asp->y);
  }
}

////////////////////////////////////////////////////////////////
// XPM LOADERS
////////////////////////////////////////////////////////////////

void game_load_all_xpm() {
  game_buttons_load();
  game_explosions_load();
  game_mainmenu_load();
  mouse_load_sprite_xpm();
  game_ship_load();
  game_load_flame();

  // Helpers
  xpm_load(shootplayer_xpm, XPM_8_8_8, &shootplayerhelp.info);
  xpm_load(shootoponent_xpm, XPM_8_8_8, &shootophelp.info);
  xpm_load(buildinghelp_xpm, XPM_8_8_8, &buildhelp.info);

  // winscreen and losescreen
  xpm_load(winscreen_xpm, XPM_8_8_8, &winscreen.info);
  xpm_load(losescreen_xpm, XPM_8_8_8, &losescreen.info);

  // Turn titles
  xpm_load(yourturn_xpm, XPM_8_8_8, &yourturn.info);
  xpm_load(opturn_xpm, XPM_8_8_8, &opturn.info);

  // Turn waitscreen
  xpm_load(waitscreen_xpm, XPM_8_8_8, &waitscreen.info);

  // Bot Menu
  xpm_load(dumbbot_xpm, XPM_8_8_8, &bdumb.info);
  // xpm_load(slightsmartbot_xpm, XPM_8_8_8, &bslight.info);
  xpm_load(absolutebot_xpm, XPM_8_8_8, &babsolute.info);
  xpm_load(botback_xpm, XPM_8_8_8, &botback.info);
}

void game_mainmenu_load() {
  xpm_load(backgroundtitle_xpm, XPM_8_8_8, &background_mainmenu.info);
  xpm_load(vsbot_xpm, XPM_8_8_8, &mainmenu_vsbot.info);
  xpm_load(vsplayer_xpm, XPM_8_8_8, &mainmenu_vsplayer.info);
  xpm_load(main_cursor_xpm, XPM_8_8_8, &mainmenu_cursor.info);
}

void game_ship_load() {
  xpm_load(cruiser_xpm, XPM_8_8_8, &test_ships[2].info);
  xpm_load(carrier_xpm, XPM_8_8_8, &test_ships[4].info);
  xpm_load(battleship_xpm, XPM_8_8_8, &test_ships[3].info);
  xpm_load(destroyer_xpm, XPM_8_8_8, &test_ships[0].info);
  xpm_load(submarine_xpm, XPM_8_8_8, &test_ships[1].info);
}

void game_explosions_load() {
  explosion.x = 0;
  explosion.y = 0;
  explosion.size = 9;
  explosion.ticks_to_next_frame = 0;
  explosion.fps = 10;
  explosion.currentframe = 0;
  explosion.looping = false;
  explosion.frames = malloc(sizeof(Sprite) * 9);
  xpm_load(exp1_xpm, XPM_8_8_8, &explosion.frames[0].info);
  xpm_load(exp2_xpm, XPM_8_8_8, &explosion.frames[1].info);
  xpm_load(exp3_xpm, XPM_8_8_8, &explosion.frames[2].info);
  xpm_load(exp4_xpm, XPM_8_8_8, &explosion.frames[3].info);
  xpm_load(exp5_xpm, XPM_8_8_8, &explosion.frames[4].info);
  xpm_load(exp6_xpm, XPM_8_8_8, &explosion.frames[5].info);
  xpm_load(exp7_xpm, XPM_8_8_8, &explosion.frames[6].info);
  xpm_load(exp8_xpm, XPM_8_8_8, &explosion.frames[7].info);
  xpm_load(exp9_xpm, XPM_8_8_8, &explosion.frames[8].info);
}

void game_buttons_load() {
  build_buttons = malloc(sizeof(List));
  buttons_sprite = malloc(sizeof(Sprite) * 4);
  xpm_load(sqrbutton_xpm, XPM_8_8_8, &buttons_sprite[0].info);
  xpm_load(sqrbuttonpressed_xpm, XPM_8_8_8, &buttons_sprite[1].info);
  xpm_load(circbuttonno_xpm, XPM_8_8_8, &buttons_sprite[2].info);
  xpm_load(circbuttonyes_xpm, XPM_8_8_8, &buttons_sprite[3].info);
  xpm_load(deletemode_xpm, XPM_8_8_8, &deletemode.info);
}

void game_load_flame() {
  flame.x = 0;
  flame.y = 0;
  flame.size = 13;
  flame.ticks_to_next_frame = 0;
  flame.fps = 10;
  flame.currentframe = 0;
  flame.looping = true;

  flame.frames = malloc(sizeof(Sprite) * 13);
  xpm_load(fire1_xpm, XPM_8_8_8, &flame.frames[0].info);
  xpm_load(fire2_xpm, XPM_8_8_8, &flame.frames[1].info);
  xpm_load(fire3_xpm, XPM_8_8_8, &flame.frames[2].info);
  xpm_load(fire4_xpm, XPM_8_8_8, &flame.frames[3].info);
  xpm_load(fire5_xpm, XPM_8_8_8, &flame.frames[4].info);
  xpm_load(fire6_xpm, XPM_8_8_8, &flame.frames[5].info);
  xpm_load(fire7_xpm, XPM_8_8_8, &flame.frames[6].info);
  xpm_load(fire8_xpm, XPM_8_8_8, &flame.frames[7].info);
  xpm_load(fire9_xpm, XPM_8_8_8, &flame.frames[8].info);
  xpm_load(fire10_xpm, XPM_8_8_8, &flame.frames[9].info);
  xpm_load(fire11_xpm, XPM_8_8_8, &flame.frames[10].info);
  xpm_load(fire12_xpm, XPM_8_8_8, &flame.frames[11].info);
  xpm_load(fire13_xpm, XPM_8_8_8, &flame.frames[12].info);
}

