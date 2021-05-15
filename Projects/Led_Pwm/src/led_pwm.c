#include <stdint.h>
#include <stdbool.h>
#include "system_tm4c1294.h" // CMSIS-Core
#include "driverleds.h" // device drivers
#include "cmsis_os2.h" // CMSIS-RTOS
// includes da biblioteca driverlib
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"


osThreadId_t control, led1_thread, led2_thread, led3_thread, led4_thread;
osMutexId_t led_mutex;
osMessageQueueId_t led1_queue, led2_queue, led3_queue, led4_queue;
osTimerId_t timer1_id, timer2_id;

int tickdebounce = 0;


typedef struct typeled{
  int i;
  int dc;
  uint8_t led_n;
  osMessageQueueId_t *queue;
} led;

void callback_timer(void *arg){
  
  
} // callback

void callback_system(void *arg){
  
  
} // callback

void button(){
  
  if (tickdebounce < osKernelGetTickCount()){
    
    if(GPIOPinRead(GPIO_PORTJ_BASE, GPIO_PIN_0) == GPIO_PIN_0) { // Testa estado do push-button SW1
      
      
    }
    if(GPIOPinRead(GPIO_PORTJ_BASE, GPIO_PIN_1) == GPIO_PIN_1) { // Testa estado do push-button SW2
      
      
    }
    tickdebounce = osKernelGetTickCount() + 300;
  }
    
  GPIOIntClear(GPIO_PORTJ_BASE, GPIO_INT_PIN_0);
} // button

void led_thread (void *arg){
  led *led = arg;
  while(1){
    int on_off;
    osMessageQueueGet(led->queue, &on_off, NULL, osWaitForever);
    osMutexAcquire(led_mutex, osWaitForever);
    if(on_off == 1){
      LEDOn(led->led_n);
    }
    else if (on_off == 0){
      LEDOff(led->led_n);
    }
    else{
      osMutexRelease(led_mutex);
    }
  } // while
}

void control_thread (void *arg){
  
  while(1){

  } // while
}


void main(void){
  SystemInit();
  LEDInit(LED4 | LED3 | LED2 | LED1);

  led led1, led2, led3, led4;
  
  led1.i = 1;
  led2.i = 2;
  led3.i = 3;
  led4.i = 4;
  led1.led_n = LED1;
  led2.led_n = LED2;
  led3.led_n = LED3;
  led4.led_n = LED4;
  *led1.queue = &led1_queue;
  *led2.queue = &led2_queue;
  *led3.queue = &led3_queue;
  *led4.queue = &led4_queue;

  
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ); // Habilita GPIO J (push-button SW1 = PJ0, push-button SW2 = PJ1)
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ)); // Aguarda final da habilitação
  
  GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0 | GPIO_PIN_1); // push-button SW1 como entrada
  GPIOPadConfigSet(GPIO_PORTJ_BASE, GPIO_PIN_0 | GPIO_PIN_1, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
  
  GPIOIntRegister(GPIO_PORTJ_BASE, button);
  GPIOIntTypeSet(GPIO_PORTJ_BASE, GPIO_PIN_0 | GPIO_PIN_1, GPIO_FALLING_EDGE);
  GPIOIntEnable(GPIO_PORTJ_BASE, GPIO_INT_PIN_0 | GPIO_PIN_1);
  
  osKernelInitialize();
  
  led1_thread = osThreadNew(led_thread, &led1, NULL);
  led2_thread = osThreadNew(led_thread, &led2, NULL);
  led3_thread = osThreadNew(led_thread, &led3, NULL);
  led4_thread = osThreadNew(led_thread, &led4, NULL);
  control = osThreadNew(control_thread, NULL, NULL);
  
  led1_queue = osMessageQueueNew(10, sizeof(int), NULL);
  led2_queue = osMessageQueueNew(10, sizeof(int), NULL);
  led3_queue = osMessageQueueNew(10, sizeof(int), NULL);
  led4_queue = osMessageQueueNew(10, sizeof(int), NULL);
  
  timer1_id = osTimerNew(callback_timer, osTimerPeriodic, NULL, NULL);
  
  
  if(osKernelGetState() == osKernelReady)
    osKernelStart();

  while(1);
} // main