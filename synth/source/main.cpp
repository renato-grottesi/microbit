#include "MicroBit.h"

enum NOTE
{
	Z0=0,
	C0, CS0, D0, DS0, E0, F0, FS0, G0, GS0, A0, AS0, B0,
	C1, CS1, D1, DS1, E1, F1, FS1, G1, GS1, A1, AS1, B1,
	C2, CS2, D2, DS2, E2, F2, FS2, G2, GS2, A2, AS2, B2,
	C3, CS3, D3, DS3, E3, F3, FS3, G3, GS3, A3, AS3, B3,
	C4, CS4, D4, DS4, E4, F4, FS4, G4, GS4, A4, AS4, B4,
	C5, CS5, D5, DS5, E5, F5, FS5, G5, GS5, A5, AS5, B5,
	C6, CS6, D6, DS6, E6, F6, FS6, G6, GS6, A6, AS6, B6,
	C7, CS7, D7, DS7, E7, F7, FS7, G7, GS7, A7, AS7, B7,
	C8, CS8, D8, DS8, E8, F8, FS8, G8, GS8, A8, AS8, B8,
} NOTE;

// Note periods
uint16_t periods[] = {
	0, // Silence
	//  C,   C#,    D,   D#,    E,    F,   F#,    G,   G#,    A,   A#,    B,
	61156,57723,54483,51425,48539,45815,43243,40816,38525,36363,34322,32396, // 0
	30578,28861,27241,25712,24269,22907,21621,20408,19262,18181,17161,16198, // 1
	15289,14430,13620,12856,12134,11453,10810,10204, 9631, 9090, 8580, 8099, // 2
	7644, 7215, 6810, 6428, 6067, 5726, 5405, 5102, 4815, 4545, 4290, 4049,  // 3
	3822, 3607, 3405, 3214, 3033, 2863, 2702, 2551, 2407, 2272, 2145, 2024,  // 4
	1911, 1803, 1702, 1607, 1516, 1431, 1351, 1275, 1203, 1136, 1072, 1012,  // 5
	955,  901,  851,  803,  758,  715,  675,  637,  601,  568,  536,  506,   // 6
	477,  450,  425,  401,  379,  357,  337,  318,  300,  284,  268,  253,   // 7
	238,  225,  212,  200,  189,  178,  168,  159,  150,  142,  134,  126,   // 8
};

#include "pirates.h"
#include "mario.h"

MicroBit uBit;

int main()
{
	uint16_t i;
	uBit.init();
	uBit.io.P0.setAnalogValue(200);
	uBit.io.P0.setAnalogPeriodUs(50);
	MicroBitButton buttonA(MICROBIT_PIN_BUTTON_A, MICROBIT_ID_BUTTON_A); 
	MicroBitButton buttonB(MICROBIT_PIN_BUTTON_B, MICROBIT_ID_BUTTON_B);

	// Sawtooth wave
	for(;;)
	{
		for(i=0;i<1024;i=i+10)
		{
			uBit.io.P0.setAnalogValue(i);
			wait_us(100);
		}
		if(buttonA.isPressed()) break;
	}

	for(i=0;i<12*9;i++)
	{
		uBit.io.P0.setAnalogValue(511);
		uBit.io.P0.setAnalogPeriodUs(periods[i]);
		wait_us(250000);
		if(buttonB.isPressed()) break;
	}

	for(i=0;i<sizeof(mario_melody)/sizeof(uint8_t);i++)
	{
		uBit.io.P0.setAnalogValue(511);
		uBit.io.P0.setAnalogPeriodUs(periods[mario_melody[i]]);
		wait_ms(1500/mario_tempo[i]);
		if(buttonB.isPressed()) break;
	}

	for(i=0;i<sizeof(underworld_melody)/sizeof(uint8_t);i++)
	{
		uBit.io.P0.setAnalogValue(511);
		uBit.io.P0.setAnalogPeriodUs(periods[underworld_melody[i]]);
		wait_ms(1500/underworld_tempo[i]);
		if(buttonA.isPressed()) break;
	}

	for(i=0;i<notes_count;i++)
	{
		uBit.io.P0.setAnalogValue(511);
		uBit.io.P0.setAnalogPeriodUs(periods[pirates_melody[i]]);
		wait_ms(pirates_duration[i]);
		if(buttonA.isPressed()) break;
	}

	uBit.io.P0.setAnalogPeriodUs(periods[0]);

	release_fiber();
	return 0;
}

