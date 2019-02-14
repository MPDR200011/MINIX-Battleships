#include "sys/stdint.h"
#include "lcom/lcf.h"

/** @defgroup mouse mouse
 * @{
 * 
 */

/**
 * @brief Reset the mouse position on the screen.
 * 
 */
void mouse_reset();

/**
 * @brief Subscribe mouse interrupts.
 * 
 * @param bit_no Variable to which the mouse irq mask will be written.
 * @return int 0 on success.
 */
int mouse_subscribe_int(uint32_t *bit_no);
/**
 * @brief Unsubscribe mouse interrupts.
 * 
 * @return int 0 on success. 
 */
int mouse_unsubscribe_int();
/**
 * @brief Process extracted byte.
 * 
 * Processes byte extracted from KBC output buffer and creates a mouse packet when possible.
 * 
 */
void mouse_generatePacket();

/**
 * @brief Check it's possible to send commands to the mouse.
 * 
 * @return true If it's possible. 
 * @return false If it isn't possible.
 */
bool canSendCommand();

/**
 * @brief Reset the KBC default byte.
 * 
 * Put the KBC default control byte in its control register.
 * 
 * @return int 0 on success.
 */
int kbc_reset_default_byte();

/**
 * @brief Enable mouse.
 * 
 * Tells the KBC to enable the mouse.
 * 
 * @return int 0 on success.
 */
int mouse_enable();
/**
 * @brief Disable mouse.
 * 
 * Tells the KBC to disable the mouse.
 * 
 * @return int 0 on success.
 */
int mouse_disable();
/**
 * @brief Enable mouse data reporting.
 * 
 * @return int 0 on success.
 */
int mouse_enable_data_rep();
/**
 * @brief Disable mouse data reporting.
 * 
 * @return int 0 on success. 
 */
int mouse_disable_data_rep();
/**
 * @brief Set mouse stream mode.
 * 
 * @return int 0 on success.
 */
int mouse_set_stream_mode();
/**
 * @brief Set mouse remote mode.
 * 
 * @return int 0 on success.
 */
int mouse_set_remote_mode();
/**
 * @brief Load the mouse's sprite.
 * 
 * @return int 0 on success.
 */
int mouse_load_sprite_xpm();
/**
 * @brief Generate mouse event.
 * 
 * Keeps track of mouse state and generates mouse events based on that and the packets it receives.
 * 
 * @param pp Pointer to mouse packet.
 * @return struct mouse_ev Mouse event.
 */
struct mouse_ev mouse_make_event(struct packet *pp);

/**
 * @brief Draw the mouse sprite.
 * 
 */
void mouse_draw_mouse();

/**
 * @brief Get the mouse X coordinate on the screen.
 * 
 * @return int 
 */
int mouse_getX();
/**
 * @brief Get the mouse Y coordinate on the screen.
 * 
 * @return int 
 */
int mouse_getY();

/**@}*/
