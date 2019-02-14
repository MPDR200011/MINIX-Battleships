#include "mouse.h"
#include <lcom/lcf.h>
#include <math.h>
#include <sys/stdint.h>
#include "event.h"
#include "graphics.h"
#include "i8042.h"
#include "sprites/cursor.xpm"
#include "stdbool.h"

static int mouseX = 0;
static int mouseY = 0;

static Sprite* mouse_cursor;

uint8_t mouse_data;
struct packet mouse_packet;

bool xSign = false;
bool ySign = false;
bool mouse_ih_success;
int mouseId_hook = MOUSE_IRQ;

int mouse_load_sprite_xpm() {
  mouse_cursor = malloc(sizeof(Sprite));
  if (xpm_load(cursor, XPM_8_8_8, &mouse_cursor->info) == NULL) {
    return 1;
  }
  return 0;
}

void mouse_reset() {
  mouseX = getHres() / 2;
  mouseY = getVres() / 2;
}

int mouse_subscribe_int(uint32_t* bit_no) {
  int idHook_back = mouseId_hook;

  // IRQ set policy
  if (sys_irqsetpolicy(MOUSE_IRQ, (IRQ_REENABLE | IRQ_EXCLUSIVE),
                       &mouseId_hook) != OK) {
    printf("Error setting irq_policy.");
    return -1;
  }

  (*bit_no) = BIT(idHook_back);
  return 0;
}

int mouse_unsubscribe_int() {
  if (sys_irqrmpolicy(&mouseId_hook) != OK) {
    printf("Error removing irq_policy.");
    return -1;
  }
  mouseId_hook = MOUSE_IRQ;
  return 0;
}

void(mouse_ih)(void) {
  uint32_t status;
  if (sys_inb(STATUS_REG, &status) != OK) {
    mouse_ih_success = false;
    printf("IH - Failed to read status.\n");
    return;
  }

  uint32_t tempData;
  if ((uint8_t)status & OBF) {
    if (sys_inb(KBC_OUT_BUFFER, &tempData) != OK) {
      mouse_ih_success = false;
      printf("IH - Failed to read out_buff.\n");
      return;
    }

    if ((status & (PRT | TIMEOUT)) == OK) {
      mouse_data = (uint8_t)tempData;
      mouse_ih_success = true;
      return;
    } else {
      mouse_ih_success = false;
      printf("IH - Communication error.\n");
      return;
    }
  }
}

void mouse_generatePacket() {
  enum state { CONFIG, XDELTA, YDELTA };
  static enum state currState = CONFIG;
  switch (currState) {
    case CONFIG:
      mouse_packet.bytes[0] = mouse_data;
      mouse_packet.rb = mouse_data & MOUSE_RB;
      mouse_packet.mb = mouse_data & MOUSE_MB;
      mouse_packet.lb = mouse_data & MOUSE_LB;
      mouse_packet.x_ov = mouse_data & MOUSE_XOVF;
      mouse_packet.y_ov = mouse_data & MOUSE_YOVF;
      xSign = mouse_data & MOUSE_XSIGN;
      ySign = mouse_data & MOUSE_YSIGN;
      currState = XDELTA;
      break;
    case XDELTA:
      mouse_packet.bytes[1] = mouse_data;
      mouse_packet.delta_x = xSign ? (int16_t)mouse_data - 256 : mouse_data;
      currState = YDELTA;
      break;
    case YDELTA:
      mouse_packet.bytes[2] = mouse_data;
      mouse_packet.delta_y = ySign ? (int16_t)mouse_data - 256 : mouse_data;
      currState = CONFIG;
      struct mouse_ev event = mouse_make_event(&mouse_packet);
      if (event.type == MOUSE_MOV) {
        mouseX += event.delta_x;
        mouseY -= event.delta_y;
      } else {
        raise_mouse_btn_event(event);
      }
      break;
    default:
      break;
  }
}

bool canSendCommand() {
  uint32_t status;
  if (sys_inb(STATUS_REG, &status) != OK) {
    printf("Failed to retrieve status.");
    return false;
  }

  return ((uint8_t)status & IBF) ? false : true;
}

int mouse_enable() {
  while (1) {
    if (!canSendCommand()) {
      continue;
    }

    if (sys_outb(KBC_CMD_PORT, KBC_ENABLE_MOUSE) != OK) {
      printf("Could not enable mouse \n");
      return -1;
    }

    return 0;
  }
}

int mouse_disable() {
  while (1) {
    if (!canSendCommand()) {
      continue;
    }

    if (sys_outb(KBC_CMD_PORT, KBC_DISABLE_MOUSE) != OK) {
      printf("Could not enable mouse \n");
      return -1;
    }

    return 0;
  }
}

int mouse_enable_data_rep() {
  uint32_t ackByte = 0;
  do {
    if (!canSendCommand())
      continue;

    if (sys_outb(KBC_CMD_PORT, KBC_MOUSE_WRITE_BYTE) != OK) {
      printf("Failed to send command.\n");
      return -1;
    }

    if (!canSendCommand())
      continue;

    if (sys_outb(KBC_CMDARG_PORT, MOUSE_ENABLE_DATA_REP) != OK) {
      printf("Failed to send byte.\n");
      return -1;
    }

    tickdelay(micros_to_ticks(KBC_DELAY));
    if (sys_inb(KBC_OUT_BUFFER, &ackByte) != OK) {
      printf("Failed to retrieve ack byte.\n");
      return -1;
    }

  } while ((uint8_t)ackByte != MOUSE_BYTE_ACK);

  return OK;
}

