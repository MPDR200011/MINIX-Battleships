#ifndef RTC_H
#define RTC_H

#include "stdint.h"

/** @defgroup rtc rtc
 * @{
 * 
 */

/**
 * @brief Enumaration of rtc event types
 * 
 */
enum rtc_event_type { 
  timeoutAlarm /**< @brief Alarm event type. */
  };

/**
 * @brief RTC event struct.
 * 
 */
struct rtc_event {
  enum rtc_event_type type; /**< @brief Event type. */
};

#define RTC_IRQ 8/**< @brief RTC irq line. */
#define RTC_ADDR_REG 0x70/**< @brief RTC address register. */
#define RTC_DATA_REG 0x71/**< @brief RTC data register. */

#define SEC_LOC 0/**< @brief Inner RTC seconds location. */
#define SEC_AL_LOC 1/**< @brief Inner RTC alarm seconds location */
#define MIN_LOC 2/**< @brief Inner RTC alarm minutes location */
#define MIN_AL_LOC 3/**< @brief Inner RTC alarm minutes location */
#define HOUR_LOC 4/**< @brief Inner RTC alarm seconds location */
#define HOUR_AL_LOC 5/**< @brief Inner RTC alarm seconds location */

#define REGB_LOC 11/**< @brief Inner RTC Register B location. */
#define REGC_LOC 12/**< @brief Inner RTC Register C location. */

#define UPDATE_INT (1 << 4)/**< @brief Time update interrupt bit. */
#define ALARM_INT (1 << 5)/**< @brief Alarm interrupt bit. */
#define IRQF (1 << 7)/**< @brief Interrupt pending bit. */

/**
 * @brief Initialize RTC config and subsscribe interrupts.
 * 
 * Enables update and alarm interrupts and subscribes the timer interrupts.
 * 
 * @param rtc_irq Variable in which to write RTC irq mask.
 * @return int 0 on success.
 */
int rtc_init(uint16_t* rtc_irq);
/**
 * @brief Unsubscribe RTC interrupts.
 * 
 * @return int 0 on success.
 */
int rtc_exit();

/**
 * @brief RTC interrupt handler.
 * 
 * If it is an update interrupt, it will extract the date.
 * 
 */
void rtc_ih();

/**
 * @brief Set an alarm s seconds in the future.
 * 
 * @param s Number of seconds in the future to set the alarm.
 */
void rtc_set_timeout(uint32_t s);
/**
 * @brief Convert an unsigned two digit BCD number to binary. 
 * 
 * @param bcd The BCD number to convert.
 * @return uint8_t The number in binary.
 */
uint8_t BCDtoINT(uint8_t bcd);
/**
 * @brief Convert an unsigned binary number to a two digit BCD number.
 * 
 * WARNING: Since a two digit number in BCD can only be 99 at max, if a number over 99 is passed, 
 * the function will return 99.
 * 
 * @param i Binary number to convert.
 * @return uint8_t Number in BCD.
 */
uint8_t INTtoBCD(uint8_t i);

/**
 * @brief Get data from an RTC register.
 * 
 * Gets data from an RTC register through its specified location.
 * 
 * @param loc Location of the register in the RTC.
 * @param data Variable in which to store the data.
 * @return int 0 on success.
 */
int rtc_get_data(uint32_t loc, uint8_t* data);
/**
 * @brief Set date in an RTC register.
 * 
 * Set the data in an RTC register though its specified location.
 * 
 * @param loc Location of the register in the RTC.
 * @param data Data to send to the register,
 * @return int 0 on success.
 */
int rtc_send_data(uint32_t loc, uint8_t data);

/**@}*/

#endif
