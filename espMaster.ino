#include <Wire.h>
#include <ESP8266WiFi.h>

//#define _XSISTOR_FOR_ON

//#define NUM_LEDS	90
#define NUM_LEDS	15


#define _AT85_ADDR	0x10

#define FULL_BRIGHT		16
#define EXTRA_BRIGHT	31



//#define _USE_OLED
#ifdef _USE_OLED

// https://github.com/ThingPulse/esp8266-oled-ssd1306.git
//#include <SSD1306Wire.h>
//SSD1306Wire display(0x3c, SDA, SCL);

// https://github.com/datacute/Tiny4kOLED.git
#include <Tiny4kOLED.h>
#endif

#include <atLEDS.h>


ATleds leds(_AT85_ADDR);


#ifdef _USE_OLED
SSD1306Device oled;
#endif

void setup()
{
	Serial.begin(9600);
	delay(5000);

	WiFi.mode(WIFI_OFF);
	while (WiFi.getMode() != WIFI_OFF)
		delay(1);

	Serial.println("LED toy");
	delay(2000);

	leds.begin();
	Wire.setClockStretchLimit(3000);


	i2cscan();

	// set the size
	// tell it how many leds it has
	Serial.println("SIZE");
	if (!leds.SetSize(NUM_LEDS))
	{
		Serial.println("err size");
		delay(_ATLEDS_ERROR_DELAY);
	}

#ifdef _USE_OLED

	oled.begin();
	oled.clear();
	oled.on();
	oled.switchRenderFrame();
	oled.setFont(FONT6X8);

	// oled 
	//display.init();
	//display.drawString(0, 0, "wibble");
	//display.display();

#endif

}

//#define _TEST_EVERYTHING
// or
//#define _TEST_ALL
//#define _TEST_ONE
//#define _TEST_WIPE
//#define _TEST_INVERT_
//#define _TEST_LONG_CHAIN
//#define _TEST_LOOPS
//#define _TEST_PALETTE
//#define _TEST_USER_PALETTE
#define _TESTING_MACRO

bool doOnce = false;

