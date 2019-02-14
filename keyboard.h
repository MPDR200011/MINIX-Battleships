#ifndef KBD_H
#define KBD_H

#include <stdint.h>
#include <stdlib.h>
#include <lcom/lcf.h>
#include "i8042.h"

/** @defgroup keyboard keyboard
 * @{
 * 
 */

/**
 * @brief Struct representing a keyboard event,
 * 
 */
struct kbd_event {
	bool pressed; /**< @brief Tells if the event was of a key press or release, true means pressed. */
	bool twoBytes; /**< @brief Tells if the scan code of the key is of two bytes. */
	uint8_t codes[2]; /**< @brief Key scan codes. */
};

/**
 * @brief Subscribe keyboard interrupts.
 * 
 * @param bit_no Variable to wich to write keyboard irq mask.
 * @return int 0 on success.
 */
int kbd_subscribe_int(uint8_t *bit_no);
/**
 * @brief Unsubscribe keyboard interupts. 
 * 
 * @return int 0 on success.
 */
int kbd_unsubscribe_int();
/**
 * @brief Extract data from a given port
 * 
 * @param port Address of the port from which to extract data
 * @param status Variable where data will be put.
 * @return int 0 on success.
 */
int kbd_get_data(uint8_t port,uint8_t *status);
/**
 * @brief Set the data in a given port
 * 
 * @param port Address of the port to which to write data.
 * @param status Data to write.
 * @return int 0 on success.
 */
int kbd_set_data(uint8_t port,uint8_t status);
/**
 * @brief Enable keyboard interrupts.
 * 
 * @return int 0 on success.
 */
int kbc_enable_inter();
/**
 * @brief Process the byte extrected from the output buffer.
 * 
 */
void kbd_process_byte();
/**
 * @brief Keyboard interrupt handler.
 * 
 * Extracts the keyboard byte from the output buffer.
 * 
 */
void (kbc_ih)(void);

/**@}*/

#endif

