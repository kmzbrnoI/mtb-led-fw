#ifndef _TLC5940_H_
#define _TLC5940_H_

/* Setting state of outputs.
 */

#include <stdint.h>
#include <stddef.h>

#include "io.h"

extern uint32_t tlc_outputs_state;

void tlc_init(uint32_t out_state);
void tlc_out_set(uint32_t state);

#endif
