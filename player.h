#ifndef _PLAYERH
#define _PLAYERH

#include "gameStructs.h"
#include "stdbool.h"
#include "stdint.h"
#include "stdlib.h"

/** @defgroup player player
 * @{
 * 
 */

/**
 * @brief Struct representing a player.
 * 
 */
typedef struct {
  uint32_t player_board[10][10]; /**< @brief Player's current view of his board. */
  uint32_t oponent_board[10][10]; /**< @brief Player's current view of the oponent's board. */
  /**
   * @brief Ship array
   * 
   * Array of the player's ships.
   * Appear in the following order: destroyer, submarine, cruiser, battleship, carrier.
   * 
   */
  ship* battleships;
  int battleships_size; /**< @brief Number of ships the player has. */
} Player;

/**
 * @defgroup BOARD_MACROS BOARD_MACROS
 * 
 * The boards are implemented as bidimensional arrays of RGB colors.
 * So, the following macros abstract the colors and what they mean in the game.
 * 
 * @{
 */

#define OPONENT_UNKNOWN 0x003d3d3d /**< @brief Represents unknown territory in oponent_board. */
#define OCEAN 0x004286f4/**< @brief Represents ocean on both views, a miss in oponent_board. */
#define PLAYER_SHIP_HIT 0x00ff0000/**< @brief Represents hits on the player_board. */
#define PLAYER_SHIP 0x003d3d3d/**< @brief Represents a ship in the player_board. */
#define OPONENT_TRY 0x00ff00ff/**< @brief Represents a missed shot on the player_board. */
#define OPONENT_SHIP_HIT 0x002eff00/**< @brief Represents a hit on the oponent_board. */

/**@}*/

/**
 * @brief Create a player object.
 * 
 * Initializes a player struct and returns it.
 * 
 * @return Player The initialized struct.
 */
Player player_init();

/**
 * @brief Places a ship in the player board.
 * 
 * Places a given ship in the plaeyr's player_board.
 * Will fail if the ship doesn't fit in the given position, or overlaps another ship. 
 * 
 * @param self Pointer to the player.
 * @param s Ship to be inserted.
 * @return int 0 if it was successful, non-zero otherwise.
 */
int player_setPlayerShip(Player* self, ship s);
/**
 * @brief Remove a ship from the player board.
 * 
 * Removes the first ship of the specified type from the player's board.
 * 
 * @param self Pointer to the plasyer.
 * @param type Type of the ship.
 */
void player_removeShip(Player* self, shiptype type);
/**
 * @brief Emulate a shot at player.
 * 
 * Tries a shot at a player at sets the value in the board according to the result.
 * 
 * @param self Pointer to the player.
 * @param x X coordinate of the shot in the board.
 * @param y Y coordinate of the shot in the board.
 * @return true If it was a hit.
 * @return false If it was a miss.
 */
bool player_shootPlayer(Player* self, uint32_t x, uint32_t y);
/**
 * @brief Checks if the player's fleet has been sunk.
 * 
 * @param self Pointer to the player.
 * @return true If the player_board has no more PLAYER_SHIP cells.
 * @return false If the player_board still has PLAYER_SHIP cells.
 */
bool player_checkIfFleetDestroyed(Player* self);
/**
 * @brief Get the cell color in the player's opoenent_board at x,y coordinates.
 * 
 * @param self Pointer to the player.
 * @param x X coordinate.
 * @param y Y coordinate.
 * @return uint32_t 
 */
uint32_t player_getOponentCellColor(Player* self, uint32_t x, uint32_t y);

/**@}*/

#endif
