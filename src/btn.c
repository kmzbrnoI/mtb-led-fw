#include <stddef.h>
#include "btn.h"
#include "io.h"

volatile bool btn_pressed = false;

#define DEBOUNCE_THRESHOLD 20 // 10 ms
volatile uint8_t _btn_debounce_counter = 0;

void button_debounce_update(void) {
	bool state = io_button();
	if (state & 0x01) {
		if (_btn_debounce_counter > 0) {
			_btn_debounce_counter--;
			if ((_btn_debounce_counter == 0) && (btn_pressed)) {
				btn_pressed = false;
				btn_on_depressed();
			}
		}
	} else {
		if (_btn_debounce_counter < DEBOUNCE_THRESHOLD) {
			_btn_debounce_counter++;
			if ((_btn_debounce_counter == DEBOUNCE_THRESHOLD) && (!btn_pressed)) {
				btn_pressed = true;
				btn_on_pressed();
			}
		}
	}
}
