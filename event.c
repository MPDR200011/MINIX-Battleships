#include "event.h"
#include "lcom/lcf.h"

// enum mouse_ev_t { LB_PRESSED,
//                   LB_RELEASED,
//                   RB_PRESSED,
//                   RB_RELEASED,
//                   BUTTON_EV, middle mouse
//                   MOUSE_MOV };

static void (*mouse_b_handler)(struct mouse_ev) = NULL;
static void (*kbd_handler)(struct kbd_event) = NULL;
static void (*serial_handler)(struct serial_event) = NULL;
static void (*rtc_handler)(struct rtc_event) = NULL;

void reset_handlers() {
  mouse_b_handler = NULL;
  kbd_handler = NULL;
  serial_handler = NULL;
  rtc_handler = NULL;
}

void raise_mouse_btn_event(struct mouse_ev event) {
  if (mouse_b_handler == NULL) {
    return;
  }
  (*mouse_b_handler)(event);
}

void set_mouse_btn_handler(void (*handler)(struct mouse_ev event)) {
  mouse_b_handler = handler;
}

void raise_keyboard_event(struct kbd_event event) {
  if (kbd_handler == NULL) {
    return;
  }
  (*kbd_handler)(event);
}

void set_keyboard_handler(void (*handler)(struct kbd_event event)) {
  kbd_handler = handler;
}

void set_serial_handler(void (*handler)(struct serial_event)) {
  serial_handler = handler;
}

void raise_serial_event(struct serial_event event) {
  if (serial_handler == NULL) {
    return;
  }
  (*serial_handler)(event);
}

void raise_rtc_event(struct rtc_event event) {
  if (rtc_handler == NULL) {
    return;
  }
  (*rtc_handler)(event);
}

void set_rtc_handler(void (*handler)(struct rtc_event event)) {
  rtc_handler = handler;
}

