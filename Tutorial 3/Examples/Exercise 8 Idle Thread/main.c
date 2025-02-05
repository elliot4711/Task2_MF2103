/*----------------------------------------------------------------------------
	
	Designers Guide to the Cortex-M Family
	CMSIS RTOS Idle Example

*----------------------------------------------------------------------------*/
#include "STM32F10x.h"
#include "cmsis_os2.h"
#include "Board_LED.h"


/*----------------------------------------------------------------------------
  Task 1 'ledOn': switches the LED on
*----------------------------------------------------------------------------*/
static const osThreadAttr_t ThreadAttr_LED1 = {
	.name = "LED1",
};

__NO_RETURN void led1 (void *argument) {
	
	for (;;) {
		LED_On(1);                                        
		osDelay(500);
		LED_Off(1);
		osDelay(500);
	}
}

/*----------------------------------------------------------------------------
  Task 2 'ledOff': switches the LED off
 *---------------------------------------------------------------------------*/
static const osThreadAttr_t ThreadAttr_LED2 = {
	.name = "LED2",
};

__NO_RETURN void led2 (void *argument) {
	
	for (;;) {
    LED_On(2);                                        
		osDelay(100);
		LED_Off(2);
		osDelay(100);
	}
}



/*----------------------------------------------------------------------------
  Main: Initialize and start RTX Kernel
 *---------------------------------------------------------------------------*/

osThreadId_t main_ID, led1_ID,led2_ID;

static const osThreadAttr_t ThreadAttr_app_main = {
	.name = "app_main",	
};

void app_main (void *argument) {
		
	LED_Initialize ();
	led1_ID = osThreadNew(led1, NULL, &ThreadAttr_LED1);
	led2_ID = osThreadNew(led2, NULL, &ThreadAttr_LED2);
}


int main (void) {

  SystemCoreClockUpdate();
  osKernelInitialize();                 								// Initialize CMSIS-RTOS
  osThreadNew(app_main, NULL,&ThreadAttr_app_main);    	// Create application main thread
  if (osKernelGetState() == osKernelReady) {
    osKernelStart();                    								// Start thread execution
  }

  while(1);
}
 