int mouse_disable_data_rep() {
  uint32_t ackByte = 0;
  do {
    if (!canSendCommand())
      continue;

    if (sys_outb(KBC_CMD_PORT, KBC_MOUSE_WRITE_BYTE) != OK) {
      printf("Failed to send command.\n");
      return -1;
    }

    if (!canSendCommand())
      continue;

    if (sys_outb(KBC_CMDARG_PORT, MOUSE_DISABLE_DATA_REP) != OK) {
      printf("Failed to send byte.\n");
      return -1;
    }

    tickdelay(micros_to_ticks(KBC_DELAY));
    if (sys_inb(KBC_OUT_BUFFER, &ackByte) != OK) {
      printf("Failed to retrieve ack byte.\n");
      return -1;
    }

  } while ((uint8_t)ackByte != MOUSE_BYTE_ACK);
  return OK;
}

int mouse_set_stream_mode() {
  uint32_t ackByte = 0;
  do {
    if (!canSendCommand())
      continue;

    if (sys_outb(KBC_CMD_PORT, KBC_MOUSE_WRITE_BYTE) != OK) {
      printf("Failed to send command.\n");
      return -1;
    }

    if (!canSendCommand())
      continue;

    if (sys_outb(KBC_CMDARG_PORT, MOUSE_SET_STREAM_MODE) != OK) {
      printf("Failed to send byte.\n");
      return -1;
    }

    tickdelay(micros_to_ticks(KBC_DELAY));
    if (sys_inb(KBC_OUT_BUFFER, &ackByte) != OK) {
      printf("Failed to retrieve ack byte.\n");
      return -1;
    }

  } while ((uint8_t)ackByte != MOUSE_BYTE_ACK);
  return OK;
}

int mouse_set_remote_mode() {
  uint32_t ackByte = 0;
  do {
    if (!canSendCommand())
      continue;

    if (sys_outb(KBC_CMD_PORT, KBC_MOUSE_WRITE_BYTE) != OK) {
      printf("Failed to send command.\n");
      return -1;
    }

    if (!canSendCommand())
      continue;

    if (sys_outb(KBC_CMDARG_PORT, MOUSE_SET_REMOTE_MODE) != OK) {
      printf("Failed to send byte.\n");
      return -1;
    }

    tickdelay(micros_to_ticks(KBC_DELAY));
    if (sys_inb(KBC_OUT_BUFFER, &ackByte) != OK) {
      printf("Failed to retrieve ack byte.\n");
      return -1;
    }
  } while ((uint8_t)ackByte != MOUSE_BYTE_ACK);

  return OK;
}

int kbc_reset_default_byte() {
  uint8_t defaultByte = minix_get_dflt_kbc_cmd_byte();

  while (1) {
    if (!canSendCommand())
      continue;

    if (sys_outb(KBC_CMD_PORT, KBC_WRITE_CMD) != OK) {
      return -1;
    }

    if (!canSendCommand())
      continue;

    if (sys_outb(KBC_CMDARG_PORT, defaultByte) != OK) {
      return -1;
    }

    break;
  }
  return 0;
}

struct mouse_ev mouse_make_event(struct packet* pp) {
  static struct packet bfr = {.rb = false,
                              .lb = false,
                              .mb = false,
                              .delta_x = 0,
                              .delta_y = 0,
                              .x_ov = false,
                              .y_ov = false};
  static struct mouse_ev tmp = {.type = BUTTON_EV, .delta_x = 0, .delta_y = 0};

  // Left Button
  if (bfr.lb != pp->lb) {
    if (pp->lb) {
      tmp.type = LB_PRESSED;
      tmp.delta_x = bfr.delta_x;
      tmp.delta_y = bfr.delta_y;
      bfr = (*pp);
      return tmp;
    } else {
      tmp.type = LB_RELEASED;
      tmp.delta_x = bfr.delta_x;
      tmp.delta_y = bfr.delta_y;
      bfr = (*pp);
      return tmp;
    }
  }

  // Right Button
  if (bfr.rb != pp->rb) {
    if (pp->rb) {
      tmp.type = RB_PRESSED;
      tmp.delta_x = bfr.delta_x;
      tmp.delta_y = bfr.delta_y;
      bfr = (*pp);
      return tmp;
    } else {
      tmp.type = RB_RELEASED;
      tmp.delta_x = bfr.delta_x;
      tmp.delta_y = bfr.delta_y;
      bfr = (*pp);
      return tmp;
    }
  }

  // Middle button
  if (bfr.mb != pp->mb) {
    tmp.type = BUTTON_EV;
    tmp.delta_x = bfr.delta_x;
    tmp.delta_y = bfr.delta_y;
    bfr = (*pp);
    return tmp;
  }

  // Mouse Mov
  if (bfr.delta_x != pp->delta_x) {
    tmp.type = MOUSE_MOV;
    tmp.delta_x = pp->delta_x;
    if (bfr.delta_y != pp->delta_y) {
      tmp.delta_y = pp->delta_y;
      return tmp;
    } else {
      tmp.delta_y = bfr.delta_y;
      bfr = (*pp);
      return tmp;
    }
  }

  if (bfr.delta_y != pp->delta_y) {
    tmp.type = MOUSE_MOV;
    tmp.delta_y = pp->delta_y;
    if (bfr.delta_x != pp->delta_x) {
      tmp.delta_x = pp->delta_x;
      return tmp;
    } else {
      tmp.delta_x = bfr.delta_x;
      bfr = (*pp);
      return tmp;
    }
  }
  return tmp;
}

void mouse_draw_mouse() {
  // vg_draw_rectangle(mouseX, mouseY, 10, 10, 0xff00b2);
  draw_sprite(mouse_cursor, mouseX - mouse_cursor->info.width / 2,
              mouseY - mouse_cursor->info.height / 2);
}

int mouse_getX() {
  return mouseX;
}

int mouse_getY() {
  return mouseY;
}
