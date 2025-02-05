#include "controller.h"
#define control_signal_max 1073741823

// PI controller parameters
int32_t Kp = 30000;
int32_t Ki = 500000;

// Variables
uint32_t prev_ms = 0;
int32_t delta_ms = 0;
int32_t err = 0;
int32_t I = 0;
int32_t control_value = 0; 
int8_t reset = 0;
int32_t P = 0;

int32_t Controller_PIController(const int32_t* ref, const int32_t* meas, const uint32_t* ms)
{
	delta_ms = *ms - prev_ms;
	err = *ref - *meas;
	
	I += ((err * delta_ms) * (Ki / 1000));
	
	// Saturate to 31 bits signed
	I = __ssat(I, 31);
	
	P = Kp * err;
	
	// Saturate to 31 bits signed
	P = __ssat(P, 31); 
	
	// Due to saturation previously, control_value will never exceed 32 bits signed, as two 31 bit numbers added can never exceed 32 bits
	control_value = (P + I);
	
	// Saturate control signal
	if (control_value > control_signal_max)
	{
		control_value = control_signal_max;
	}	

	else if (control_value < -control_signal_max)
	{
		control_value = -control_signal_max;
	}

	// Save last state
	prev_ms = *ms;
	
	// Reset controller, set first control signal output to zero
	if (reset)
	{
		I = 0;
		control_value = 0;
		reset = 0; // Reset flag
	}
	
	// Max value 1073741823
	return control_value;
}

void Controller_Reset(void)
{
	reset = 1; // Set flag
  return;
}
