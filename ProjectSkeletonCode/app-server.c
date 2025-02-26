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
uint8_t timer_running = 0;

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
	osThreadFlagsSet(T_ID1, 0x01); // Set flag 01 to thread T_ID1
}


void app_com() {
		for(;;) {
			
			osThreadFlagsWait(0x01, osFlagsWaitAll, osWaitForever); // Wait until all flags (01) are set
			
			socket_return = recv(APP_SOCK, (uint8_t*)&data, sizeof(data));
			if((socket_return) == sizeof(data)) // recv command returns the data size it recieved if successful so we can check that against the expected size
			{
				printf("Received data successfully: time=%u, vel=%d\n", data.time, data.vel);
			
			control = Controller_PIController(&reference, &data.vel, &data.time);
			
			socket_return = send(APP_SOCK, (uint8_t*)&control, sizeof(control));
			if((socket_return) == sizeof(control)) // send command returns the data size it sent if successful, so we check for that
			{
				printf("Control signal has been sent: %d\n\r", control);
			}
			else
			{
				// We reset the controller at any hint of failure
				printf("Failure when sending control signal \n\r");
				Controller_Reset();
				data.vel = 0;
			}
		}
		else
		{
			// We reset the controller at any hint of failure
			printf("Failure in recieving velocity \n\r");
			Controller_Reset();
			data.vel = 0;
		}
		osThreadFlagsSet(T_ID3, 0x01); // Give go-ahead to main thread which checks if still connected
	}

}

void app_ref(){
	for(;;) {
		
		osThreadFlagsWait(0x01, osFlagsWaitAll, osWaitForever); // Wait until all flags (01) are set, this flag is set by the timer
		reference = -reference;
	
	}
}

void static app_main() {
	// Define timer with callback function, set to periodic, define argument to callback
	timer_ref = osTimerNew(callback_signal_flags, osTimerPeriodic, NULL, NULL);
	
  for (;;)
  {
		Application_Loop();
  }

}

void Application_Loop()
{
		printf("Opening socket");
		if((socket_return = socket(APP_SOCK, SOCK_STREAM, SERVER_PORT, SF_TCP_NODELAY)) == APP_SOCK) // socket returns socket number if successfully connected. 
		// SF_TCP_NODELAY forces reciever to acknowledge every packet instead of batching acks or packets together to save time, in our case this this helps reduce latency
		// SOCK_STREAM defines that we want a TCP socket
		{
			printf("Socket successfully opened \n\r");
			printf("Checking if connection is ok");

			if ((socket_return = listen(APP_SOCK)) == SOCK_OK) // Listen checks for connection request, returns SOCK_OK if succesfully connected
			{
				printf("Successfully checked connection \n");
				socket_return = getsockopt(APP_SOCK, SO_STATUS, &socket_status); // Get socket status using keyword SO_STATUS and store it in socket_status
				while (socket_status == SOCK_LISTEN || socket_status == SOCK_ESTABLISHED) // Checks if socket is listening or established
				{
					if (socket_status == SOCK_ESTABLISHED)
					{
						// Start or restart timer
						if (timer_running == 0){
							// Start timers with set period time
							osTimerStart(timer_ref, REF_FLIP_TIME);
							timer_running = 1;
						}
						else {
						}
						// If established, give go ahead to com thread and wait
						osThreadFlagsSet(T_ID2, 0x01); // Gives go-ahead to com thread to begin sending
						osThreadFlagsWait(0x01, osFlagsWaitAll, osWaitForever); // Waits until com thread gives go-ahead, essentially this checks if socket is still connected for every loop of the com thread
					}
					else
					{
						printf("Something went wrong, socket not established, trying again in 5ms \n");
						osTimerStop(timer_ref);
						timer_running = 0;
						osDelay(10);
					}
					socket_return = getsockopt(APP_SOCK, SO_STATUS, &socket_status); // Get socket status using keyword SO_STATUS and store it in socket_status
				}
				// If the socket is not listening or established we reset controller, close socket and try again
				printf("Socket has been disconnected\n\r");
				Controller_Reset();
				close(APP_SOCK);
				printf("Socket has been closed \n\r");
			}
		}
		else // Socket cant open
		{
			printf("Socket failed to open \n\r");
			
		}
		osDelay(500);
}
