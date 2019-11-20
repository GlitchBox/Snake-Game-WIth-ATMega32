
#ifndef F_CPU
#define F_CPU 1000000 // 16 MHz clock speed
#endif

#define D0 eS_PORTD0
#define D1 eS_PORTD1
#define D2 eS_PORTD2
#define D3 eS_PORTD3
#define D4 eS_PORTD4
#define D5 eS_PORTD5
#define D6 eS_PORTD6
#define D7 eS_PORTD7
#define RS eS_PORTC6
#define EN eS_PORTC7

#include <avr/io.h>
#include <util/delay.h>
#include "lcd.h"
int main(void)
{
	DDRD = 0xFF;
	DDRC = 0xFF;
	DDRB = 0xff;           // Configure PORTB as output

	
	int i;
	Lcd8_Init();

	while(1)
	{
		Lcd8_Set_Cursor(1,1);
		PORTB = 0xff;        // Turn ON the Buzzer conneted to PORTB
		Lcd8_Write_String("Buzzer On");
		_delay_ms(2000);      // Wait for some time
		Lcd8_Clear();
		PORTB = 0x00;        // Turn OFF the Buzzer connected to PORTB
		
		Lcd8_Write_String("Buzzer Off");
		_delay_ms(2000);
		Lcd8_Clear();
		//_delay_ms(5000);
		/*for(i=0;i<15;i++)
		{
			_delay_ms(500);
			Lcd8_Shift_Left();
		}
		
		for(i=0;i<15;i++)
		{
			_delay_ms(500);
			Lcd8_Shift_Right();
		}
		*/
		//Lcd8_Clear();
		//Lcd8_Write_Char('e');
		//Lcd8_Write_Char('S');
		//_delay_ms(2000);
	}

}

