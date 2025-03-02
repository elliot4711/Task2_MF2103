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
#define SAMPLE_TIME 10

// Global variables ----------------------------------------------------------*/
int32_t control;
int8_t socket_return;
uint8_t socket_status;
uint8_t com_success; // Success flag
uint8_t server_addr[4] = {192, 168, 0, 10};
uint8_t timer_running = 0;


struct msg {
  uint32_t time;
  int32_t vel;
};

struct msg data;


// Thread definitions
static const osThreadAttr_t ThreadAttr_com = {
	.name = "com",		
  .priority	= osPriorityNormal,
};

static const osThreadAttr_t ThreadAttr_ctrl = {
	.name = "ctrl",		
  .priority	= osPriorityAboveNormal,
};

static const osThreadAttr_t ThreadAttr_main = {
	.name = "main",		
  .priority	= osPriorityBelowNormal,
};



osThreadId_t T_ID1, T_ID2, T_ID3;


// Define timer IDs
osTimerId_t timer_ctrl;

// Predefine functions
void static app_main();
void static app_com();
void static app_ctrl();
void callback_signal_flags();

void Application_Setup()
{
	// Reset global variables
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
	T_ID1 = osThreadNew(app_com, NULL, &ThreadAttr_com);
	T_ID2 = osThreadNew(app_ctrl, NULL, &ThreadAttr_ctrl);
	T_ID3 = osThreadNew(app_main, NULL, &ThreadAttr_main);
	
	// Start kernel
	osKernelStart();
}

void callback_signal_flags(){
	// Callback function that sets thread flag
	osThreadFlagsSet(T_ID2, 0x01); // Set flag 01 to thread T_ID2
}


void app_ctrl () {
		for(;;) {
		
			osThreadFlagsWait(0x01, osFlagsWaitAll, osWaitForever); // Wait until all flags (01) are set
			
			// Get time
			data.time = Main_GetTickMillisec();

			// Calculate motor velocity
			data.vel = Peripheral_Encoder_CalculateVelocity(data.time);
			
			osThreadFlagsSet(T_ID1, 0x01); // Give go-ahead to com thread to start communicating
			osThreadFlagsWait(0x01, osFlagsWaitAll, osWaitForever); // Wait to recieve control signal

				
			if (com_success == 0)
			{
				// If there is an error in recieving control signal, stop motor
				TIM3->CCR1 = 0;
				TIM3->CCR2 = 0;
				control = 0;
				osThreadFlagsSet(T_ID3, 0x01);
			}
			
			else if (com_success == 1){
				// If flag indicates success in recieving control signal, then we actuate motor
				Peripheral_PWM_ActuateMotor(control);
			}
		
	}

}

void app_com(){
	for(;;) {
		
		osThreadFlagsWait(0x01, osFlagsWaitAll, osWaitForever); // Wait until velocity has been read and ctrl thread gives go-ahead
		com_success= 0; // Reset flag
		
		socket_return = send(APP_SOCK, (uint8_t*)&data, sizeof(data));
		if((socket_return) == sizeof(data)) // send command returns the data size it sent if successful, so we check for that
		{
			
			printf("Sending vel: %d \n\r", data.vel); // %d is for int, \n\r is newline and carriage return
			printf("Sending time: %d \n\r", data.time);
			
			socket_return = recv(APP_SOCK, (uint8_t*)&control, sizeof(control));
			if((socket_return) == sizeof(control)) // recv command returns the data size it recieved if successful so we can check that against the expected size
			{
				printf("Control signal recieve success: %d\n\r", control);
				com_success = 1;
			}
			else
			{
				printf("Control signal recieve failure \n\r");
			}
		}
		else
		{
			printf("Velocity send failure \n\r");
		}
	osThreadFlagsSet(T_ID2, 0x01); // Give go-ahead to ctrl thread to continue after recieving control signal
	}
}

void static app_main() {
	// Define timer with callback function, set to periodic, define argument to callback, in this case null
	timer_ctrl = osTimerNew(callback_signal_flags, osTimerPeriodic, NULL, NULL);
	
  for (;;)
  {
		Application_Loop();
  }

}

void Application_Loop()
{
	printf("Opening socket \n\r");
	if((socket_return = socket(APP_SOCK, SOCK_STREAM, SERVER_PORT, SF_TCP_NODELAY)) == APP_SOCK) // socket returns socket number if successfully connected. 
	// SF_TCP_NODELAY forces reciever to acknowledge every packet instead of batching acks or packets together to save time, in our case this this helps reduce latency
	// SOCK_STREAM defines that we want a TCP socket
	
	{
		printf("Successfully opened socket \n\r");
		
		// Try to connect to server
		printf("Connecting \n\r");
		
		if((socket_return = connect(APP_SOCK, server_addr, SERVER_PORT)) == SOCK_OK) // connect returns SOCK_OK if successful
		{
			
			printf("Socket has been connected\n\r");
			
			socket_return = getsockopt(APP_SOCK, SO_STATUS, &socket_status); // Get socket status using keyword SO_STATUS and store it in socket_status
			
			while(socket_status == SOCK_ESTABLISHED) // Check if socket_status indicates socket is established
			{		
					if (timer_running == 0){
						// Start or restart timers with set period time if timer is not already started
						osTimerStart(timer_ctrl, SAMPLE_TIME);
						timer_running = 1;
					}
					osThreadFlagsWait(0x01, osFlagsWaitAll, osWaitForever); // Wait until thread flag is 0x01 which will only be set if there is an error and com_success == 0, then we check socket status again
					// Essentially this part ensures that if there is an issue recieving the control signal, the client will try to reconnect
					socket_return = getsockopt(APP_SOCK, SO_STATUS, &socket_status);
			}
			osTimerStop(timer_ctrl);
			timer_running = 0;
			printf("Socket disconnected \n\r");
		}
		else
		{
			printf("Failure opening socket \n\r");
		}
		osDelay(100);
		
	}
}
