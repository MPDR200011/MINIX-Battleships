// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>

// Any header files included below this line should have been created by you
#include "game.h"
#include "graphics.h"
#include "keyboard.h"
#include "mouse.h"
#include "rtc.h"
#include "serial.h"
#include "stdlib.h"

int main(int argc, char* argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/proj/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/proj/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int(proj_main_loop)(int argc, char* argv[]) {
  printf("%d\n", argc);
  if (argc) {
    free(argv);
  }
  bool init_success = false;

  if (vg_init(0x115) == NULL) {  // 800x600
    printf("Couldn't initialize graphics.\n");
    return -1;
  }

  uint8_t timer_irq;
  if (timer_subscribe_int(&timer_irq) != OK) {
    printf("failed to subscribe timer ints\n");
    return -1;
  }

  if (mouse_enable_data_rep() != OK) {
    printf("failed to enable mouse data reporting\n");
    return -1;
  }

  uint32_t mouse_irq;
  if (mouse_subscribe_int(&mouse_irq) != OK) {
    printf("failed to subscribe mouse ints\n");
    return -1;
  }

  uint8_t kbd_irq;
  if (kbd_subscribe_int(&kbd_irq) != OK) {
    printf("Couldn't subscribe kbd ints\n");
    return -1;
  }

  uint8_t serial_irq;
  if (serial_init(&serial_irq) != OK) {
    return -1;
  }

  uint16_t rtc_irq;
  if (rtc_init(&rtc_irq) != OK) {
    return -1;
  }
  rtc_ih();

  init_success = true;

  game_init();

  if (init_success) {
    int ipc_status;
    message msg;
    int r;
    while (isGameRunning()) {
      if ((r = driver_receive(ANY, &msg, &ipc_status)) != OK) {
        printf("driver receive failed with: %d", r);
        continue;
      }
      if (is_ipc_notify(ipc_status)) {
        switch (_ENDPOINT_P(msg.m_source)) {
          case HARDWARE: {
            if (msg.m_notify.interrupts & rtc_irq) {
              rtc_ih();
            }
            if (msg.m_notify.interrupts & kbd_irq) {
              kbc_ih();
              kbd_process_byte();
            }

            if (msg.m_notify.interrupts & mouse_irq) {
              mouse_ih();
              mouse_generatePacket();
            }

            if (msg.m_notify.interrupts & serial_irq) {
              serial_ih();
              serial_processData();
            }

            if (msg.m_notify.interrupts & timer_irq) {
              // Render frames
              vg_clear_canvas();
              game_render_current_state();
              game_advance_tick();
              vg_flip_buffer();

              // Send serial buffer
              serial_sendData();
            }
          }
        }
      }
    }
  }

  if (rtc_exit() != OK) {
    return -1;
  }

  if (serial_exit() != OK) {
    printf("failed to unsubscribe serial ints\n");
    return -1;
  }

  if (kbd_unsubscribe_int() != OK) {
    printf("Failed to unsubscribe kbd ints\n");
    return -1;
  }

  if (mouse_unsubscribe_int() != OK) {
    printf("failed to unsubscribe mouse ints\n");
    return -1;
  }

  if (mouse_disable_data_rep() != OK) {
    printf("failed to disable mouse data reporting\n");
    return -1;
  }

  if (timer_unsubscribe_int() != OK) {
    printf("failed to unsubscribe timer ints\n");
    return -1;
  }

  if (vg_exit() != OK) {
    printf("failed to return to text mode\n");
    return -1;
  }

  printf("finished\n");

  return 0;
}

