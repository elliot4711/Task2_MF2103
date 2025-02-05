
/*----------------------------------------------------------------------------
	
	Designers Guide to the Cortex-M Family
	CMSIS Multiple Threads Example

*----------------------------------------------------------------------------*/
#include "STM32F10x.h"
#include "cmsis_os2.h"
#include "Board_LED.h"

/*----------------------------------------------------------------------------
  Simple delay loop 
 *---------------------------------------------------------------------------*/

void delay (unsigned int count){
unsigned int index;

	for(index =0;index<count;index++){
		;
	}
}

/*----------------------------------------------------------------------------
  led Flasher thread
 *---------------------------------------------------------------------------*/
static const osThreadAttr_t ThreadAttr_LedSwitcher1 = {
	.name = 	"LedSwitcher1",
	};

static const osThreadAttr_t ThreadAttr_LedSwitcher2 = {
 .name = 	"LedSwitcher2",
	};

__NO_RETURN void ledSwitcher (void  *argument) {
	for (;;) {
		LED_On((uint32_t)argument);                          
		delay(500);
		LED_Off((uint32_t)argument);
		delay(500);
	}
}

//define the threads allow two instances of ledSwitcher
osThreadId_t main_ID,led_ID1,led_ID2;	


/*----------------------------------------------------------------------------
  Main: Initialize and start RTX Kernel
 *---------------------------------------------------------------------------*/
static const osThreadAttr_t ThreadAttr_app_main = {
	.name = "app_main",	
};

void app_main (void *argunent){
	LED_Initialize();
	led_ID1 = osThreadNew(ledSwitcher,(void *) 1UL,&ThreadAttr_LedSwitcher1);		//Create first instance of ledswitcher
	led_ID2 = osThreadNew(ledSwitcher,(void *) 2UL,&ThreadAttr_LedSwitcher2);		//Create a second instance of ledSwitcher
  while(1)
  {
    ;
  }
}

int main (void) {

  SystemCoreClockUpdate();
 	LED_Initialize ();
  osKernelInitialize();                 								// Initialize CMSIS-RTOS
  osThreadNew(app_main, NULL, &ThreadAttr_app_main);    // Create application main thread
  if (osKernelGetState() == osKernelReady) {
    osKernelStart();                    								// Start thread execution
  }

  while(1);
}
