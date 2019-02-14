#include "serial.h"
#include "event.h"
#include "lcom/lcf.h"
#include "stdint.h"
#include "stdlib.h"

uint8_t* transmitterBuffer;
int transBufferIndex = 0;
uint8_t* receiveBuffer;
int receiveBufferIndex = 0;

int serial_hookID = COM_IRQ;
bool THR_ready = true;

int serial_init(uint8_t* serial_irq) {
  transmitterBuffer = malloc(sizeof(uint8_t) * 16);
  receiveBuffer = malloc(sizeof(uint8_t) * 16);
  // SET BITRATE
  uint32_t lcr;
  if (sys_inb(LCR_PORT, &lcr) != OK) {
    printf("failed to read lcr\n");
    return -1;
  }

  lcr |= LCR_DLAB;
  if (sys_outb(LCR_PORT, lcr) != OK) {
    printf("failed to enable DL access\n");
    return -1;
  }

  uint32_t divisor = 115200 / COM_BITRATE;
  printf("DLL: %x\n", (uint8_t)divisor);
  if (sys_outb(COM_BASE, divisor) != OK) {
    printf("failed to write DLL\n");
    return -1;
  }

  divisor >>= 8;
  printf("DLM: %x\n", (uint8_t)divisor);
  if (sys_outb(COM_BASE + 1, (uint8_t)divisor) != OK) {
    printf("failed to write lcr DLM\n");
    return -1;
  }

  lcr = lcr ^ LCR_DLAB;
  if (sys_outb(LCR_PORT, lcr) != OK) {
    printf("failed to disable DL access\n");
    return -1;
  }

  // CONFIG LCR
  if (sys_inb(LCR_PORT, &lcr) != OK) {
    printf("failed to read lcr\n");
    return -1;
  }

  lcr = (lcr & LCR_CLEAR_CONFIG) | PROG_SERIAL_CONF;

  if (sys_outb(LCR_PORT, lcr) != OK) {
    printf("failed to write lcr config\n");
    return -1;
  }

  // CONFIG IER
  uint32_t ier;
  if (sys_inb(IER_PORT, &ier) != OK) {
    printf("failed to read ier\n");
    return -1;
  }

  ier |= DATA_RECEIVE_INT | THR_EMPTY_INT;

  if (sys_outb(IER_PORT, ier) != OK) {
    printf("failed to send ier config\n");
    return -1;
  }

  // CONFIG FCR
  uint32_t fcr = 0;
  fcr |= ENABLE_BOTH_FIFOS;
  if (sys_outb(FCR_PORT, fcr) != OK) {
    printf("failed to enable FIFOs\n");
    return -1;
  }

  fcr |= FIFO_TRIG_LEVEL_4;
  if (sys_outb(FCR_PORT, fcr) != OK) {
    printf("failed to send fcr config\n");
    return -1;
  }

  // ENABLE INTERRUPTS
  uint8_t hookPass = serial_hookID;
  if (sys_irqsetpolicy(COM_IRQ, (IRQ_REENABLE | IRQ_EXCLUSIVE),
                       &serial_hookID) != OK) {
    printf("failed to set serial port interrupts\n");
    return -1;
  }

  (*serial_irq) = 1 << hookPass;

  return OK;
}

int serial_exit() {
  if (sys_irqrmpolicy(&serial_hookID) != OK) {
    printf("failed to unsubscribe serial interrupts\n");
    return -1;
  }
  return OK;
}

void serial_ih() {
  uint32_t iir;
  if (sys_inb(IIR_PORT, &iir) != OK) {
    printf("failed to read iir\n");
    return;
  }

  printf("%d\n", iir & IIR_INT_DATA);
  if (!(iir & IIR_NO_INT_PENDING)) {
    switch (iir & IIR_INT_DATA) {
      case IIR_CHAR_TIMEOUT:
      case IIR_REC_DATA_AV: {
        printf("data\n");
        serial_receiveData();
        break;
      }
      case IIR_THR_EMPTY: {
        THR_ready = true;
        printf("THR ready\n");
        serial_sendData();
        break;
      }
      default:
        break;
    }
  }
}

void serial_receiveData() {
  uint32_t lsr;
  sys_inb(LSR_PORT, &lsr);
  while (lsr & LSR_REC_DATA) {
    uint32_t data;
    sys_inb(REC_BUFFER, &data);
    receiveBuffer[receiveBufferIndex] = (uint8_t)data;
    receiveBufferIndex++;
    sys_inb(LSR_PORT, &lsr);
  }
}

void serial_sendData() {
  if (!THR_ready) {
    return;
  }
  for (int i = 0; i < transBufferIndex; i++) {
    if (sys_outb(TRANS_BUFFER, transmitterBuffer[i]) != OK) {
      printf("failed sending %x\n", transmitterBuffer[i]);
    } else {
      printf("sending %x\n", transmitterBuffer[i]);
    }
    transmitterBuffer[i] = 0;
  }
  THR_ready = true;
  transBufferIndex = 0;
}

void serial_processData() {
  enum dataState { reg, shotX, shotY };
  static enum dataState currState = reg;
  static struct serial_event ev = {.MSG = READY_MSG, .X = 0, .Y = 0};

  for (int i = 0; i < receiveBufferIndex; i++) {
    switch (currState) {
      case reg: {
        switch (receiveBuffer[i]) {
          case SHOT_HEADER:
            ev.MSG = SHOT_HEADER;
            currState = shotX;
            break;
          case READY_MSG:
          case VICT_MSG:
          case HIT_MSG:
          case MISS_MSG:
          case LATE_MSG:
            ev.MSG = receiveBuffer[i];
            raise_serial_event(ev);
            break;
          default:
            printf("invalid byte\n");
            break;
        }
        break;
      }
      case shotX: {
        ev.X = receiveBuffer[i];
        currState = shotY;
        break;
      }
      case shotY: {
        ev.Y = receiveBuffer[i];
        raise_serial_event(ev);
        currState = reg;
        break;
      }
    }
  }
  printf("\n");
  receiveBufferIndex = 0;
}

int serial_addByteToSend(uint8_t byte) {
  if (transBufferIndex >= 16) {
    printf("send queue full\n");
    serial_sendData();
  }

  transmitterBuffer[transBufferIndex] = byte;
  transBufferIndex++;
  return OK;
}

