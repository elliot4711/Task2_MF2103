#include "stm32l4xx.h"

int main(void) {
  /* Init LED1 */
  RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;      // Enable GPIOA clock
  GPIOA->MODER &= ~GPIO_MODER_MODER5;       // Output mode
  GPIOA->MODER |= GPIO_MODER_MODER5_0;
  GPIOA->OSPEEDR &= ~GPIO_OSPEEDR_OSPEED5;  // Output: high speed
  GPIOA->OSPEEDR |= GPIO_OSPEEDR_OSPEED5_1;

  /* Init USER_BUTTON */
  RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;      // Enable GPIOC clock
  GPIOC->MODER &= ~GPIO_MODER_MODER13;      // Set GPIOC Pin 13 in input mode

  char user_button;
  // Infinite loop
  while(1) {
    //Read Pin13
    user_button = (GPIOC->IDR & GPIO_IDR_ID13) >> GPIO_IDR_ID13_Pos;

    if (user_button == 1)
      GPIOA->BSRR = GPIO_BSRR_BR5; // Turn LED off
    else
      GPIOA->BSRR = GPIO_BSRR_BS5;  // Turn LED on
  }
}