void loop()
{
	Serial.println("starting ...");


#ifdef _USE_OLED

	oled.fill(255);
	oled.switchFrame();
	delay(2000);

	oled.clear();
	//oled.setCursor(0, 0);
	oled.print("ticktock");
	oled.switchFrame();

	//display.clear();
	//display.drawString(0, 0, "wibble");
	//display.display();
	delay(2000);
	return;

#endif

#ifdef _XSISTOR_FOR_ON
	leds.On();
#endif

#ifdef _TESTING_MACRO

	byte macro[] = {
		CMD_RESET,
		CMD_SETALL_PALETTE, _COLOR_PALLETE_LIME,
		CMD_SETONE_PALETTE, 0, _COLOR_PALLETE_CYAN,
		CMD_SETONE_PALETTE, 14, _COLOR_PALLETE_CYAN,
		CMD_DISPLAY,
		CMD_DELAY_MACRO, 1,
		CMD_SETALL_PALETTE, _COLOR_PALLETE_RED,
		CMD_DISPLAY,
		CMD_DELAY_MACRO, 1,
		CMD_RESET
		// macro parser always adds a CMD_DISPLAY as the last instruction
	};

	if (!doOnce)
	{
		if (!leds.SetMacro(macro, sizeof(macro)))
		{
			Serial.println("failed to set macro");

			leds.SetOnePalette(0, _COLOR_PALLETE_RED);
			leds.DisplayAndWait();

		}
		else
		{
#ifdef _TESTING_MACRO_RUN
			delay(_ATLEDS_COMMAND_DELAY);

			for (int each = 0; each < 50; each++)
			{
				if (!leds.RunMacro())
				{
					Serial.println("failed to run macro");
				}
				delay(_ATLEDS_COMMAND_DELAY);
			}

			delay(_ATLEDS_COMMAND_DELAY * 5);
#endif
		}
		doOnce = true;
	}
	else
	{
		//Serial.println("clearing");
		//leds.Clear();
		//leds.DisplayAndWait();
		delay(_ATLEDS_COMMAND_DELAY * 5);
	}

#endif

#ifdef _TEST_USER_PALETTE

	leds.SetUserPalette(_COLOR_PALLETE_USER1, 128, 128, 0);
	leds.SetUserPalette(_COLOR_PALLETE_USER2, 0, 128, 128);
	leds.SetUserPalette(_COLOR_PALLETE_USER3, 128, 0, 128);

	leds.SetAllPalette(16);
	leds.DisplayAndWait();
	delay(_ATLEDS_COMMAND_DELAY);

	leds.SetAllPalette(17);
	leds.DisplayAndWait();
	delay(_ATLEDS_COMMAND_DELAY);

	leds.SetAllPalette(18);
	leds.DisplayAndWait();
	delay(_ATLEDS_COMMAND_DELAY);

#endif

#ifdef _TEST_PALETTE

	for (unsigned div = 0; div < 7; div++)
	{
		leds.SetPaletteDiv(div);

		// try some palette commands :)
		Serial.println("RED");
		leds.SetOnePalette(0, _COLOR_PALLETE_RED);
		leds.DisplayAndWait();
		delay(_ATLEDS_COMMAND_DELAY);

		Serial.println("BLUE");
		leds.SetOnePalette(1, _COLOR_PALLETE_BLUE);
		leds.DisplayAndWait();
		delay(_ATLEDS_COMMAND_DELAY);

		Serial.println("GREEN");
		leds.SetOnePalette(2, _COLOR_PALLETE_LIME);
		leds.DisplayAndWait();
		delay(_ATLEDS_COMMAND_DELAY);

	
		for (int roll = 0; roll < NUM_LEDS; roll++)
		{
			leds.WipeRightPalette(0);
			leds.DisplayAndWait();
			delay(_ATLEDS_WIPE_DELAY);
		}

		Serial.println("RESET");
		leds.Clear();
		leds.DisplayAndWait();
		delay(_ATLEDS_COMMAND_DELAY);


		Serial.println("RED");
		leds.SetAllPalette(_COLOR_PALLETE_RED);
		leds.DisplayAndWait();
		delay(_ATLEDS_COMMAND_DELAY);

		Serial.println("BLUE");
		leds.SetAllPalette(_COLOR_PALLETE_BLUE);
		leds.DisplayAndWait();
		delay(_ATLEDS_COMMAND_DELAY);

		Serial.println("GREY");
		leds.SetAllPalette(_COLOR_PALLETE_GREY);
		leds.DisplayAndWait();
		delay(_ATLEDS_COMMAND_DELAY);

	}


	leds.SetAllPalette(_COLOR_PALLETE_GREY);
	for (int eeek = 0; eeek < 256; eeek++)
	{
		leds.SetPaletteDiv(eeek & 7);
		leds.DisplayAndWait();
		delay(_ATLEDS_WIPE_DELAY);
	}

	Serial.println("RESET");
	leds.Clear();
	leds.DisplayAndWait();
	delay(_ATLEDS_COMMAND_DELAY);

#endif

#if defined( _TEST_ALL ) || defined(_TEST_EVERYTHING)
	Serial.println("RED");
	// all red
	if(!leds.SetAll(FULL_BRIGHT,0,0))
	{
		Serial.println("err");
		delay(_ATLEDS_ERROR_DELAY);
		return;
	}

	leds.DisplayAndWait();

	delay(_ATLEDS_COMMAND_DELAY);

	Serial.println("GREEN");
	// all green
	if (!leds.SetAll(0, FULL_BRIGHT, 0))
	{
		Serial.println("err");
		delay(_ATLEDS_ERROR_DELAY);
		return;

	}
	leds.DisplayAndWait();
	delay(_ATLEDS_COMMAND_DELAY);

	Serial.println("BLUE");
	// all blue
	if(!leds.SetAll(0, 0, FULL_BRIGHT))
	{
		Serial.println("err");
		delay(_ATLEDS_ERROR_DELAY);
		return;

	}
	leds.DisplayAndWait();
	delay(_ATLEDS_COMMAND_DELAY);

	// all white
	Serial.println("WHITE");
	if (!leds.SetAll(FULL_BRIGHT, FULL_BRIGHT, FULL_BRIGHT))
	{
		Serial.println("err");
		delay(_ATLEDS_ERROR_DELAY);
		return;

	}
	leds.DisplayAndWait();
	delay(_ATLEDS_COMMAND_DELAY);

#endif


#if defined(_TEST_ONE) || defined(_TEST_EVERYTHING)

	// all white
	Serial.println("WHITE");
	if (!leds.SetAll(FULL_BRIGHT, FULL_BRIGHT, FULL_BRIGHT))
	{
		Serial.println("err");
		delay(_ATLEDS_ERROR_DELAY);
		return;

	}
	leds.DisplayAndWait();
	delay(_ATLEDS_COMMAND_DELAY);

	Serial.println("SETONE BLACK");
	for (unsigned each = 0; each < NUM_LEDS; each++)
	{
		if (!leds.SetOne(each, 0, 0, 0))
		{
			Serial.println("err");
			delay(_ATLEDS_ERROR_DELAY);
			return;
		}
		leds.DisplayAndWait();
		delay(_ATLEDS_WIPE_DELAY);
	}
	delay(_ATLEDS_COMMAND_DELAY);


#endif

#if defined(_TEST_WIPE) || defined(_TEST_EVERYTHING)

	// idx0 green
	Serial.println("ALT CYAN");
	for(int alt=0;alt<NUM_LEDS;alt+=3)
	{
		if (!leds.SetOne(alt, 0, FULL_BRIGHT, FULL_BRIGHT))
		{
			Serial.println("err");
			delay(_ATLEDS_ERROR_DELAY);
			return;
		}
		leds.DisplayAndWait();
		delay(_ATLEDS_WIPE_DELAY);
	}
	leds.DisplayAndWait();
	delay(_ATLEDS_COMMAND_DELAY);

#if defined (_TEST_INVERT_) && defined(_TEST_EVERYTHING)

	leds.Invert(FULL_BRIGHT);
	leds.DisplayAndWait();
	delay(_ATLEDS_COMMAND_DELAY);

	leds.Invert(FULL_BRIGHT);
	leds.DisplayAndWait();
	delay(_ATLEDS_COMMAND_DELAY);

#endif 



	// shift right, replace red
	Serial.println("SHIFT RIGHT");
	for (unsigned each = 0; each < (NUM_LEDS); each++)
	{
		if(!leds.WipeRight(FULL_BRIGHT,0,0))
		{
			Serial.println("err");
			return;

		}
		leds.DisplayAndWait();
		delay(_ATLEDS_WIPE_DELAY);
	}
	leds.DisplayAndWait();
	delay(_ATLEDS_COMMAND_DELAY);

	// idx0 green
	Serial.println("ALT CYAN");
	for (int alt = 0; alt<NUM_LEDS; alt += 3)
	{
		if (!leds.SetOne(alt, 0, FULL_BRIGHT, FULL_BRIGHT))
		{
			Serial.println("err");
			delay(_ATLEDS_ERROR_DELAY);
			return;
		}
		//leds.DisplayAndWait();
		//delay(_ATLEDS_WIPE_DELAY);
	}
	leds.DisplayAndWait();
	delay(_ATLEDS_COMMAND_DELAY);


	// shift left - replace blue
	Serial.println("SHIFT LEFT");
	for (unsigned each = 0; each < NUM_LEDS; each++)
	{
		if(!leds.WipeLeft(FULL_BRIGHT, FULL_BRIGHT, FULL_BRIGHT))
		{
			Serial.println("err");
			return;

		}
		leds.DisplayAndWait();
		delay(_ATLEDS_WIPE_DELAY);
	}
	leds.DisplayAndWait();
	delay(_ATLEDS_COMMAND_DELAY);
#endif

#if defined(_TEST_ONE) || defined(_TEST_EVERYTHING)

	Serial.println("SETONE YELLOW");
	for (unsigned each = 0; each < NUM_LEDS; each++)
	{
		if (!leds.SetOne(each, FULL_BRIGHT, FULL_BRIGHT, 0))
		{
			Serial.println("err");
			delay(_ATLEDS_ERROR_DELAY);
			return;

		}
		leds.DisplayAndWait();
		delay(_ATLEDS_WIPE_DELAY);

	}
	delay(_ATLEDS_COMMAND_DELAY);

#endif

#ifdef _XSISTOR_FOR_ON
	leds.Off();
#endif

#if defined(_TEST_LOOPS) || defined(_TEST_EVERYTHING)
	// loops
	Serial.println("LOOPS");
	for (unsigned each = 0; each < NUM_LEDS; each++)
	{
		leds.Clear();
		//if(each>0)
		//	leds.SetOne(each-1, FULL_BRIGHT, 0, 0);
		leds.SetOne(each, EXTRA_BRIGHT, FULL_BRIGHT, 0);
		//if(each<NUM_LEDS)
		//	leds.SetOne(each+1, FULL_BRIGHT, 0, 0);

		leds.DisplayAndWait();
		delay(_ATLEDS_WIPE_DELAY);
	}


#endif



#if defined(_TEST_LOOPS) || defined(_TEST_EVERYTHING)
	Serial.println("LOOP COLORS");
	for (unsigned r = 0; r < FULL_BRIGHT; r+=4)
		for (unsigned g = 0; g < FULL_BRIGHT; g+= 4)
			for (unsigned b = 0; b < FULL_BRIGHT; b+= 4)
			{
				leds.SetAll(r, g, b);
				leds.DisplayAndWait();
				delay(_ATLEDS_WIPE_DELAY);
			}
#endif

#ifdef _XSISTOR_FOR_ON
	leds.On();
#endif

#if defined(_TEST_INVERT_) || defined(_TEST_EVERYTHING)

	Serial.println("INVERT");
	leds.SetAll(FULL_BRIGHT, 0, 0);
	leds.DisplayAndWait();
	delay(_ATLEDS_WIPE_DELAY);
	leds.Invert(15);
	leds.DisplayAndWait();
	delay(_ATLEDS_WIPE_DELAY);
	delay(_ATLEDS_COMMAND_DELAY);

#endif

#if defined(_TEST_RESET) || defined(_TEST_EVERYTHING)

	Serial.println("RESET");

	if (!leds.Clear())
	{
		Serial.println("err");
		delay(_ATLEDS_ERROR_DELAY);
		return;
	}

	leds.DisplayAndWait();
	delay(_ATLEDS_COMMAND_DELAY);

#endif


	
	Serial.println("DONE");

#ifdef _XSISTOR_FOR_ON	
	leds.Off();
#endif


	delay(_ATLEDS_COMMAND_DELAY);
	

}



////

void i2cscan()
{
	byte error, address;
	int nDevices;

	Serial.println("Scanning...");

	nDevices = 0;
	for (address = 1; address < 127; address++)
	{
		// The i2c_scanner uses the return value of
		// the Write.endTransmisstion to see if
		// a device did acknowledge to the address.
		Wire.beginTransmission(address);
		error = Wire.endTransmission();

		if (error == 0)
		{
			Serial.print("I2C device found at address 0x");
			if (address < 16)
				Serial.print("0");
			Serial.print(address, HEX);
			Serial.println("  !");

			nDevices++;
		}
		else if (error == 4)
		{
			Serial.print("Unknown error at address 0x");
			if (address < 16)
				Serial.print("0");
			Serial.println(address, HEX);
		}
	}
	if (nDevices == 0)
		Serial.println("No I2C devices found\n");
	else
		Serial.println("done\n");
}