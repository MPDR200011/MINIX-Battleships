#include "keyboard.h"
#include "event.h"

uint32_t inb_count;
uint8_t kbd_data;
bool ihResult = true;
int kbd_idHook = 1;

int kbd_get_data(uint8_t port,uint8_t *status) {

    uint32_t data;

    if (sys_inb(port, &data) != OK) {
      return -1;
    }

    (*status) = (uint8_t) data;

    return 0;

}

int kbd_set_data(uint8_t port,uint8_t status) {

    uint32_t data = (uint32_t) status;

    if (sys_outb(port, data) != OK) {
      return -1;
    }

    return 0;

}

int kbc_send_command(uint8_t cmd){
  uint8_t status;

  while (1){
    
    if (kbd_get_data(STATUS_REG,&status)!= OK)
    {
      continue;
    }

    if ((status & IBF) == 0){
        kbd_set_data(KBC_CMD_PORT,cmd);
        return 0;
    }

  }
  return -1;
}

int kbd_subscribe_int(uint8_t *bit_no) {
  int idHook_back = kbd_idHook;

  //IRQ set policy
  if (sys_irqsetpolicy(KBD_IRQ, (IRQ_REENABLE|IRQ_EXCLUSIVE), &kbd_idHook) != OK) {
    printf("Error setting irq_policy.");
    return -1;
  }

  (*bit_no) |= BIT(idHook_back);
  return 0;
}

int kbd_unsubscribe_int() {
  if (sys_irqrmpolicy(&kbd_idHook) != OK) {
    printf("Error removing irq_policy.");
    return -1;
  }
  return 0;
}

void kbd_process_byte() {
  static bool twoByteFlag = false;
  static struct kbd_event event;
  if (kbd_data == TWO_BYTE_SCAN) {
    event.codes[0] = kbd_data;
    twoByteFlag = true;
	event.twoBytes = true;
  } else if (twoByteFlag) {
    event.codes[1] = kbd_data;
    twoByteFlag = false;
	event.pressed = kbd_data & MAKE_CODE_SPEC ? false : true;
	raise_keyboard_event(event);
  } else {
    event.codes[1] = kbd_data;
	event.twoBytes = false;
	event.pressed = kbd_data & MAKE_CODE_SPEC ? false : true;
	raise_keyboard_event(event);
  }
}

void (kbc_ih)(void) {
  uint8_t status;
  
  if (kbd_get_data(STATUS_REG, &status) != OK) {
    printf("IH - Failed to read status.\n");
    ihResult = false;
    return;
  }

  if (status & OBF) {
    if (kbd_get_data(KBC_OUT_BUFFER, &kbd_data) != OK) {
      printf("IH - Failed to read out_buff.\n");
      ihResult = false;
      return;
    }

    if ((status & (PRT | TIMEOUT)) == OK) {
      ihResult = true;
      return;
    } else {
      printf("IH - Communication error.\n");
      ihResult = false;
      return;
    }
  }
  
  ihResult = false;
  return;
}

int kbc_enable_inter(){
  uint8_t cmd_byte;
  if(kbc_send_command(KBC_READ_CMD) != OK){ 
    return -1;
  }

  if (kbd_get_data(KBC_CMDARG_PORT, &cmd_byte)!= OK){
    return -1;
  }

  cmd_byte = KBC_CMDB_ENABLE_KEYBOARD;
  // cmd_byte |= KBC_CMDB_INT | KBC_CMDB_INT2;

  if(kbc_send_command(KBC_WRITE_CMD) != OK){
    return -1;
  }

  if (kbd_set_data(KBC_CMDARG_PORT, cmd_byte)!= OK){
    return -1;
  }
  return 0;
}
