/*----------------------------------------------------------------------------
	
	Designers Guide to the Cortex-M Family
	CMSIS RTOS Delay Example

*----------------------------------------------------------------------------*/
#include "STM32F10x.h"
#include <cmsis_os2.h>
#include "Board_LED.h"


/*----------------------------------------------------------------------------
  Task 1 'ledOn': switches the LED on
 *---------------------------------------------------------------------------*/
 static const osThreadAttr_t ThreadAttr_LED1 = {
	.name = "LED1",
	.priority = osPriorityAboveNormal,
};
 
__NO_RETURN void led1 (void  *argument) {
	
	for (;;) {
		LED_On(1);                          //Toggle LED 1 with a relative delay
		osDelay(50);
		LED_Off(1);
		osDelay(50);
	}
}

/*----------------------------------------------------------------------------
  Task 2 'ledOff': switches the LED off
 *---------------------------------------------------------------------------*/
static const osThreadAttr_t ThreadAttr_LED2 = {
	.name = "LED2",
	.priority = osPriorityAboveNormal,
};

uint64_t ticks;

__NO_RETURN void led2 (void  *argument) {
	
	for (;;) {
		ticks = osKernelGetTickCount();	
    LED_On(2);                          
		osDelayUntil((ticks + 100));		//Toggle LED 2 with an absolute delay
		LED_Off(2);
		osDelayUntil((ticks+200));
	}
}

osThreadId_t main_ID,led1_ID,led2_ID;	

/*----------------------------------------------------------------------------
  Main: Initialize and start RTX Kernel
 *---------------------------------------------------------------------------*/
static const osThreadAttr_t ThreadAttr_app_main = {
	.name = "app_main",	
};

void app_main (void *argument) {
	
	led1_ID = osThreadNew(led1, NULL, &ThreadAttr_LED1);		//Create the LED threads
	led2_ID = osThreadNew(led2, NULL, &ThreadAttr_LED2);
}

int main (void) {

  SystemCoreClockUpdate();
 	LED_Initialize ();
  osKernelInitialize();                 									// Initialize CMSIS-RTOS
  osThreadNew(app_main, NULL, &ThreadAttr_app_main);    	// Create application main thread
  if (osKernelGetState() == osKernelReady) {
    osKernelStart();                    									// Start thread execution
  }
  while(1);
}
