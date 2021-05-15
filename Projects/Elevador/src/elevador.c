#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "system_tm4c1294.h"
#include "cmsis_os2.h"

// includes do driverlib
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/pin_map.h"
#include "driverlib/uart.h"

#include "inc/hw_memmap.h"
#include "utils/uartstdio.h"

osThreadId_t elevador_left_id, elevador_mid_id, elevador_right_id, UART_Read_id, UART_Write_id;
osMessageQueueId_t queue_left_id,queue_right_id,queue_mid_id,queue_uartwrite_id;
osMutexId_t Mutex_UART;

typedef struct {
  int andar;
  char pos;
  int status; // 1 = parado, 2 = subindo, 3 = descendo
  bool paradas[16];
  osMessageQueueId_t fila;
}struct_elevador;

extern void UARTStdioIntHandler(void);


void abrir(char pos){
  char enviar[2];
  enviar[0]=pos;
  enviar[1]='a';//abrir
  osMessageQueuePut(queue_uartwrite_id, enviar, NULL, osWaitForever);
  osDelay(1500);
}

void fechar(char pos){
  char enviar[2];
  enviar[0]=pos;
  enviar[1]='f';//fechar
  osMessageQueuePut(queue_uartwrite_id, enviar, NULL, osWaitForever);
  osDelay(1500);
}

void subir(char pos){
  char enviar[2];
  enviar[0]=pos;
  enviar[1]='s';//subir
  osMessageQueuePut(queue_uartwrite_id, enviar, NULL, osWaitForever);
}

void descer(char pos){
  char enviar[2];
  enviar[0]=pos;
  enviar[1]='d';//descer
  osMessageQueuePut(queue_uartwrite_id, enviar, NULL, osWaitForever);
}

void parar(char pos){
  char enviar[2];
  enviar[0]=pos;
  enviar[1]='p';//parar
  osMessageQueuePut(queue_uartwrite_id, enviar, NULL, osWaitForever);
  osDelay(1500);
}

void receber(struct_elevador e){
  parar(e.pos);
  osDelay(500);
  abrir(e.pos);
  osDelay(500);
  fechar(e.pos);
  osDelay(500);
}

void reset(char pos){
  char enviar[2];
  enviar[0]=pos;
  enviar[1]='r';
  osMessageQueuePut(queue_uartwrite_id, enviar, NULL, osWaitForever);
}

int conv_letra_andar(char andar_num)
{
  int andar;
  switch(andar_num)
  {
  case 'a':
    {andar=0;break;}
  case 'b':
    {andar=1;break;}
  case 'c':
    {andar=2;break;}
  case 'd':
    {andar=3;break;}
  case 'e':
    {andar=4;break;}
  case 'f':
    {andar=5;break;}
  case 'g':
    {andar=6;break;}
  case 'h':
    {andar=7;break;}
  case 'i':
    {andar=8;break;}
  case 'j':
    {andar=9;break;}
  case 'k':
    {andar=10;break;}
  case 'l':
    {andar=11;break;}
  case 'm':
    {andar=12;break;}
  case 'n':
    {andar=13;break;}
  case 'o':
    {andar=14;break;}
  case 'p':
    {andar=15;break;}
  }
  return andar;
}

int conv_num_andar(char dezena, char unidade){
  return (dezena-0x30)*10+(unidade-0x30);  
}

int get_andar(char dezena, char unidade){
  if(unidade=='\0'){
    return   (dezena-0x30);
  }
  return (dezena-0x30)*10+(unidade-0x30);
}

void adicionar_parada(struct_elevador* e,int andar_destino){
  e->paradas[andar_destino]=true;
}

void remover_parada(struct_elevador* e){
  e->paradas[e->andar]=false;
}

bool sem_parada(bool*vet){
  for(int i=0;i<=15;i++)
  {
    if(vet[i]==true)
    {
      return false;
    }
  }
  return true;
}

bool comparar_andar(int andar_atual,bool *vet){
  if(vet[andar_atual]==true)
  {
    return true;
  }
  else{return false;}
}


bool paradas_acima(struct_elevador e){
  for(int i=0;i<=15;i++){
    if(i>e.andar)
    {
      if(e.paradas[i]==true)
      {
        return true;
      }
    }
  }
  return false;
}


bool paradas_abaixo(struct_elevador e){
  for(int i=0;i<=15;i++){
    if(i<e.andar)
    {
      if(e.paradas[i]==true)
      {
        return true;
      }
    }
  }
  return false;
}

// *************** Funções e Configuração da UART ************

void UART_Read(void *arg){
  char instr[6];
  while(1){
    UARTgets(instr,20);
    if(instr[0]=='e'){
      osMessageQueuePut(queue_left_id, instr, NULL, 0);
    }
    else if(instr[0]=='c'){
      osMessageQueuePut(queue_mid_id, instr, NULL, 0);
    }
    else if(instr[0]=='d'){
      osMessageQueuePut(queue_right_id, instr, NULL, 0);
    }  
    osThreadYield();
  }
}

