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
#include <rtx_os.h>
#include <stdio.h>

#define LED1_ON 1
#define LED1_OFF 0

osThreadId_t main_ID,led_ID1,led_ID2,worker_ID1,worker_ID2;

/*----------------------------------------------------------------------------
  Dumb delay routine
 *---------------------------------------------------------------------------*/
void delay (unsigned int count){
unsigned int index;

	for(index =0;index<count;index++)
	{
		;
	}
}

/*----------------------------------------------------------------------------
  Flash LED 1

 *---------------------------------------------------------------------------*/

static const osThreadAttr_t ThreadAttr_LED1 = {
	"LED_Thread_1",
	NULL,										//attributes
	NULL,										//cb memory
	NULL,										//cb size
	NULL,										//stack memory
	1024,										//stack size			This memory is allocated from the global memory pool
	osPriorityNormal,
	NULL,										//trust zone id
	NULL										//reserved
};


__NO_RETURN void led_thread1 (void *argument) 
{
int32_t stackSize;
	
	stackSize = osThreadGetStackSize(led_ID1);
	printf("LED 1 Thread stack size is %d",stackSize);
	for (;;) 
	{
		LED_On(1);
		delay(100);
		LED_Off(1);
		delay(100);
	}
}

/*----------------------------------------------------------------------------
 Flash LED 2
 *---------------------------------------------------------------------------*/

static uint64_t LED2_thread_stk[64];
static osRtxThread_t LED2_thread_tcb;

static const osThreadAttr_t ThreadAttr_LED2 = {
	"LED_Thread_2",
	NULL,														//attributes
	&LED2_thread_tcb,								//cb memory
	sizeof(LED2_thread_tcb),				//cb size
	&LED2_thread_stk[0],					  //stack memory						Here the control block and user stack space are statically allocated
	sizeof(LED2_thread_stk),				//stack size
	osPriorityNormal,
	NULL,														//trust zone id
	NULL														//reserved
};


__NO_RETURN void led_thread2 (void *argument) {
	
	for (;;){
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
	.name = "app_main",	
};

void app_main (void *argument){
	
	LED_Initialize ();
	led_ID1 = osThreadNew(led_thread1, NULL,&ThreadAttr_LED1);
	led_ID2 = osThreadNew(led_thread2, NULL, &ThreadAttr_LED2);
}

int main (void) {
  
  SystemCoreClockUpdate();
 	LED_Initialize ();
  osKernelInitialize();                 								// Initialize CMSIS-RTOS
  osThreadNew(app_main, NULL,&ThreadAttr_app_main);    	// Create application main thread
  if (osKernelGetState() == osKernelReady) {
    osKernelStart();                    								// Start thread execution
  }
  while(1);
}

