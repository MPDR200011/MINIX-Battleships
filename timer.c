#include <lcom/lab2.h> 
#include <lcom/lcf.h>
#include <minix/syslib.h>
#include <stdint.h>

#include "i8254.h"   


uint32_t mouse_interrupt_count = 0; 
int timer_idHook = 0;     

int(util_get_LSB)(uint16_t val, uint8_t *lsb) {
  
  *lsb = (uint8_t) val;

  return 0;
}

int(util_get_MSB)(uint16_t val, uint8_t *msb) {
  
  *msb = (uint8_t) (val >> 8);

  return 0;
}

int(timer_set_frequency)(uint8_t timer, uint32_t freq) {

  if (timer > 2 || timer < 0) {
    return 1;   
  }  

  //grabing timer config
  uint8_t timerConfig; 
  timer_get_conf(timer, &timerConfig);

  //extracting counting mode and BCD bits
  uint8_t timerModeAndBCD = (uint8_t)(timerConfig & TIMER_BCD) |
                            (timerConfig & (TIMER_SQR_WAVE | BIT(3)));

  //setting bits specifying timer
  uint8_t timerSelection = 0;
  switch (timer) {
    case 0:
      timerSelection = timerModeAndBCD | TIMER_SEL0;
      break;
    case 1:
      timerSelection = timerModeAndBCD | TIMER_SEL1;
      break;
    case 2:
      timerSelection = timerModeAndBCD | TIMER_SEL2;
      break;
  }

  uint8_t finalTimerConfig = timerSelection | TIMER_LSB_MSB;

  //calculation of necessary initial value
  uint16_t timerInitialValue = (uint16_t)(TIMER_FREQ / freq);
  // printf("%d\n", timerInitialValue);
  uint8_t freqLSB = 0;
  uint8_t freqMSB = 0;
  util_get_LSB(timerInitialValue, &freqLSB);
  util_get_MSB(timerInitialValue, &freqMSB);

  if (sys_outb(TIMER_CTRL, finalTimerConfig) != OK) {
    printf("Could not load timer config.\n");
    return 1;
  }

  //Sending initial value LSB to timer LSB
  int freqWriteResult;
  freqWriteResult = sys_outb(TIMER_0 + timer, freqLSB);
      
  if (freqWriteResult) {
    printf("Failed to load LSB.\n");
    return 1;
  }

  //Sending initial value MSB to timer MSB
  freqWriteResult = sys_outb(TIMER_0 + timer, freqMSB);

  if (freqWriteResult) {
    printf("Failed to load MSB.\n");
    return 1;
  }

  return 0;
}

int(timer_subscribe_int)(uint8_t *bit_no) {
  int idHook_back = timer_idHook; 

  //Irq set policy
  if (sys_irqsetpolicy(TIMER0_IRQ, IRQ_REENABLE, &timer_idHook) != OK){
    printf("Error setting IQR policy.\n");
    return -1;
  }

  (*bit_no)|=BIT(idHook_back);

  return 0;
}

int(timer_unsubscribe_int)() {
  
  if (sys_irqrmpolicy(&timer_idHook) != OK){
    printf("Error removing IQR policy.\n");
    return -1;
  }

  return 0;
}

void(timer_int_handler)() {
    mouse_interrupt_count++;
}

int(timer_get_conf)(uint8_t timer, uint8_t *st) {

  if (timer > 2 || timer < 0) {
    return 1;
  }

  uint32_t byte = TIMER_RB_CMD | TIMER_RB_COUNT_ | TIMER_RB_SEL(timer);

  int readBackResult = sys_outb(TIMER_CTRL, byte);
  if (readBackResult != OK) {
    printf("Could not send command.\n");
    return 1;
  }

  unsigned int ctrlData;
  readBackResult = sys_inb(TIMER_0 + timer, &ctrlData);    

  if (readBackResult != OK) {
    printf("Could not read timer data.\n");
    return 1;
  }

  *st = (unsigned char) ctrlData;

  return 0;
}

int(timer_display_conf)(uint8_t timer, uint8_t st,
                        enum timer_status_field field) {

  union timer_status_field_val status;

  switch (field) {
    case all:
      status.byte = st;
      break;

    case initial:
      switch (st & TIMER_LSB_MSB) {
        case TIMER_LSB_MSB:
          status.in_mode = MSB_after_LSB;
          break;
        case TIMER_LSB:
          status.in_mode = LSB_only;
          break;
        case TIMER_MSB:
          status.in_mode = MSB_only;
          break;
        default:
          status.in_mode = INVAL_val;
          break;
      }
      break;

    case mode:
      if (!(st & (TIMER_SQR_WAVE | BIT(3)))) {
        status.count_mode = 0;
      }

      switch ((st & (TIMER_SQR_WAVE | BIT(3))) >> 1) {
        case 1:
          status.count_mode = 1;
          break;
        case 2: case 6:
          status.count_mode = 2;
          break;
        case 3: case 7:
          status.count_mode = 3;
          break;
        case 4:
          status.count_mode = 4;
          break;
        case 5:
          status.count_mode = 5;
          break;
      }

      break;

    case base:
      status.bcd = (st & TIMER_BCD) ? true : false;
      break;

    default:
      break;
  }

  return timer_print_config(timer, field, status);
}
