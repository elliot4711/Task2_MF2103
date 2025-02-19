#include "main.h" 

#include "application.h" 
#include "controller.h"
#include "peripherals.h"
#include "cmsis_os2.h"
#include "socket.h"
#include "wizchip_conf.h"
#include <stdio.h>

#define APP_SOCK 0
#define SERVER_PORT 2103
#define CLIENT_PORT 2104
#define SAMPLE_TIME 10
#define REF_FLIP_TIME 4000


// Global variables ----------------------------------------------------------*/
int32_t reference, velocity, control;
uint32_t millisec;

uint8_t retval_server;
uint8_t sock_status_server;


// Thread definitions
static const osThreadAttr_t ThreadAttr_ref = {
	.name = "ref",		
  .priority	= osPriorityAboveNormal,
};

static const osThreadAttr_t ThreadAttr_sendrecieve = {
	.name = "sendrecieve",		
  .priority	= osPriorityNormal,
};

static const osThreadAttr_t ThreadAttr_main = {
	.name = "main",		
  .priority	= osPriorityBelowNormal,
};



osThreadId_t T_ID1, T_ID2, T_ID3;


// Define timer IDs
osTimerId_t timer_ref;

// Predefine functions
void static app_main();
void static app_ref();
void static app_sendrecieve();
void callback_signal_flags(void *argument);

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
	T_ID2 = osThreadNew(app_sendrecieve, NULL, &ThreadAttr_sendrecieve);
	T_ID3 = osThreadNew(app_main, NULL, &ThreadAttr_main);
	
	// Start kernel
	osKernelStart();
}

void callback_signal_flags(void *argument){
	// Callback function that sets thread flags based on input
	int32_t val = (int)(uintptr_t)argument;
	if (val == 1){
		osThreadFlagsSet(T_ID1, 0x01); // Set flag 101 to thread T_ID1
	}
	else {
		//Why are you here?
	}
}


void app_sendrecieve () {
		for(;;) {
		
			osThreadFlagsWait(0x01, osFlagsWaitAll, osWaitForever); // Wait until all flags (01) are set
			
			if ((retval_server = recv(APP_SOCK, (uint8_t*)&velocity, sizeof(velocity))) == sizeof(velocity))
			{
			printf("Received: %d\n\r", velocity);
			millisec = Main_GetTickMillisec();
			
			control = Controller_PIController(&reference, &velocity, &millisec);
			
			if((retval_server = send(APP_SOCK, (uint8_t*)&control, sizeof(control))) == sizeof(control))
			{
				printf("Sent: %d\n\r", control);
			}
			else
			{
				printf("Could not send control signal!\n\r");
				Controller_Reset();
				velocity = 0;
			}
		}
		else
		{
			printf("Could not receive velocity!\n\r");
			Controller_Reset();
			velocity = 0;
		}
		osThreadFlagsSet(T_ID3, 0x01);
	}

}

void app_ref(){
	for(;;) {
		
		osThreadFlagsWait(0x01, osFlagsWaitAll, osWaitForever); // Wait until all flags (01) are set
		reference = -reference;
		Controller_Reset();
	
	}
}

void static app_main() {
	// Define timer with callback function, set to periodic, define argument to callback
	timer_ref = osTimerNew(callback_signal_flags, osTimerPeriodic, (void *)(uintptr_t)1, NULL);
	
	// Start timers with set period time
	osTimerStart(timer_ref, REF_FLIP_TIME);
	
  for (;;)
  {
		Application_Loop();
  }

}

void Application_Loop()
{
		printf("Opening socket... ");
		if((retval_server = socket(APP_SOCK, SOCK_STREAM, SERVER_PORT, SF_TCP_NODELAY)) == APP_SOCK)
		{
			printf("Success!\n\r");
			//Try to listen to server
			printf("Listening... ");
			if ((retval_server = listen(APP_SOCK)) == SOCK_OK)
			{
				printf("Success!\n\r");
				retval_server = getsockopt(APP_SOCK, SO_STATUS, &sock_status_server);
				while (sock_status_server == SOCK_LISTEN || sock_status_server == SOCK_ESTABLISHED)
				{
					if (sock_status_server == SOCK_ESTABLISHED)
					{
						osThreadFlagsSet(T_ID2, 0x01);
						osThreadFlagsWait(0x01, osFlagsWaitAll, osWaitForever);
					}
					else
					{
						printf("Something went wrong!\n\r");
						osDelay(10);
					}
					retval_server = getsockopt(APP_SOCK, SO_STATUS, &sock_status_server);
				}
				printf("Disconnected! \n\r");
				Controller_Reset();
				close(APP_SOCK);
				printf("Socket closed!\n\r");
			}
		}
		else // Socket cant open
		{
			printf("Failed to open socket!\n\r");
			
		}
		//Wait 500ms before trying again
		osDelay(500);
}
