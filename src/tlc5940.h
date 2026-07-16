#ifndef _TLC5940_H_
#define _TLC5940_H_

/* Communication with LED drivers TLC5940.
 * tlc_update_request is set peridically (usually each 100 ms) to read TLC's
 * Status Register to detect LED-open-port continuously.
 */

#include <stdint.h>
#include <stddef.h>

#include "io.h"

extern uint32_t tlc_outputs_want_state; // read-only variable
extern uint32_t tlc_outputs_connected; // read-only variable
extern volatile bool tlc_update_request;

void tlc_init(uint32_t out_state);
void tlc_out_set(uint32_t state);
void tlc_update(void); // call as-fast-as-possible

#endif
