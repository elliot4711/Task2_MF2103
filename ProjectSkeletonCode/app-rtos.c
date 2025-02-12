#include "main.h" 

#include "application.h" 
#include "controller.h"
#include "peripherals.h"
#include "cmsis_os2.h"

#define SAMPLE_TIME 10
#define REF_FLIP_TIME 4000
#define MAIN_TIME 1

/* Global variables ----------------------------------------------------------*/
int32_t reference, velocity, control;
uint32_t millisec;
uint32_t timer_check;

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
  .priority	= osPriorityNormal, //This decleration requires C99 to be selected in the project compiler options
};

/* Functions -----------------------------------------------------------------*/


// Define threads
osThreadId_t T_ID1, T_ID2, T_ID3;

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
	app_main();
}

void callback_signal_flags(void *argument){
	switch( (uint32_t) argument)
	{
		case 0:
			osThreadFlagsSet(T_ID1, 0x03);
		break;
		
		case 1:
			osThreadFlagsSet(T_ID2, 0x05);
		break;
				
		//case 2:
			//osThreadFlagsSet(T_ID3, 0x07);
		//break;
			
	}
}


void app_ctrl () {
		for(;;) {
		
			osThreadFlagsWait(0x03, osFlagsWaitAll, osWaitForever);
			
			// Get time
			millisec = Main_GetTickMillisec();

			// Calculate motor velocity
			velocity = Peripheral_Encoder_CalculateVelocity(millisec);

			// Calculate control signal
			control = Controller_PIController(&reference, &velocity, &millisec);

			// Apply control signal to motor
			Peripheral_PWM_ActuateMotor(control);
		
		
			//osDelay(SAMPLE_TIME);
	}

}

void app_ref(){
	for(;;) {
		
		osThreadFlagsWait(0x05, osFlagsWaitAll, osWaitForever);
		
		reference = - reference;
		
		
		//osDelay(REF_FLIP_TIME);
	}
}

void static app_main() {
	osKernelInitialize();
	T_ID1 = osThreadNew(app_ref, NULL, &ThreadAttr_ref);
	T_ID2 = osThreadNew(app_ctrl, NULL, &ThreadAttr_ctrl);
	//T_ID3 = osThreadNew(app_main, NULL, &ThreadAttr_main);
	
	osTimerId_t timer_ctrl;
	timer_ctrl = osTimerNew(callback_signal_flags, osTimerPeriodic, (void *)0, NULL);
	osTimerId_t timer_ref;
	timer_ref = osTimerNew(callback_signal_flags, osTimerPeriodic, (void *)1, NULL);
	//osTimerId_t timer_main;
	//timer_main = osTimerNew(callback_signal_flags, osTimerPeriodic, (void *)2, NULL);
	
	osTimerStart(timer_ctrl, SAMPLE_TIME);
	osTimerStart(timer_ref, REF_FLIP_TIME);
	//osTimerStart(timer_main, MAIN_TIME);
	
	osKernelStart();
  for (;;)
  {
		//osThreadFlagsWait(0x07, osFlagsWaitAll, osWaitForever);
		Application_Loop();
  }

}

/* Define what to do in the infinite loop */
void Application_Loop()
{
 // Do nothing
 osThreadFlagsWait(0x01, osFlagsWaitAll, osWaitForever);
}
