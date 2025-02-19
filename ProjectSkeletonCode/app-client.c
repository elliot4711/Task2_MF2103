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
int8_t retval_client;
uint8_t sock_status_client;
uint8_t flag;
uint8_t server_addr[4] = {192, 168, 0, 10};


struct msg {
  uint32_t time;
  int32_t vel;
};


// Thread definitions
static const osThreadAttr_t ThreadAttr_sendrecieve = {
	.name = "sendrecieve",		
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
osTimerId_t timer_ctrl, timer_ref;

// Predefine functions
void static app_main();
void static app_sendrecieve();
void static app_ctrl();
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
	T_ID1 = osThreadNew(app_sendrecieve, NULL, &ThreadAttr_sendrecieve);
	T_ID2 = osThreadNew(app_ctrl, NULL, &ThreadAttr_ctrl);
	T_ID3 = osThreadNew(app_main, NULL, &ThreadAttr_main);
	
	// Start kernel
	osKernelStart();
}

void callback_signal_flags(void *argument){
	// Callback function that sets thread flags based on input
	int32_t val = (int)(uintptr_t)argument;
	
	if (val == 0){
		osThreadFlagsSet(T_ID2, 0x01); // Set flag 011 to thread T_ID2
		
	}
	else {
		//Why are you here?
	}
}


void app_ctrl () {
		for(;;) {
		
			osThreadFlagsWait(0x01, osFlagsWaitAll, osWaitForever); // Wait until all flags (01) are set
			
			// Get time
			millisec = Main_GetTickMillisec();

			// Calculate motor velocity
			velocity = Peripheral_Encoder_CalculateVelocity(millisec);
			
			osThreadFlagsSet(T_ID1, 0x01);
			osThreadFlagsWait(0x01, osFlagsWaitAll, osWaitForever); // Wait until all flags (01) are set

			
			if (flag == 1){
				Peripheral_PWM_ActuateMotor(control);
			}
			else if (flag == 0)
			{
				TIM3->CCR1 = 0;
				TIM3->CCR2 = 0;
				control = 0;
				osThreadFlagsSet(T_ID3, 0x01);
			}
		
	}

}

void app_sendrecieve(){
	for(;;) {
		
		osThreadFlagsWait(0x01, osFlagsWaitAll, osWaitForever); // Wait until all flags (01) are set
		flag = 0;
		
		if((retval_client = send(APP_SOCK, (uint8_t*)&velocity, sizeof(velocity))) == sizeof(velocity))
		{
			printf("Sending velocity: %d \n\r", velocity);
			
			if((retval_client = recv(APP_SOCK, (uint8_t*)&control, sizeof(control))) == sizeof(control))
			{
				printf("Receive control: %d\n\r", control);
				flag = 1;
			}
			else
			{
				printf("Failed to receive control!!! \n\r");
			}
		}
		else
		{
			printf("Failed to send velocity!!! \n\r");
		}
	osThreadFlagsSet(T_ID2, 0x01);
	}
}

void static app_main() {
	// Define timer with callback function, set to periodic, define argument to callback
	timer_ctrl = osTimerNew(callback_signal_flags, osTimerPeriodic, (void *)(uintptr_t)0, NULL);

	
	// Start timers with set period time
	osTimerStart(timer_ctrl, SAMPLE_TIME);
	
  for (;;)
  {
		Application_Loop();
  }

}

void Application_Loop()
{
	//printf("Opening socket");
	if((retval_client = socket(APP_SOCK, SOCK_STREAM, SERVER_PORT, SF_TCP_NODELAY)) == APP_SOCK)
	{
		printf("Success!\n\r");
		
		// Try to connect to server
		printf("Connecting to server... ");
		if((retval_client = connect(APP_SOCK, server_addr, SERVER_PORT)) == SOCK_OK)
		{
			printf("Socket Connected!\n\r");
			retval_client = getsockopt(APP_SOCK, SO_STATUS, &sock_status_client);
			while(sock_status_client == SOCK_ESTABLISHED)
			{
					osThreadFlagsWait(0x01, osFlagsWaitAll, osWaitForever);
					retval_client = getsockopt(APP_SOCK, SO_STATUS, &sock_status_client);
			}
			printf("Disconnected! \n\r");
		}
		else
		{
			printf("Failed to open the socket \n\r");
		}
		osDelay(100);
		
	}
}
