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


struct msg {
  uint32_t time;
  int32_t vel;
};

struct msg data;


// Thread definitions
static const osThreadAttr_t ThreadAttr_com = {
	.name = "com",		
  .priority	= osPriorityAboveNormal,
};

static const osThreadAttr_t ThreadAttr_ctrl = {
	.name = "ctrl",		
  .priority	= osPriorityNormal,
};

static const osThreadAttr_t ThreadAttr_main = {
	.name = "main",		
  .priority	= osPriorityBelowNormal,
};



osThreadId_t T_ID1, T_ID2, T_ID3;


// Define timer IDs
osTimerId_t timer_ctrl, timer_ref;

// Predefine functions
void static app_main();
void static app_com();
void static app_ctrl();
void callback_signal_flags(void *argument);

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
	T_ID1 = osThreadNew(app_com, NULL, &ThreadAttr_com);
	T_ID2 = osThreadNew(app_ctrl, NULL, &ThreadAttr_ctrl);
	T_ID3 = osThreadNew(app_main, NULL, &ThreadAttr_main);
	
	// Start kernel
	osKernelStart();
}

void callback_signal_flags(){
	// Callback function that sets thread flag
	osThreadFlagsSet(T_ID2, 0x01); // Set flag 011 to thread T_ID2
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

			
			if (com_success == 1){
				// If flag indicates success in recieving control signal, then we actuate motor
				Peripheral_PWM_ActuateMotor(control);
			}
			else if (com_success == 0)
			{
				// If there is an error in recieving control signal, stop motor
				TIM3->CCR1 = 0;
				TIM3->CCR2 = 0;
				control = 0;
				osThreadFlagsSet(T_ID3, 0x01);
			}
		
	}

}

void app_com(){
	for(;;) {
		
		osThreadFlagsWait(0x01, osFlagsWaitAll, osWaitForever); // Wait until velocity has been read and ctrl thread gives go-ahead
		com_success= 0; // Reset flag
		
		if((socket_return = send(APP_SOCK, (uint8_t*)&data, sizeof(data))) == sizeof(data)) //
		{
			
			printf("Sending vel: %d \n\r", data.vel); // %d is for int, \n\r is newline and carriage return
			printf("Sending time: %d \n\r", data.time);
			
			if((socket_return = recv(APP_SOCK, (uint8_t*)&control, sizeof(control))) == sizeof(control))
			{
				printf("Control signal recieve success: %d\n\r", control);
				com_success= 1;
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
	osThreadFlagsSet(T_ID2, 0x01);
	}
}

void static app_main() {
	// Define timer with callback function, set to periodic, define argument to callback
	timer_ctrl = osTimerNew(callback_signal_flags, osTimerPeriodic, NULL, NULL);

	
	// Start timers with set period time
	osTimerStart(timer_ctrl, SAMPLE_TIME);
	
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
		printf("Successfully opened socket \n\r");
		
		// Try to connect to server
		printf("Connecting");
		if((socket_return = connect(APP_SOCK, server_addr, SERVER_PORT)) == SOCK_OK)
		{
			printf("Socket has been connected\n\r");
			socket_return = getsockopt(APP_SOCK, SO_STATUS, &socket_status);
			while(socket_status == SOCK_ESTABLISHED)
			{
					osThreadFlagsWait(0x01, osFlagsWaitAll, osWaitForever);
					socket_return = getsockopt(APP_SOCK, SO_STATUS, &socket_status);
			}
			printf("Socket disconnected \n\r");
		}
		else
		{
			printf("Failure opening socket \n\r");
		}
		osDelay(100);
		
	}
}
