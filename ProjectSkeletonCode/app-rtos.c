#include "main.h" 

#include "application.h" 
#include "controller.h"
#include "peripherals.h"
#include "cmsis_os2.h"

#define SAMPLE_TIME 10
#define REF_FLIP_TIME 4000

// Global variables ----------------------------------------------------------*/
int32_t reference, velocity, control;
uint32_t millisec;


// Thread definitions
static const osThreadAttr_t ThreadAttr_ref = {
	.name = "ref",		
  .priority	= osPriorityAboveNormal, //This decleration requires C99 to be selected in the project compiler options
};

static const osThreadAttr_t ThreadAttr_ctrl = {
	.name = "ctrl",		
  .priority	= osPriorityNormal, //This decleration requires C99 to be selected in the project compiler options
};

static const osThreadAttr_t ThreadAttr_main = {
	.name = "main",		
  .priority	= osPriorityBelowNormal, //This decleration requires C99 to be selected in the project compiler options
};



osThreadId_t T_ID1, T_ID2, T_ID3;


// Define timer IDs
osTimerId_t timer_ctrl, timer_ref;

// Prototypes of functions
void static app_main();
void static app_ref();
void static app_ctrl();
void callback_signal_flags(void *argument);


/* Run setup needed for all periodic tasks */
void Application_Setup()
{
  // Reset global variables
  reference = 2000;
  velocity = 0;
  control = 0;
  millisec = 0;

  // Initialise hardware
  Peripheral_GPIO_EnableMotor();

  // Initialize controller
  Controller_Reset();
	
	// Initialize kernel
	osKernelInitialize();
	
	// Create new threads
	T_ID1 = osThreadNew(app_ref, NULL, &ThreadAttr_ref);
	T_ID2 = osThreadNew(app_ctrl, NULL, &ThreadAttr_ctrl);
	T_ID3 = osThreadNew(app_main, NULL, &ThreadAttr_main);
	
	// Start kernel
	osKernelStart();
}

void callback_signal_flags(void *argument){
	// Callback function that sets thread flags based on input
	int32_t val = (int)(uintptr_t)argument;
	if (val == 1){
		osThreadFlagsSet(T_ID1, 0x05); // Set flag 101 to thread T_ID1
		
	}
	else if (val == 0){
		osThreadFlagsSet(T_ID2, 0x03); // Set flag 011 to thread T_ID2
		
	}
	else {
		//Why are you here?
	}
}


void app_ctrl () {
		for(;;) {
		
			osThreadFlagsWait(0x03, osFlagsWaitAll, osWaitForever); // Wait until all flags (011) are set
			
			// Get time
			millisec = Main_GetTickMillisec();

			// Calculate motor velocity
			velocity = Peripheral_Encoder_CalculateVelocity(millisec);

			// Calculate control signal
			control = Controller_PIController(&reference, &velocity, &millisec);

			// Apply control signal to motor
			Peripheral_PWM_ActuateMotor(control);
		
		
			//osDelay(SAMPLE_TIME);
			// The problem with osDelay is that it only counts the ticks, so if you say osDelay(10) right before a new tick, the actual delay might be closer to 9 ticks
	}

}

void app_ref(){
	for(;;) {
		
		osThreadFlagsWait(0x05, osFlagsWaitAll, osWaitForever); // Wait until all flags (101) are set
		
		reference = - reference;
		
		
		//osDelay(REF_FLIP_TIME);
	}
}

void static app_main() {
	// Define timer with callback function, set to periodic, define argument to callback
	timer_ctrl = osTimerNew(callback_signal_flags, osTimerPeriodic, (void *)(uintptr_t)0, NULL);
	
	timer_ref = osTimerNew(callback_signal_flags, osTimerPeriodic, (void *)(uintptr_t)1, NULL);
	
	// Start timers, set period time
	osTimerStart(timer_ctrl, SAMPLE_TIME);
	osTimerStart(timer_ref, REF_FLIP_TIME);
	
  for (;;)
  {
		Application_Loop();
  }

}

/* Define what to do in the infinite loop */
void Application_Loop()
{
 // Do nothing
 osThreadFlagsWait(0x01, osFlagsWaitAll, osWaitForever); // Never triggered as we never set a flag to thread T_ID3
}
