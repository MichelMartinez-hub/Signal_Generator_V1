#ifndef PARAMETER_MANAGER_H
#define PARAMETER_MANAGER_H

#include <stdbool.h>
#include "app/controls.h"

/* Application policy for three potentiometers and the quadrature encoder.
   Call once after Generator_Init(), then call Update() periodically from
   foreground code (initial proposal: every 50 ms, NOT from an ISR). */
void ParameterManager_Init(void);

/* Returns false on invalid argument or when the generator rejects an update.
   Returns true when a sample was handled, including when nothing changed. */
bool ParameterManager_Update(const ControlsRaw_t *raw);

#endif /* PARAMETER_MANAGER_H */
