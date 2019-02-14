#include "keyboard.h"
#include "lcom/lcf.h"
#include "rtc.h"
#include "serial.h"

/** @defgroup event event
 * @{
 * 
 */

/**
 * @brief Reset's the handler function pointers to NULL
 * 
 */
void reset_handlers();

/**
 * @brief Raise a mouse button event
 * 
 * Call's the function pointed by the mouse_btn_handler function pointer.
 * Passes the event in the argument to the argument in the pointed function.
 * 
 * @param event Mouse event to pass
 */
void raise_mouse_btn_event(struct mouse_ev event);
/**
 * @brief Set the mouse btn handler function pointer
 * 
 * @param handler Pointer to save
 */
void set_mouse_btn_handler(void (*handler)(struct mouse_ev event));

/**
 * @brief Raise a keyboard event
 * 
 * Call's the function pointed by the kbd_handler function pointer.
 * Passes the event in the argument to the argument in the pointed function.
 * 
 * @param event Keyboard event to pass
 */
void raise_keyboard_event(struct kbd_event event);
/**
 * @brief Set the keyboard handler function pointer 
 * 
 * @param handler Pointer to save
 */
void set_keyboard_handler(void (*handler)(struct kbd_event event));

/**
 * @brief Raise a serial event
 * 
 * Call's the function pointed by the serial_handler function pointer.
 * Passes the event in the argument to the argument in the pointed function.
 * 
 * @param event Keyboard event to pass
 */
void raise_serial_event(struct serial_event event);
/**
 * @brief Set the serial handler function pointer
 * 
 * @param handler Pointer to save
 */
void set_serial_handler(void (*handler)(struct serial_event event));

/**
 * @brief Raise an rtc event
 * 
 * Call's the function pointed by the rtc_handler function pointer.
 * Passes the event in the argument to the argument in the pointed function.
 * 
 * @param event RTC event to pass
 */
void raise_rtc_event(struct rtc_event event);
/**
 * @brief Set the rtc handler function pointer
 * 
 * @param handler Pointer to save
 */
void set_rtc_handler(void (*handler)(struct rtc_event event));

/**@}*/
