#ifndef _OUTPUTS_H_
#define _OUTPUTS_H_

/* Setting state of outputs.
 */

#include <stdint.h>
#include <stddef.h>

#include "io.h"

extern uint32_t outputs_state;

void out_init(uint32_t state);
void out_set(uint32_t state);

#endif
