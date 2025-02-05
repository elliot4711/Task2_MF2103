/*----------------------------------------------------------------------------
	
	Designers Guide to the Cortex-M Family
	CMSIS RTOS Threads Example

*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Include the microcontroller header for register defenitions and CMSIS core functions
	Include the CMSIS RTOS header for the RTOS API
 *---------------------------------------------------------------------------*/

#include "STM32F10x.h"
#include <cmsis_os2.h>
#include "Board_LED.h"

#define LED1_ON 1

#define LED1_OFF 0
osThreadId_t main_ID,led_ID1,led_ID2,worker_ID1,worker_ID2;

/*----------------------------------------------------------------------------
  Dumb delay routine
 *---------------------------------------------------------------------------*/
void delay (unsigned int count){
unsigned int index;

	for(index =0;index<count;index++){
		;
	}
}

/*----------------------------------------------------------------------------
  Flash LED 1
 *---------------------------------------------------------------------------*/
static const osThreadAttr_t ThreadAttr_LED1 = {
	"LED_Thread_1",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	osPriorityNormal,
	NULL,
	NULL
};

__NO_RETURN void led_thread1 (void *argument){

	for (;;){
		LED_On(1);
		delay(100);
		LED_Off(1);
		delay(100);
	}
}

/*----------------------------------------------------------------------------
 Flash LED 1
 *---------------------------------------------------------------------------*/
static const osThreadAttr_t ThreadAttr_LED2 = {
	.name = "LED_Thread_2",		
  .priority	= osPriorityAboveNormal, //This decleration requires C99 to be selected in the project compiler options
};

__NO_RETURN void led_thread2 (void *argument){

	for (;;) {
		LED_On(2);
		delay(100);
		LED_Off(2);
		delay(100);
	}
}

/*----------------------------------------------------------------------------
 Initilise the LED's and start the RTOS
 *---------------------------------------------------------------------------*/
static const osThreadAttr_t ThreadAttr_app_main = {
	"app_main",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	osPriorityNormal,
	NULL,
	NULL
};


void app_main (void *argument) {

	LED_Initialize ();
	led_ID1 = osThreadNew(led_thread1, NULL, &ThreadAttr_LED1);
	led_ID1 = osThreadNew(led_thread2, NULL, &ThreadAttr_LED2);
}

int main (void) {

  SystemCoreClockUpdate();
 	LED_Initialize ();
  osKernelInitialize();                 								// Initialize CMSIS-RTOS
  osThreadNew(app_main, NULL,&ThreadAttr_app_main);    	// Create application main thread
  if (osKernelGetState() == osKernelReady){
    osKernelStart();                    								// Start thread execution
  }
  while(1);
}

