#include <xc.h>
#include <stdint.h>

//Configuracion de FUSES del PIC12F675
#pragma config FOSC=INTRCIO //Int OSC = 4MHz
#pragma config WDTE=OFF     //Enable Watchdog Timer
#pragma config PWRTE=OFF    //Disable Power-Up Timer
#pragma config MCLRE=ON     //Enable Master Clear
#pragma config BOREN=OFF    //Disable Blown-ou Detect
#pragma config CP=OFF       //Disable Code Protection
#pragma config CPD=OFF      //Disable Data Code Protection

#define _XTAL_FREQ 4000000  // 4 MHz

#define pinTX_Out       GPIObits.GP0
#define pinRX_In        GPIObits.GP1
#define pinBuffer_Out   GPIObits.GP2
#define pinTrigger_Out  GPIObits.GP4
#define pinEcho_In      GPIObits.GP5

#define LEJANA 150        
#define INTERMEDIA 100    
#define CERCANA 70         
#define CRITICA 30       



void Init(void){
    CMCONbits.CM = 0b111;     //Deactivamos comparadores
    TRISIO = 0b00100010;             //GP0 como salida, en realidad todo el puerto
    T1CON = 0b00000001;             //Activamos TIMER!
    pinTX_Out  = 0;
    pinRX_In   = 0;
    pinBuffer_Out  = 0;
    pinTrigger_Out  = 0;
    pinEcho_In      = 0; 
}

uint16_t Contador(void){
    uint32_t timeout = 300*58;

    // esperar subida
    while(!pinEcho_In && timeout--);
    if(timeout == 0) return 0;

    TMR1H = 0;
    TMR1L = 0;

    timeout = 300*58;

    // esperar bajada
    while(pinEcho_In && timeout--);
    if(timeout == 0) return 0;

    return ((uint16_t)TMR1H << 8) | TMR1L;
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

uint16_t Alerta(uint16_t tmp){
    if (tmp >= LEJANA) {
        
        pinBuffer_Out  = 0;
        
        return 0;
    }  
    
    if (tmp <= LEJANA && tmp >= INTERMEDIA) {
        
        pinBuffer_Out  = 1;
        __delay_ms(60);
        pinBuffer_Out  = 0;
        
        return 1;
    } 
    
    if (tmp <= INTERMEDIA && tmp >= CERCANA) {
        
        pinBuffer_Out  = 1;
        __delay_ms(30);
        pinBuffer_Out  = 0;
        
        return 2;
    }
    
    if ( tmp <= CERCANA && tmp >= CRITICA){
        
        pinBuffer_Out  = 1;
        __delay_ms(20);
        pinBuffer_Out  = 0;
        
        return 3;
    }
    
    if ( tmp <= CRITICA && tmp > 0){
        
        pinBuffer_Out  = 1;

        return 0;
    }
}

void uart_tx_byte(uint8_t data){
    // Start bit
    pinTX_Out = 0;
    __delay_us(833);

    // 8 bits de datos
    for (uint8_t i = 0; i < 8; i++)
    {
        pinTX_Out = (data >> i) & 1;
        __delay_us(833);
    }

    // Stop bit
    pinTX_Out = 1;
    __delay_us(833);
}

void uart_tx_string(const char *str){
    while(*str)
    {
        uart_tx_byte(*str++);
    }
}

void uart_tx_num(uint16_t num){
    char buffer[7];
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

void Disparo (void){
    pinTrigger_Out = 0;
    __delay_us(2);
    pinTrigger_Out = 1;
    __delay_us(10);
    pinTrigger_Out = 0;
}

uint16_t mediana3(uint16_t a, uint16_t b, uint16_t c)
{
    a = a / 58;
    b = b / 58;
    c = c / 58;
    
    if ((a>=b && a<=c)||(a>=c && a<=b)) return a;
    if ((b>=a && b<=c)||(b>=c && b<=a)) return b;
    return c;
}

uint16_t filtro_prom(uint16_t nueva)
{
    static uint16_t f = 0;
    f = (f*3 + nueva) / 4;
    return f;
}


void main (void){
    uint16_t tiempo[3];
    uint16_t distancia, dist_f;
    uint16_t status;
    
    Init();
    
    while(1){
       // SerialIn();
        Disparo();
        tiempo[0] = Contador();
        Disparo();
        tiempo[1] = Contador();
        Disparo();
        tiempo[2] = Contador();
        
        distancia = mediana3(tiempo[0],tiempo[1],tiempo[2]);
        dist_f = filtro_prom(distancia);
        
        status = Alerta(dist_f);
        
        uart_tx_string("Dist: ");
        uart_tx_num(dist_f);
        uart_tx_string(" cm\n\r");

        switch(status){
            case 0: 
                __delay_ms(100);
                break;
            case 1:
                __delay_ms(40);
                break;
            case 2:
                __delay_ms(70);
                break;
            case 3:
                __delay_ms(80);
                break;
            default:
                __delay_ms(100);
                break;
        }
    }
}