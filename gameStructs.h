#ifndef _STRUCTSH
#define _STRUCTSH

#include "lcom/lcf.h"

/** @defgroup gameStructs gameStructs
 * @{
 * 
 */

/**
 * @brief Enumeration of ship types.
 * 
 */
typedef enum {
  destroyer,   // size 2
  submarine,   // size 3
  cruiser,     // size 3
  battleship,  // size 4
  carrier,     // size 5
} shiptype;

/**
 * @brief Ship representation.
 * 
 */
typedef struct {
  bool exists;  /**< @brief If the ship was built. Useful in the building phase. */
  shiptype st; /**< @brief Type of the ship. */
  bool vertical; /**< @brief If the ship is in vertical orientation. */
  int x; /**< @brief X coordinate of the ship's first cell. */
  int y; /**< @brief X coordinate of the ship's first cell. */
  bool* hits; /**< @brief  */
  uint size;  /**< @brief Ship size. */
} ship;

// Button class
/**
 * @brief Struct representing button.
 * 
 */
typedef struct {
  uint16_t x; /**< @brief X coordinate of the button on the screen. */
  uint16_t y; /**< @brief Y coordinate of the button on the screen. */
  uint16_t height; /**< @brief Button height. */
  uint16_t width; /**< @brief Button width. */
  bool pressed; /**< @brief If the button is pressed. */
} Button;

#endif

/**@}*/
