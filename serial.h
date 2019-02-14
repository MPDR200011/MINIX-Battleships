#ifndef SERIALH
#define SERIALH
#include "stdint.h"

/** @defgroup serial serial
 * @{
 * 
 */

/**
 * @brief Struct to identify a serial event
 */
struct serial_event {
  uint8_t MSG; /**< @brief Message */
  uint8_t X; /**< @brief X shot coordinate, in case of shot message. */
  uint8_t Y; /**< @brief Y shot coordinate, in case of shot mesasge. */
};

/*
 * Protocol:
 *  5 bits per char
 *  1 stop bit
 *  No parity
 */

// Protocol
#define READY_MSG 0x01 /**< @brief Player ready notification */
#define SHOT_HEADER 0x02 /**< @brief Header of the message containing a shot's coordinates */
/**
 * @brief Player victory message
 * 
 * Sent by the oponent to indicate the player's victory
 * 
 */
#define VICT_MSG 0x03
/**
 * @brief Accurate shot message
 * 
 * Sent by oponent to tell that payer's shot was accurate
 * 
 */
#define HIT_MSG 0x05
/**
 * @brief Player timeout message
 * 
 * Sent by the player to tell oponent he has axceeded 10 seconds turn limit
 * 
 */
#define MISS_MSG 0x06
/**
 * @brief Synchronization message
 * 
 * Used to tell that one player was already waiting for the other.
 * This is useful when the player sets ready before the other even had turned the game on.
 * 
 */
#define LATE_MSG 0x07

#define COM_IRQ 4 /**< @brief Serial port irq line. */
#define COM_BASE 0x3F8 /**< @brief Base address of serial port's registers. */
#define COM_BITRATE 1200 /**< @brief Serial port bitrate. */

#define REC_BUFFER COM_BASE/**< @brief Receiver buffer address. */
#define TRANS_BUFFER COM_BASE /**< @brief Transmitter buffer address. */

// LCR config
#define LCR_PORT (COM_BASE + 3) /**< @brief LCR register address. */
#define BPC_5 0x00 /**< @brief 5 bits per second setting. */
#define BPC_8 0x03 /**< @brief 8 bits per second setting. */
#define STOP_BITS_1 (0x00 << 2) /**< @brief 1 stop bit setting. */
#define STOP_BITS_2 (0x01 << 2) /**< @brief 2 stop bit setting. */
#define NO_PARITY (0x00 << 3) /**< @brief No parity setting. */
#define ODD_PARITY (0x01 << 3) /**< @brief Odd parity setting. */
#define LCR_DLAB (0x01 << 7) /**< @brief Dvivisor latch access bit. */
#define PROG_SERIAL_CONF (BPC_5 | STOP_BITS_1 | NO_PARITY) /**< @brief Complete serial port config. */
#define LCR_CLEAR_CONFIG (0x03 << 6) /**< @brief Mask to clear the config register, but leave the DLAB and Break control bits alone. */

// IIR
#define IIR_PORT (COM_BASE + 2) /**< @brief IIR register address. */
#define IIR_NO_INT_PENDING 0x01 /**< @brief No interrupts pending mask. */
#define IIR_INT_DATA (0x07 << 1) /**< @brief Interrupts indentification mask. */
#define IIR_CHAR_TIMEOUT (0x06 << 1) /**< @brief Character timeout mask. */
#define IIR_REC_DATA_AV (0x02 << 1) /**< @brief Received data mask. */
#define IIR_THR_EMPTY (0x01 << 1) /**< @brief THR impty mask. */

// IER
#define IER_PORT (COM_BASE + 1) /**< @brief IER ragister address. */
#define DATA_RECEIVE_INT (0x01 << 0) /**< @brief Enable data received interrupt bit. */
#define THR_EMPTY_INT (0x01 << 1) /**< @brief Enable THR empty interrupt bit */

// FCR
#define FCR_PORT (COM_BASE + 2) /**< @brief FCR register address. */
#define ENABLE_BOTH_FIFOS 0x01 /**< @brief Enalbe FIFOs bit. */
#define FIFO_TRIG_LEVEL_1 (0x00 << 6) /**< @brief FIFO trigger level 1. */
#define FIFO_TRIG_LEVEL_4 (0x01 << 6) /**< @brief FIFO trigger level 4. */

// LSR
#define LSR_PORT (COM_BASE + 5) /**< @brief LSR register address. */
#define LSR_REC_DATA 0x01 /**< @brief Receiver data ready mask. */

/**
 * @brief Initialize the serial port with the game's settings.
 * 
 * @param serial_irq Variable in which to write the serial port's irq mask.
 * @return int 0 on success, -1 otherwise.
 */
int serial_init(uint8_t* serial_irq);
/**
 * @brief Unsubscribes serial ports interrupts.
 * 
 * @return int 
 */
int serial_exit();
/**
 * @brief Serial port interrupt handler.
 * 
 */
void serial_ih();
/**
 * @brief Extract data for receiver FIFO to receiver queue.
 * 
 */
void serial_receiveData();
/**
 * @brief Send data from transmitter queue to transmitter FIFO.
 * 
 */
void serial_sendData();
/**
 * @brief Process data from the receiver queues.
 * 
 */
void serial_processData();
/**
 * @brief Add byte to transmitter queues.
 * 
 * @param byte Byte to add.
 * @return int 0 on success and non 0 otherwise.
 */
int serial_addByteToSend(uint8_t byte);

/**@}*/

#endif

