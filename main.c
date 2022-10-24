#include <mega328p.h>
#include <delay.h>
//________________________________________________
#define la PORTD.6
#define lb PORTB.0
#define lc PORTB.1
#define ld PORTB.2
#define le PORTB.3
#define lf PORTB.4
#define lg PORTD.5
//________________________________________________
unsigned int kqADC6;
unsigned char demHienthi;
unsigned char VTA,dienapVTA;//don vi 0.1V
//________________________________________________
void hienthiLed(unsigned char so)
{
    switch (so)
    {
    case(0):
        la=1;lb=1;lc=1;ld=1;le=1;lf=1;lg=0;
        break;
    case(1):
        la=0;lb=1;lc=1;ld=0;le=0;lf=0;lg=0;
        break;
    case(2):
        la=1;lb=1;lg=1;le=1;ld=1;lc=0;lf=0;
        break;
    case(3):
        la=1;lb=1;lc=1;ld=1;le=0;lf=0;lg=1;
        break;
    case(4):
        lf=1;lg=1;lb=1;lc=1;la=0;ld=0;le=0;
        break;
    case(5):
        la=1;lf=1;lg=1;lc=1;ld=1;lb=0;le=0;
        break;
    case(6):
        la=1;lc=1;ld=1;le=1;lf=1;lg=1;lb=0;
        break;
    case(7):
        la=1;lb=1;lc=1;ld=0;le=0;lf=0;lg=0;
        break;
    case(8):
        la=1;lb=1;lc=1;ld=1;le=1;lf=1;lg=1;
        break;
    case(9):
        la=1;lb=1;lc=1;ld=1;le=0;lf=1;lg=1;
        break;
    case(16):
        la=0;lb=0;lc=0;ld=0;le=0;lf=0;lg=0;
        break;
    }
}
//--------------------UART----------------------
#define DATA_REGISTER_EMPTY (1<<UDRE0)
#define RX_COMPLETE (1<<RXC0)
#define FRAMING_ERROR (1<<FE0)
#define PARITY_ERROR (1<<UPE0)
#define DATA_OVERRUN (1<<DOR0)

// USART Receiver buffer
#define RX_BUFFER_SIZE0 16
char rx_buffer0[RX_BUFFER_SIZE0];

#if RX_BUFFER_SIZE0 <= 256
unsigned char rx_wr_index0=0,rx_rd_index0=0;
#else
unsigned int rx_wr_index0=0,rx_rd_index0=0;
#endif

#if RX_BUFFER_SIZE0 < 256
unsigned char rx_counter0=0;
#else
unsigned int rx_counter0=0;
#endif

// This flag is set on USART Receiver buffer overflow
bit rx_buffer_overflow0;

// USART Receiver interrupt service routine
interrupt [USART_RXC] void usart_rx_isr(void)
{
char status,data;
status=UCSR0A;
data=UDR0;
if ((status & (FRAMING_ERROR | PARITY_ERROR | DATA_OVERRUN))==0)
   {
   rx_buffer0[rx_wr_index0++]=data;
#if RX_BUFFER_SIZE0 == 256
   // special case for receiver buffer size=256
   if (++rx_counter0 == 0) rx_buffer_overflow0=1;
#else
   if (rx_wr_index0 == RX_BUFFER_SIZE0) rx_wr_index0=0;
   if (++rx_counter0 == RX_BUFFER_SIZE0)
      {
      rx_counter0=0;
      rx_buffer_overflow0=1;
      }
#endif
   }
}

#ifndef _DEBUG_TERMINAL_IO_
// Get a character from the USART Receiver buffer
#define _ALTERNATE_GETCHAR_
#pragma used+
char getchar(void)
{
char data;
while (rx_counter0==0);
data=rx_buffer0[rx_rd_index0++];
#if RX_BUFFER_SIZE0 != 256
if (rx_rd_index0 == RX_BUFFER_SIZE0) rx_rd_index0=0;
#endif
#asm("cli")
--rx_counter0;
#asm("sei")
return data;
}
#pragma used-
#endif

// USART Transmitter buffer
#define TX_BUFFER_SIZE0 16
char tx_buffer0[TX_BUFFER_SIZE0];

#if TX_BUFFER_SIZE0 <= 256
unsigned char tx_wr_index0=0,tx_rd_index0=0;
#else
unsigned int tx_wr_index0=0,tx_rd_index0=0;
#endif

#if TX_BUFFER_SIZE0 < 256
unsigned char tx_counter0=0;
#else
unsigned int tx_counter0=0;
#endif

// USART Transmitter interrupt service routine
interrupt [USART_TXC] void usart_tx_isr(void)
{
if (tx_counter0)
   {
   --tx_counter0;
   UDR0=tx_buffer0[tx_rd_index0++];
#if TX_BUFFER_SIZE0 != 256
   if (tx_rd_index0 == TX_BUFFER_SIZE0) tx_rd_index0=0;
#endif
   }
}

