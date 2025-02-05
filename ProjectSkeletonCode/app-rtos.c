#include "main.h" 

#include "application.h" 
#include "controller.h"
#include "peripherals.h"
#include "cmsis_os2.h"

#define SAMPLE_TIME 10
#define REF_FLIP_TIME 4000

/* Global variables ----------------------------------------------------------*/
int32_t reference, velocity, control;
uint32_t millisec;

static const osThreadAttr_t ThreadAttr_ref = {
	.name = "ref",		
  .priority	= osPriorityAboveNormal, //This decleration requires C99 to be selected in the project compiler options
};

static const osThreadAttr_t ThreadAttr_ctrl = {
	.name = "ctrl",		
  .priority	= osPriorityNormal, //This decleration requires C99 to be selected in the project compiler options
};

/* Functions -----------------------------------------------------------------*/


// Define threads
osThreadId_t T_ID1, T_ID2;

void static app_main();


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
	app_main();
}

void app_ctrl () {
		for(;;) {
		
			
			// Get time
			millisec = Main_GetTickMillisec();

			// Calculate motor velocity
			velocity = Peripheral_Encoder_CalculateVelocity(millisec);

			// Calculate control signal
			control = Controller_PIController(&reference, &velocity, &millisec);

			// Apply control signal to motor
			Peripheral_PWM_ActuateMotor(control);
		
		
			osDelay(SAMPLE_TIME);
	}

}

void app_ref(){
	for(;;) {
		
		reference = - reference;
		
		
		osDelay(REF_FLIP_TIME);
	}
}

void static app_main() {
	osKernelInitialize();
	T_ID1 = osThreadNew(app_ref, NULL, &ThreadAttr_ref);
	T_ID2 = osThreadNew(app_ctrl, NULL, &ThreadAttr_ctrl);
	osKernelStart();
  for (;;)
  {
    Application_Loop();
  }

}

/* Define what to do in the infinite loop */
void Application_Loop()
{
 // Do nothing
 osThreadFlagsWait(0x01, osFlagsWaitAll, osWaitForever);
}
