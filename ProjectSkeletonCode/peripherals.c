#include "peripherals.h"

#define RESOLUTION 2048		//

int16_t encoder;			// Global variable, can be used for debugging purposes
int16_t difference_debug;	
int32_t scaled_vel = 0;
int32_t vel_debug = 0;


// #define TIM1_ADRESS (*((volatile unsigned int*) 0x40012C24U))

/* Enable both half-bridges to drive the motor */
void Peripheral_GPIO_EnableMotor(void)
{
	// Write a 1 to the bits controlling I/O
	GPIOA->BSRR = GPIO_BSRR_BS5;
	GPIOA->BSRR = GPIO_BSRR_BS6;
	return;
}

/* Disable both half-bridges to stop the motor */
void Peripheral_GPIO_DisableMotor(void)
{
	// Write a 0 to the bits controlling I/O
	GPIOA->BSRR = GPIO_BSRR_BR5;
	GPIOA->BSRR = GPIO_BSRR_BR6;
	return;
}

/* Drive the motor in both directions */
void Peripheral_PWM_ActuateMotor(int32_t vel)
{
	// Vel is actually a control signal, not a velocity, it has max value 1073741823
	// Max value PWM timer can take is 2047, min value is -2048 so must be adjusted to this, so we shift 19 steps right
	// We are essentially dividing the input signal into 2048 steps in each direction
	
	scaled_vel = vel >> 19;

	// vel_debug = scaled_vel;
	if (vel > 0) {
		TIM3->CCR1 = (uint16_t)scaled_vel;
		TIM3->CCR2 = 0;
	}
	else {
		TIM3->CCR1 = 0;
		TIM3->CCR2 = (uint16_t)-scaled_vel;
	}
	
	return;
}

/* Read the encoder value and calculate the current velocity in RPM */
int32_t Peripheral_Encoder_CalculateVelocity(uint32_t ms)
{	
	static int16_t lastcycle_encoder = 0;
	static int32_t lastcycle_ms = 0;
	
	// Base address: 0x40012C00
	// Address offset: 0x24 (we want bits 0-15)
	
	// Read encoder
	encoder = (int16_t)TIM1->CNT;
	
	int16_t encoder_difference = encoder - lastcycle_encoder;
	difference_debug = encoder_difference;
	int32_t ms_difference = (int32_t)ms - lastcycle_ms;
	
	// Calculate RPM, 60000 because time is in ms, 2048 steps is one revolution
	int32_t rpm = ((encoder_difference) * 60000) / (2048 * (10));
	
	// Save states
	lastcycle_encoder = encoder;
	lastcycle_ms = (int32_t)ms;
	
	// Flip value to get correct positive direction
	vel_debug = -rpm;
	return (-rpm);
}
