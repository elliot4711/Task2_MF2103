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
#define REF_FLIP_TIME 4000


// Global variables ----------------------------------------------------------*/
int32_t reference, control;
uint8_t socket_return;
uint8_t socket_status;

struct msg {
  uint32_t time;
  int32_t vel;
};

struct msg data;



// Thread definitions
static const osThreadAttr_t ThreadAttr_ref = {
	.name = "ref",		
  .priority	= osPriorityNormal,
};

static const osThreadAttr_t ThreadAttr_com = {
	.name = "com",		
  .priority	= osPriorityAboveNormal,
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
void static app_com();
void callback_signal_flags();

void Application_Setup()
{
	// Reset global variables
	reference = 2000;
	data.vel = 0;
	control = 0;
	data.time = 0;

	// Initialise hardware
	Peripheral_GPIO_EnableMotor();

	// Initialize controller
	Controller_Reset();
		
	// Initialize kernel
	osKernelInitialize();
	
	// Create new threads
	T_ID1 = osThreadNew(app_ref, NULL, &ThreadAttr_ref);
	T_ID2 = osThreadNew(app_com, NULL, &ThreadAttr_com);
	T_ID3 = osThreadNew(app_main, NULL, &ThreadAttr_main);
	
	// Start kernel
	osKernelStart();
}

void callback_signal_flags(){
	// Callback function that sets thread flag
	osThreadFlagsSet(T_ID1, 0x01); // Set flag 101 to thread T_ID1
}


void app_com() {
		for(;;) {
			
			osThreadFlagsWait(0x01, osFlagsWaitAll, osWaitForever); // Wait until all flags (01) are set
			
			if ((socket_return = recv(APP_SOCK, (uint8_t*)&data, sizeof(data))) == sizeof(data))
			{
				printf("Received data successfully: time=%u, vel=%d\n", data.time, data.vel);
			
			control = Controller_PIController(&reference, &data.vel, &data.time);
			
			if((socket_return = send(APP_SOCK, (uint8_t*)&control, sizeof(control))) == sizeof(control))
			{
				printf("Sent control signal: %d\n\r", control);
			}
			else
			{
				printf("Failure when sending control signal \n\r");
				Controller_Reset();
				data.vel = 0;
			}
		}
		else
		{
			printf("Failure in recieving velocity \n\r");
			Controller_Reset();
			data.vel = 0;
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
	timer_ref = osTimerNew(callback_signal_flags, osTimerPeriodic, NULL, NULL);
	
	// Start timers with set period time
	osTimerStart(timer_ref, REF_FLIP_TIME);
	
  for (;;)
  {
		Application_Loop();
  }

}

void Application_Loop()
{
		printf("Opening socket");
		if((socket_return = socket(APP_SOCK, SOCK_STREAM, SERVER_PORT, SF_TCP_NODELAY)) == APP_SOCK)
		{
			printf("Socket successfully opened \n\r");
			//Try to listen to server
			printf("Listening to socket");
			if ((socket_return = listen(APP_SOCK)) == SOCK_OK)
			{
				printf("Successfully listened to socket \n\r");
				socket_return = getsockopt(APP_SOCK, SO_STATUS, &socket_status);
				while (socket_status == SOCK_LISTEN || socket_status == SOCK_ESTABLISHED)
				{
					if (socket_status == SOCK_ESTABLISHED)
					{
						osThreadFlagsSet(T_ID2, 0x01);
						osThreadFlagsWait(0x01, osFlagsWaitAll, osWaitForever);
					}
					else
					{
						printf("Something went wrong, socket not established \n\r");
						osDelay(10);
					}
					socket_return = getsockopt(APP_SOCK, SO_STATUS, &socket_status);
				}
				printf("Socket has been disconnected\n\r");
				Controller_Reset();
				close(APP_SOCK);
				printf("Socket has been closed \n\r");
			}
		}
		else // Socket cant open
		{
			printf("Socket failed to opem \n\r");
			
		}
		osDelay(500);
}
