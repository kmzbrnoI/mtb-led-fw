#ifndef _BTN_H_
#define _BTN_H_

/* Button input debouncing.
 */

#include <stdint.h>
#include <stdbool.h>

extern volatile bool btn_pressed;
void btn_on_pressed(void);
void btn_on_depressed(void);

// This function should be called each 100 us
void button_debounce_update(void);

#endif