#ifndef _DEBUG_TERMINAL_IO_
// Write a character to the USART Transmitter buffer
#define _ALTERNATE_PUTCHAR_
#pragma used+
void putchar(char c)
{
while (tx_counter0 == TX_BUFFER_SIZE0);
#asm("cli")
if (tx_counter0 || ((UCSR0A & DATA_REGISTER_EMPTY)==0))
   {
   tx_buffer0[tx_wr_index0++]=c;
#if TX_BUFFER_SIZE0 != 256
   if (tx_wr_index0 == TX_BUFFER_SIZE0) tx_wr_index0=0;
#endif
   ++tx_counter0;
   }
else
   UDR0=c;
#asm("sei")
}
#pragma used-
#endif

// Standard Input/Output functions
#include <stdio.h>
void enter(void){
    putchar(13);putchar(10);

}
void gui1byte(unsigned char so){
   putchar(so/100 +48);
   putchar(so/10%10 +48);
   putchar(so%10 +48);
}
void gui1byte_1le(unsigned char so){
   putchar(so/100 +48);
   putchar(so/10%10 +48);
   putchar('.');
   putchar(so%10 +48);
}
void gui2byte(unsigned int so){

   putchar(so/10000 +48);
   putchar(so/1000%10 +48);
   putchar(so/100%10 +48);
   putchar(so/10%10 +48);
   putchar(so%10 +48);
}

//-----------------ket thuc UART----------------
//------------------ADC-------------------------

// Voltage Reference: AVCC pin
#define ADC_VREF_TYPE ((0<<REFS1) | (1<<REFS0) | (0<<ADLAR))

// Read the AD conversion result
unsigned int read_adc(unsigned char adc_input)
{
ADMUX=adc_input | ADC_VREF_TYPE;
// Delay needed for the stabilization of the ADC input voltage
delay_us(10);
// Start the AD conversion
ADCSRA|=(1<<ADSC);
// Wait for the AD conversion to complete
while ((ADCSRA & (1<<ADIF))==0);
ADCSRA|=(1<<ADIF);
return ADCW;
}
//-----------------ket thuc ADC--------------------
void main(void)
{

#pragma optsize-
CLKPR=(1<<CLKPCE);
CLKPR=(0<<CLKPCE) | (0<<CLKPS3) | (0<<CLKPS2) | (0<<CLKPS1) | (0<<CLKPS0);
#ifdef _OPTIMIZE_SIZE_
#pragma optsize+
#endif

// USART initialization
// Communication Parameters: 8 Data, 1 Stop, No Parity
// USART Receiver: On
// USART Transmitter: On
// USART0 Mode: Asynchronous
// USART Baud Rate: 9600
UCSR0A=(0<<RXC0) | (0<<TXC0) | (0<<UDRE0) | (0<<FE0) | (0<<DOR0) | (0<<UPE0) | (0<<U2X0) | (0<<MPCM0);
UCSR0B=(1<<RXCIE0) | (1<<TXCIE0) | (0<<UDRIE0) | (1<<RXEN0) | (1<<TXEN0) | (0<<UCSZ02) | (0<<RXB80) | (0<<TXB80);
UCSR0C=(0<<UMSEL01) | (0<<UMSEL00) | (0<<UPM01) | (0<<UPM00) | (0<<USBS0) | (1<<UCSZ01) | (1<<UCSZ00) | (0<<UCPOL0);
UBRR0H=0x00;
UBRR0L=0x67;
//--------------------------------------------------------------------------------------
// ADC initialization
// ADC Clock frequency: 1000.000 kHz
// ADC Voltage Reference: AVCC pin
// ADC Auto Trigger Source: ADC Stopped
// Digital input buffers on ADC0: On, ADC1: On, ADC2: On, ADC3: On
// ADC4: On, ADC5: On
DIDR0=(0<<ADC5D) | (0<<ADC4D) | (0<<ADC3D) | (0<<ADC2D) | (0<<ADC1D) | (0<<ADC0D);
ADMUX=ADC_VREF_TYPE;
ADCSRA=(1<<ADEN) | (0<<ADSC) | (0<<ADATE) | (0<<ADIF) | (0<<ADIE) | (1<<ADPS2) | (0<<ADPS1) | (0<<ADPS0);
ADCSRB=(0<<ADTS2) | (0<<ADTS1) | (0<<ADTS0);

//------------------------------------------------------------------------------------------
// Global enable interrupts
#asm("sei")

// Set up register
DDRD = 0b01100000;
DDRB  = 0b00111111;

while (1){
    // Read ADC6
    kqADC6 = read_adc(6);
    if(++demHienthi >=  20){
        demHienthi = 0;
        // Fault 1
        if(kqADC6 > 920){
            putsf("loi dut mass ");
            hienthiLed(0);
        }
	// Fault 2
        else if(kqADC6 < 102){
            putsf("loi dut duong ");
            hienthiLed(8);
        }
	// Not Fault
        else{
            VTA =(unsigned long int)(kqADC6-102)*100/818;
            putsf("VTA = ");gui1byte(VTA); putsf(" %");
            hienthiLed(16);
        }
        dienapVTA = (unsigned long int)kqADC6*50/1023;//don vi 0.1V
        putsf(" U = ");gui1byte_1le(dienapVTA); putsf(" V");enter();
    }
//--------------------------------------------------------------------------
    delay_ms(10);
}
}





