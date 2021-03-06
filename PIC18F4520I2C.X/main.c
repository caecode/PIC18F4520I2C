/*
 * File:   main.c
 * Author: 
 *
 * Created on August 2, 2018, 2:03 PM
 */

// PIC18F4520 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1H
#pragma config OSC = INTIO67    // Oscillator Selection bits (Internal oscillator block, port function on RA6 and RA7)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor disabled)
#pragma config IESO = OFF       // Internal/External Oscillator Switchover bit (Oscillator Switchover mode disabled)

// CONFIG2L
#pragma config PWRT = OFF       // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = SBORDIS  // Brown-out Reset Enable bits (Brown-out Reset enabled in hardware only (SBOREN is disabled))
#pragma config BORV = 3         // Brown Out Reset Voltage bits (Minimum setting)

// CONFIG2H
#pragma config WDT = ON         // Watchdog Timer Enable bit (WDT enabled)
#pragma config WDTPS = 32768    // Watchdog Timer Postscale Select bits (1:32768)

// CONFIG3H
#pragma config CCP2MX = PORTC   // CCP2 MUX bit (CCP2 input/output is multiplexed with RC1)
#pragma config PBADEN = ON      // PORTB A/D Enable bit (PORTB<4:0> pins are configured as analog input channels on Reset)
#pragma config LPT1OSC = OFF    // Low-Power Timer1 Oscillator Enable bit (Timer1 configured for higher power operation)
#pragma config MCLRE = ON       // MCLR Pin Enable bit (MCLR pin enabled; RE3 input pin disabled)

// CONFIG4L
#pragma config STVREN = ON      // Stack Full/Underflow Reset Enable bit (Stack full/underflow will cause Reset)
#pragma config LVP = ON         // Single-Supply ICSP Enable bit (Single-Supply ICSP enabled)
#pragma config XINST = OFF      // Extended Instruction Set Enable bit (Instruction set extension and Indexed Addressing mode disabled (Legacy mode))

// CONFIG5L
#pragma config CP0 = OFF        // Code Protection bit (Block 0 (000800-001FFFh) not code-protected)
#pragma config CP1 = OFF        // Code Protection bit (Block 1 (002000-003FFFh) not code-protected)
#pragma config CP2 = OFF        // Code Protection bit (Block 2 (004000-005FFFh) not code-protected)
#pragma config CP3 = OFF        // Code Protection bit (Block 3 (006000-007FFFh) not code-protected)

// CONFIG5H
#pragma config CPB = OFF        // Boot Block Code Protection bit (Boot block (000000-0007FFh) not code-protected)
#pragma config CPD = OFF        // Data EEPROM Code Protection bit (Data EEPROM not code-protected)

// CONFIG6L
#pragma config WRT0 = OFF       // Write Protection bit (Block 0 (000800-001FFFh) not write-protected)
#pragma config WRT1 = OFF       // Write Protection bit (Block 1 (002000-003FFFh) not write-protected)
#pragma config WRT2 = OFF       // Write Protection bit (Block 2 (004000-005FFFh) not write-protected)
#pragma config WRT3 = OFF       // Write Protection bit (Block 3 (006000-007FFFh) not write-protected)

// CONFIG6H
#pragma config WRTC = OFF       // Configuration Register Write Protection bit (Configuration registers (300000-3000FFh) not write-protected)
#pragma config WRTB = OFF       // Boot Block Write Protection bit (Boot block (000000-0007FFh) not write-protected)
#pragma config WRTD = OFF       // Data EEPROM Write Protection bit (Data EEPROM not write-protected)

// CONFIG7L
#pragma config EBTR0 = OFF      // Table Read Protection bit (Block 0 (000800-001FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR1 = OFF      // Table Read Protection bit (Block 1 (002000-003FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR2 = OFF      // Table Read Protection bit (Block 2 (004000-005FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR3 = OFF      // Table Read Protection bit (Block 3 (006000-007FFFh) not protected from table reads executed in other blocks)

