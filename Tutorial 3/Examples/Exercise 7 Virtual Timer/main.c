
/*----------------------------------------------------------------------------
	
	Designers Guide to the Cortex-M Family
	CMSIS RTOS User Timer Example

*----------------------------------------------------------------------------*/


#include "stm32f10x.h"
#include "cmsis_os2.h"
#include "Board_LED.h"

void callback(void *param);
void ledThread1 (void  *argument);
void ledThread2 (void  *argument);

osThreadId_t T_ledThread1, T_ledThread2;

/*----------------------------------------------------------------------------
  Timer callback function. Toggle the LED associated with the timer
 *---------------------------------------------------------------------------*/
void callback(void *param){
	
	switch( (uint32_t) param){
		
		case 0:
			GPIOB->ODR ^= 0x2;
		break;

		case 1:
			GPIOB->ODR ^= 0x4;
		break;
	}
}

void callback2(void *param){
	
	switch( (uint32_t) param){
		
		case 2:
			GPIOB->ODR ^= 0x8;	
		break;

		case 3:
			GPIOB->ODR ^= 0x10;
		break;
	}
}
/*----------------------------------------------------------------------------
  Flash LED 1 when signaled by thread 2
 *---------------------------------------------------------------------------*/
static const osThreadAttr_t ThreadAttr_LED1 = {
	.name = "LED1",
};

__NO_RETURN void ledThread1 (void *argument) {
  
	for (;;){
		osDelay(500);
		LED_On(1);                          
		osDelay(500);	
		LED_Off(1);
	}
}
/*----------------------------------------------------------------------------
  Flash LED 2 and signal thread 1
 *---------------------------------------------------------------------------*/
static const osThreadAttr_t ThreadAttr_LED2 = {
	.name = "LED2",
};

__NO_RETURN void ledThread2 (void  *argument) {
  
	for (;;) {
		LED_On(2);		
		osDelay(500);
		LED_Off(2);
		osDelay(500);
	}
}




/*----------------------------------------------------------------------------
 Create and start the timers then initilise the LED, Start the threads
 *---------------------------------------------------------------------------*/

osTimerId_t timer0,timer1,timer2,timer3; 

static const  osTimerAttr_t timerAttr_timer0 = {
	.name = "timer_0",
};

static const osThreadAttr_t ThreadAttr_app_main = {
	.name = "app_main",	
};

void app_main (void *argument) {
	
	timer0 = osTimerNew(&callback, osTimerPeriodic,(void *)0, &timerAttr_timer0);	
	timer1 = osTimerNew(&callback, osTimerPeriodic,(void *)1, NULL);	
	timer2 = osTimerNew(&callback2, osTimerPeriodic,(void *)2, NULL);	
	timer3 = osTimerNew(&callback2, osTimerPeriodic,(void *)3, NULL);	
			                  
	osTimerStart(timer0, 500);	
	osTimerStart(timer1, 100);	
	osTimerStart(timer2, 300);	
	osTimerStart(timer3, 200);	

	T_ledThread1 = osThreadNew(ledThread1, NULL, &ThreadAttr_LED1);
	T_ledThread2 = osThreadNew(ledThread2, NULL, &ThreadAttr_LED2);	
}

int main (void){

  SystemCoreClockUpdate();
  LED_Initialize ();
  osKernelInitialize();                 								// Initialize CMSIS-RTOS
  osThreadNew(app_main, NULL, &ThreadAttr_app_main);    // Create application main thread
  if (osKernelGetState() == osKernelReady) {
    osKernelStart();                    								// Start thread execution
  }

  while(1);
}
