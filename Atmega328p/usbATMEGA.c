/********************************************************************************************************/
/********************************************************************************************************/
/***********************************TUTORAT SYSTEMES IMA*************************************************/
/***********************************Amaury KNOCKAERT - Thomas CUNIN**************************************/
/***********************************PROGRAMME ATMEGA238P*************************************************/
/********************************************************************************************************/
/********************************************************************************************************/
/********************************************************************************************************/
/********************************************************************************************************/
#include <avr/io.h>	
#include <util/delay.h>
#include <avr/interrupt.h>


// For the serial port

#define CPU_FREQ        16000000L       // Assume a CPU frequency of 16Mhz
#define NBRBOUTONS	32
#define SERIAL_SPEED	9600


/*Codage des touches :
D3 (droite) - 2
D4 (haut) - 4
D5 (bas) - 8
D6 (gauche)- 0
bouton Joystick - 1
*/

volatile uint8_t message = 0x00;	
void init_serial(int speed)
{
/* Set baud rate */
UBRR0 = CPU_FREQ/(((unsigned long int)speed)<<4)-1;

/* Enable transmitter & receiver */ //on active les interruptions sur réception avec RXCIE0
UCSR0B = (1<<RXCIE0 | 1<<TXEN0 | 1<<RXEN0);

/* Set 8 bits character and 1 stop bit */
UCSR0C = (1<<UCSZ01 | 1<<UCSZ00);

/* Set off UART baud doubler */
UCSR0A &= ~(1 << U2X0);
}

void send_serial(unsigned char c)
{
loop_until_bit_is_set(UCSR0A, UDRE0);
UDR0 = c;
}


void input_init(void){
DDRD &= 0x83;  // PIN 2 as input
PORTD |= 0xff;//0x7C; // Pull-up activated on PIN 2
}


void majLed() 
{

	uint8_t rx = UDR0;
	if(rx == 48)	//rx = '0', la led s'éteint
		PORTB = 0x00;
	if(rx == 49)	//rx = '1', la led s'allume
		PORTB = 0x20;
}

ISR(PCINT2_vect) //PCINT 19 20 21 22
{
	message = ~PIND;  //On récupère l'état de PIND et on l'inverse 
}


ISR(USART_RX_vect) //interruption à la réception série
{
	majLed();

}


int main(void)
{

	input_init();
	init_serial(SERIAL_SPEED);		

	PCMSK2 = 0b01111100; //On active l'interruption sur la réception du port série
	PCICR = 0x04; //active PCIE2
	sei();
	uint8_t envoi;
	DDRB = 0xFF;
	PORTB = 0x00;

	while(1)
	{	
		if(message !=0x00)
		{
			envoi = ((message) >> 2) | 0x30; //modifie la valeur pour pouvoir afficher quelque chose de lisible sur minicom
			send_serial(envoi);
			_delay_ms(100);	//100 ms de delay pour éviter des problèmes avec le port série
			if((PIND & message) !=0) //vérifie si l'on a bien relaché le bouton
			message = 0x00; //si c'est le cas remet le message à 0 pour arreter l'envoi
			/*on travaille donc moitié par interruption (quand on appuie sur le bouton) et moitié par scrutation (pour detecter 
			quand on relache le bouton )*/
		}
	}
	cli();
	return 0;
}
