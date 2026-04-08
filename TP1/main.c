#include <xc.h>
#include <pic12f675.h>
#include <stdint.h>

//Configuracion de FUSES del PIC12F675
#pragma config FOSC=INTRCIO //Int OSC = 4MHz
#pragma config WDTE=ON     //Enable Watchdog Timer
#pragma config PWRTE=OFF    //Disable Power-Up Timer
#pragma config MCLRE=ON     //Enable Master Clear
#pragma config BOREN=OFF    //Disable Blown-ou Detect
#pragma config CP=OFF       //Disable Code Protection
#pragma config CPD=OFF      //Disable Data Code Protection


#define pinTX_Out       GPIObits.GP0
#define pinRX_In        GPIObits.GP1
#define pinBuffer_Out   GPIObits.GP2
#define pinTrigger_Out  GPIObits.GP5
#define pinEcho_In      GPIObits.GP4

#define LEJANA 203000       //350cm
#define INTERMEDIA 11600    //200cm
#define CERCANA 5800        //100cm 
#define CRITICA 4350        //75cm



void main (void){
    unsigned uint16_t tiempo;
    unsigned uint16_t distancia;
    Init();
    
    while(1){
        SerialIn();
        pinTrigger_Out = 0;
        __delay_us(2);
        pinTrigger_Out = 1;
        __delay_us(10);
        pinTrigger_Out = 0;
        tiempo = Contador();
        Alerta(tiempo);
        distancia = tiempo / 58;
        
        uart_tx_string("Dist: ");
        uart_tx_num(distancia);
        uart_tx_string(" cm\r\n");

        __delay_ms(60);    
    }
}

void Init(void){
    CMCONbits.CM = 0b111;     //Deactivamos comparadores
    TRISIO = 0b00010010;             //GP0 como salida, en realidad todo el puerto
    T1CON = 0b00000001;
    pinTX_Out  = 0;
    pinRX_In   = 0;
    pinBuffer_Out  = 0;
    pinTrigger_Out  = 0;
    pinEcho_In      = 0; 
}

unsigned uint16_t Contador(void){
    uint16_t tiempo_us;
    
    while(!pinEcho_In);
    
    TMR1H = 0;
    TMR1L = 0;
    
    while(pinEcho_In);
    
    tiempo_us = ((uint16_t)TMR1H << 8) | TMR1L;

    return tiempo_us;
}



void SerialOut(unsigned uint16_t *distancia){
    unsigned char mensaje[9];
    for(int i=0; i < 10; i++){
        if(distancia[i]==255){
        mensaje[i] = ;
        }
    }
}

void SerialIn(void){
    if(pinRX_In == 1){
        pinTX_Out  = 0;
        pinRX_In   = 0;
        pinBuffer_Out  = 0;
        pinTrigger_Out  = 0;
        pinEcho_In      = 0; 
    }
}

void Alerta(unsigned uint16_t tmp){
    if (tmp >= LEJANA) {
        
        pinBuffer_Out  = 0;
        
    } else if (tmp >= INTERMEDIA) {
        
        pinBuffer_Out  = 1;
        __delay_ms(30);
        pinBuffer_Out  = 0;
        
    } else if (tmp >= CERCANA) {
        
        pinBuffer_Out  = 1;
        __delay_ms(3);
        pinBuffer_Out  = 0;
        
    } else if (tmp <= CRITICA){
        
        pinBuffer_Out  = 1;
    } 
}

void uart_tx_byte(uint8_t data){
    // Start bit
    TX = 0;
    __delay_us(104);

    // 8 bits de datos
    for (uint8_t i = 0; i < 8; i++)
    {
        TX = (data >> i) & 1;
        __delay_us(104);
    }

    // Stop bit
    TX = 1;
    __delay_us(104);
}

void uart_tx_string(const char *str){
    while(*str)
    {
        uart_tx_byte(*str++);
    }
}

void uart_tx_num(uint16_t num){
    char buffer[6];
    uint8_t i = 0;

    if (num == 0)
    {
        uart_tx_byte('0');
        return;
    }

    while(num > 0)
    {
        buffer[i++] = (num % 10) + '0';
        num /= 10;
    }

    while(i > 0)
    {
        uart_tx_byte(buffer[--i]);
    }
}