// CONFIG7H
#pragma config EBTRB = OFF      // Boot Block Table Read Protection bit (Boot block (000000-0007FFh) not protected from table reads executed in other blocks)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>

#define _XTAL_FREQ 4000000

void write(char uControlByte, char uHighAddress, char uLowAddress, char uData);
char read(char uHighAddress, char uLowAddress);
void transmissionInProgress(void);

void main(void) {
    
    TRISD=0x00;
    
    LATD=0x00;
    
    //Set the pins directions
    TRISCbits.RC3=1;   //SCL
    TRISCbits.RC4=1;   //SDA
    
    SSPSTAT = 0x80;   //Disable SMBus & Slew Rate Control
    SSPCON1 = 0x28;   //Enable MSSP Master
    SSPADD = 0x19;    //Should be 0x19 for 100kHz
    SSPCON2 = 0x00;   //Clear MSSP Conrol Bits
 
    //write to I2C.High Address=0x00, Low Address=0x00 . Data=0x07
    write(0xA0, 0x00, 0x00,0x07);

    //wait some time
    __delay_ms(10);

    //Read I2C. High Address=0x00, Low Address=0x00
    LATD=read(0x00,0x00);
    
    while(1);

}

void write(char uControlByte, char uHighAddress, char uLowAddress, char uData){
    
    //Check if I2C is idle
    while ((SSPSTAT & 0x04) || (SSPCON2 & 0x1F)); 
    
    //Generate a Start condition by setting the Start Enable bit
    SSPCON2bits.SEN=1;
    
    transmissionInProgress();
    
    //Send control byte
    SSPBUF=uControlByte;
    
    transmissionInProgress();
    
    //send High Address
    SSPBUF=uHighAddress;
    
    transmissionInProgress();
    
    //send Low Address
    SSPBUF=uLowAddress;
    
   transmissionInProgress();
    
    //send data to store in EEPROM
    SSPBUF=uData;
    
    transmissionInProgress();
    
    //generate a stop condition
    SSPCON2bits.PEN=1;
    
    while (SSPSTATbits.P);
    
}

char read(char uHighAddress, char uLowAddress){

    char myData=0x0;
    
    while ((SSPSTAT & 0x04) || (SSPCON2 & 0x1F)); //Transmit is in progress
    //Generate a Start condition by setting the Start Enable bit
    SSPCON2bits.SEN=1;
    
    transmissionInProgress();
    
    //Send control byte
    SSPBUF=0xA0;
    
    transmissionInProgress();
    
    //Send high address
    SSPBUF=uHighAddress;
    
    transmissionInProgress();
    
    //Send low address
    SSPBUF=uLowAddress;
    
    transmissionInProgress();
    
    //Generate a Re-Start condition
    SSPCON2bits.RSEN=1;
    
    while ( SSPCON2bits.RSEN );     // wait until re-start condition is over 
    
    //Send control byte
    SSPBUF=0xA1;
    
    transmissionInProgress();
    
    //To read the data byte, the ACKDT bit is first set to indicate that a NO ACK should be sent.
    SSPCON2bits.ACKDT=1;
    
    //Then the RCEN bit is set to initialize the read and the data can be copied from SSPBUF
    SSPCON2bits.RCEN=1;
    
    transmissionInProgress();
    
    //read data from EEPROM
    myData=SSPBUF;
    
    //The Master must respond back with a NO ACT bit. This indicates that no more data will be read
    while ( SSPCON2bits.ACKEN );
    
    //Finally the master generates a STOP condition

    SSPCON2bits.PEN=1;
    
    while (SSPSTATbits.P);
        
    return myData;
}

void transmissionInProgress(void){
    
    while ((SSPSTAT & 0x04) || (SSPCON2 & 0x1F)); //Transmit is in progress
    
    while(!PIR1bits.SSPIF);
    
    PIR1bits.SSPIF=0;
    
}