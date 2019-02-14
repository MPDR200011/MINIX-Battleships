#include "rtc.h"
#include "event.h"
#include "lcom/lcf.h"

uint8_t seconds = 0;
uint8_t minutes = 0;
uint8_t hours = 0;

int rtc_idHook = RTC_IRQ;

int rtc_init(uint16_t* rtc_irq) {
  uint8_t regB;
  if (rtc_get_data(REGB_LOC, &regB) != OK) {
    return -1;
  }

  regB |= UPDATE_INT | ALARM_INT;

  if (rtc_send_data(REGB_LOC, regB) != OK) {
    return -1;
  }

  if (sys_irqsetpolicy(RTC_IRQ, IRQ_REENABLE, &rtc_idHook) != OK) {
    printf("failed to subscribe rtc ints\n");
    return -1;
  }

  (*rtc_irq) = (1 << RTC_IRQ);

  return OK;
}

int rtc_exit() {
  if (sys_irqrmpolicy(&rtc_idHook) != OK) {
    printf("failed to unsubsribe rtc ints\n");
    return -1;
  }
  return OK;
}

void rtc_ih() {
  uint8_t regC;
  if (rtc_get_data(REGC_LOC, &regC) != OK) {
    return;
  }
  if (regC & IRQF) {
    if (regC & UPDATE_INT) {
      uint8_t sec;
      uint8_t min;
      uint8_t hou;
      if (rtc_get_data(SEC_LOC, &sec) != OK) {
        return;
      }
      if (rtc_get_data(MIN_LOC, &min) != OK) {
        return;
      }
      if (rtc_get_data(HOUR_LOC, &hou) != OK) {
        return;
      }

      printf("%x %x %x\n", hou, min, sec);

      seconds = sec;
      minutes = min;
      hours = hou;
    }

    if (regC & ALARM_INT) {
      printf("alarm!\n");
      struct rtc_event event = {.type = timeoutAlarm};
      raise_rtc_event(event);
    }
  }
}

void rtc_set_timeout(uint32_t s) {
  uint8_t se = BCDtoINT(seconds);
  uint8_t mi = BCDtoINT(minutes);
  uint8_t ho = BCDtoINT(hours);

  uint8_t sec = (se + s) % 60;
  uint8_t secD = ((se + s) / 60);
  uint8_t min = (secD + mi) % 60;
  uint8_t minD = (secD + mi) / 60;
  uint8_t hou = (minD + ho) % 24;

  sec = INTtoBCD(sec);
  min = INTtoBCD(min);
  hou = INTtoBCD(hou);

  printf("alarm at - %x %x %x\n", hou, min, sec);

  if (rtc_send_data(SEC_AL_LOC, sec) != OK) {
    return;
  }
  if (rtc_send_data(MIN_AL_LOC, min) != OK) {
    return;
  }
  if (rtc_send_data(HOUR_AL_LOC, hou) != OK) {
    return;
  }
}

uint8_t BCDtoINT(uint8_t bcd) {
  uint8_t res = 0;
  res = bcd & 0xF;
  res += (bcd >> 4) * 10;
  return res;
}

uint8_t INTtoBCD(uint8_t i) {
  if (i > 99) {
    return 0x99;
  }
  uint8_t res = 0;
  res = i / 10;
  res <<= 4;
  res += i % 10;
  return res;
}

int rtc_get_data(uint32_t loc, uint8_t* data) {
  if (sys_outb(RTC_ADDR_REG, loc) != OK) {
    printf("failed to write to rtc address register\n");
    return -1;
  }

  uint32_t d;
  if (sys_inb(RTC_DATA_REG, &d) != OK) {
    printf("failed to read from rtc data register\n");
    return -1;
  }
  (*data) = d;
  return OK;
}

int rtc_send_data(uint32_t loc, uint8_t data) {
  if (sys_outb(RTC_ADDR_REG, loc) != OK) {
    printf("failed to write to rtc address register\n");
    return -1;
  }

  if (sys_outb(RTC_DATA_REG, data) != OK) {
    printf("failed to write to rtc data register\n");
    return -1;
  }
  return OK;
}