void UART_Write(void *arg){
  char instr[2];
  while(1){
    osMessageQueueGet(queue_uartwrite_id, instr, NULL, osWaitForever);
    UARTprintf("%c%c\n\r",instr[0],instr[1]); 
  }
}


void UARTInit(void){
  // Enable the GPIO Peripheral used by the UART.
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));
  
  // Enable UART0
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0));
  
  // Configure GPIO Pins for UART mode.
  GPIOPinConfigure(GPIO_PA0_U0RX);
  GPIOPinConfigure(GPIO_PA1_U0TX);
  GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
  
  // Initialize the UART for console I/O.
  UARTStdioConfig(0,115200, SystemCoreClock);
} // UARTInit

void UART0_Handler(void){
  UARTStdioIntHandler();
} // UART0_Handler


// ************* Função das Threads do Elevador *************

void elevador(void *arg){
  
  int andar_destino;
  
  struct_elevador e;
  e.andar=0;
  e.status=1; //parado
  e.pos=(char)(arg);
  
  for(int i=0;i<=15;i++){ // esvazia as paradas
    e.paradas[i]=false;
  }

  if(e.pos=='e'){
    e.fila = queue_left_id;
  }
  else if(e.pos=='d'){
    e.fila = queue_right_id;
  }
  else if(e.pos=='c'){
    e.fila = queue_mid_id;
  }
  
  reset(e.pos);
  fechar(e.pos);
  
  while(1){
    
    //osThreadFlagsWait(Flag, osFlagsWaitAny, 500);
    char instr[10];
    osMessageQueueGet(e.fila, instr, NULL, osWaitForever);
    
    if(instr[1]>='0' && instr[1]<='9') // recebe o andar que o elevador está
    {
      e.andar=get_andar(instr[1],instr[2]);
      if(comparar_andar(e.andar, e.paradas))            //andar de destino
      {
        receber(e);
        remover_parada(&e);
        
        if(sem_parada(e.paradas)){           // esvaziaram as paradas?
          parar(e.pos); //fica parado
          e.status=1;
        }
        else                                 // tem mais paradas pra fazer
        {                                           
          if(e.status==2){ // ta subindo
            if(paradas_acima(e)){ // sobe mais
              fechar(e.pos);
              subir(e.pos);
            }
            else{                 // pode inverter o direcao
              e.status=3;
              fechar(e.pos);
              descer(e.pos);
            }
          }
          else if(e.status==3)    // desce mais
          {
            if(paradas_abaixo(e)){
              fechar(e.pos);
              descer(e.pos);
            }
            else{ // pode inverter o direcao
              e.status=2;
              fechar(e.pos);
              subir(e.pos);
            }
          }
        }
      }
      else//false
      {
        if(e.status==2){
          subir(e.pos);
        }
        else if(e.status==3){
          descer(e.pos);
        }
      }
    }
    else if(instr[1]=='E')
    {
      andar_destino = conv_num_andar(instr[2],instr[3]);
      adicionar_parada(&e, andar_destino);
      if(e.status == 1) // Parado
      {
        if(andar_destino == e.andar){
          receber(e);
        }
        if(andar_destino > e.andar){
          fechar(e.pos);
          subir(e.pos);
          e.status=2;
        }
        else{
          fechar(e.pos);
          descer(e.pos);
          e.status=3;
        }
      }
    }
    
    else if(instr[1]=='I'){
      andar_destino=conv_letra_andar(instr[2]);
      adicionar_parada(&e, andar_destino);
      if(e.status==1)                              // Parado
      {
        if(andar_destino>e.andar){
          fechar(e.pos);
          subir(e.pos);
          e.status=2;
        }
        else if(andar_destino<e.andar){
          fechar(e.pos);
          descer(e.pos);
          e.status=3; 
        }
        else{
          receber(e); 
        }
      }
      else if(e.status==2){
        subir(e.pos);
      }
      else if(e.status==3){
        descer(e.pos);
      }
    } 
  } 
}


void main(void){
  
  UARTInit();
  
  SystemInit();
  
  osKernelInitialize();
  
  elevador_left_id = osThreadNew(elevador, (void*)'e', NULL);
  elevador_mid_id = osThreadNew(elevador, (void*)'c', NULL);
  elevador_right_id = osThreadNew(elevador, (void*)'d', NULL);
  
  queue_left_id =  osMessageQueueNew (20, sizeof(char)*6, NULL);
  queue_mid_id =  osMessageQueueNew (20, sizeof(char)*6, NULL); 
  queue_right_id =  osMessageQueueNew (20, sizeof(char)*6, NULL);
  
  UART_Read_id= osThreadNew(UART_Read, NULL, NULL);
  UART_Write_id= osThreadNew(UART_Write, NULL, NULL);
  
  queue_uartwrite_id =  osMessageQueueNew (20, sizeof(char)*4, NULL);
  
  //Mutex_UART = osMutexNew(NULL);
    
  if(osKernelGetState() == osKernelReady)
    osKernelStart();
  
  while(1);
}