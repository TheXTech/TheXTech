#ifndef EFFECT_H
#define EFFECT_H

#include "location.h"

// Public Sub UpdateEffects() 'Updates the effects
// Updates the effects
void UpdateEffects();
// Public Sub NewEffect(A As Integer, Location As Location, Optional Direction As Single = 1, Optional NewNpc As Integer = 0, Optional Shadow As Boolean = False)  'Create an effect
// Create an effect
void NewEffect(int A, Location_t Location_t, float Direction = 1, int NewNpc = 0, bool Shadow = false);
// Public Sub KillEffect(A As Integer) 'Remove the effect
// Remove the effect
void KillEffect(int A);


#endif // EFFECT_H
