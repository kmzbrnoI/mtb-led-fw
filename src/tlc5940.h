#ifndef _TLC5940_H_
#define _TLC5940_H_

/* Setting state of outputs.
 */

#include <stdint.h>
#include <stddef.h>

#include "io.h"

extern uint32_t tlc_outputs_state; // read-only variable
extern uint32_t tlc_outputs_connected; // read-only variable
extern bool tlc_sample_request;

void tlc_init(uint32_t out_state);
void tlc_out_set(uint32_t state);
void tlc_sample_status(void);

#endif
