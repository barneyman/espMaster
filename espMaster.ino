#include <Wire.h>
#include <ESP8266WiFi.h>

//#define _XSISTOR_FOR_ON


#define CMD_RESET	0	// turn it all off
#define CMD_SIZE	1	// actual number of LEDS
#define CMD_SETALL	2	// set all leds to RGB
#define CMD_SETONE	3	// set a single led - offset(0) RGB
#define CMD_SHIFT	4	// shift current set - signed byte (for L and R) RGB replace
#define CMD_DISPLAY	6	// shunt out to the LEDS - beware, interrupts get cleared, so I2C will fail
#define CMD_INVERT	7	// invert all rgbs
#ifdef _XSISTOR_FOR_ON
#define CMD_ON		8	// + on/off byte
#define CMD_OFF		9	// + on/off byte
#endif
#define CMD_SETALL_PALETTE		10	// set all leds to RGB
#define CMD_SETONE_PALETTE		11	// set a single led - offset(0) RGB
#define CMD_SHIFT_PALETTE		12	// shift current set - signed byte (for L and R) RGB replace

//#define NUM_LEDS	90
#define NUM_LEDS	15


#define _AT85_ADDR	0x10

#define FULL_BRIGHT		16
#define EXTRA_BRIGHT	31

// time to write to a ws2821b
// (1.5us * 8 * 3 * NUM_LEDS)+50us 
// lets go with 100us * NUM_LEDS
// .1ms * NUM_LEDS

//#define COMMAND_DELAY		1000
//#define WIPE_DELAY			50
//#define ERROR_DELAY			1000
//#define DISPLAY_DELAY		(int)(NUM_LEDS/1)
//#define REQUESTFROM_DELAY	10

#define COMMAND_DELAY		5000
#define WIPE_DELAY			0
#define ERROR_DELAY			1000
#define DISPLAY_DELAY		50
//#define REQUESTFROM_DELAY	5


//#define _USE_OLED
#ifdef _USE_OLED

// https://github.com/ThingPulse/esp8266-oled-ssd1306.git
//#include <SSD1306Wire.h>
//SSD1306Wire display(0x3c, SDA, SCL);

// https://github.com/datacute/Tiny4kOLED.git
#include <Tiny4kOLED.h>
#endif


class ATleds
{
protected:

	int m_addr;

public:
	ATleds(int addr) :m_addr(addr), successCount(0)
	{
	};

#ifdef _XSISTOR_FOR_ON
	bool On()
	{
		byte data[] = { CMD_ON };
		return SendData(&data[0], sizeof(data));
	}

	bool Off()
	{
		byte data[] = { CMD_OFF };
		return SendData(&data[0], sizeof(data));
	}
#endif

	bool SetSize(unsigned size)
	{
		byte data[] = { CMD_SIZE, (byte)size };
		return SendData(&data[0], sizeof(data));
	}

	bool SetAll(byte r, byte g, byte b)
	{
		byte data[] = { CMD_SETALL, r,g,b };
		return SendData(&data[0], sizeof(data));
	}

	bool SetAllPalette(byte colour)
	{
		byte data[] = { CMD_SETALL_PALETTE, colour };
		return SendData(&data[0], sizeof(data));
	}

	bool SetOne(byte offset, byte r, byte g, byte b)
	{
		byte data[] = { CMD_SETONE,offset, r,g,b };
		return SendData(&data[0], sizeof(data));
	}

	bool SetOnePalette(byte offset, byte colour)
	{
		byte data[] = { CMD_SETONE_PALETTE,offset, colour };
		return SendData(&data[0], sizeof(data));
	}


	bool WipeRight(byte r, byte g, byte b, byte step=1)
	{
		byte data[] = { CMD_SHIFT,step, r,g,b };
		return SendData(&data[0], sizeof(data));
	}

	bool WipeRightPalette(byte colour, byte step = 1)
	{
		byte data[] = { CMD_SHIFT_PALETTE,step, colour };
		return SendData(&data[0], sizeof(data));
	}


	bool WipeLeft(byte r, byte g, byte b, byte step = 1)
	{
		return WipeRight(r, g, b, -step);
	}

	bool WipeLeftPalette(byte colour, byte step = 1)
	{
		return WipeRightPalette(colour, -step);
	}

	bool Clear()
	{
		byte data[] = { CMD_RESET };
		return SendData(&data[0], sizeof(data));
	}

	void DisplayAndWait()
	{
		// wait until the queue is flushed, so we KNOW we're the only outstanding command
		byte data[] = { CMD_DISPLAY };
		bool ret = SendData(&data[0], sizeof(data), true);
		if (!ret)
		{
			Serial.printf("DisplayAndWait failed\n\r");
		}
	}

	bool Invert(byte mask)
	{
		byte data[] = { CMD_INVERT,mask };
		return SendData(&data[0], sizeof(data));
	}

protected:

	unsigned successCount;

	bool SendData(byte *data, unsigned size, bool waitIfDisplayed=false)
	{
		Wire.beginTransmission(m_addr);
		for (unsigned each = 0; each<size; each++)
		{
			int sent = Wire.write(data[each]);
			if ( sent!= 1)
				Serial.printf("err on write %d\n\r", sent);
		}
		byte error = Wire.endTransmission();
		if (error != I2C_OK)
		{
			Serial.printf("err on endTransmission %d (successCount %d) status %d\n\r", error, successCount, Wire.status());
			successCount = 0;
			return false;
		}
		successCount++;

		// we suffer because the at turns off interrupts when it shunts to LED
		// so - take a breath 
		if (waitIfDisplayed)
		{
			delay(DISPLAY_DELAY);
		}

		waitForSpace(waitIfDisplayed);

		return true;
	}

	// flushed means wait until Display has run really
	void waitForSpace(bool waitTilEmpty=true)
	{
		yield();
#ifndef REQUESTFROM_DELAY
		delay(5);
		return;
#else
		// we've just been sending, give the slave some breathing room
		do {
			delay(REQUESTFROM_DELAY);
			byte ack;
			while (!(ack=Wire.requestFrom(m_addr, 1))) 
			{
				// we got no reply from the slave 
				Serial.printf("%03d ", Wire.status());
				
				delay(ERROR_DELAY);
			}
			ack = Wire.read();
			if (waitTilEmpty)
			{
				if (ack & 0x40)
					break;
			}
			else if (ack & 0xC0)
			{
				break;
			}
			else
			{
				Serial.printf("ack %02x\n\r",ack);
			}
		} while (true);
#endif
	}

};

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

	Wire.begin();
	Wire.setClockStretchLimit(3000);

	Serial.println("LED toy");
	delay(2000);

	i2cscan();

	// set the size
	// tell it how many leds it has
	Serial.println("SIZE");
	if (!leds.SetSize(NUM_LEDS))
	{
		Serial.println("err size");
		delay(ERROR_DELAY);
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


	// try some palette commands :)
	Serial.println("RED");
	leds.SetOnePalette(0, 2);
	leds.DisplayAndWait();
	delay(COMMAND_DELAY);

	Serial.println("BLUE");
	leds.SetOnePalette(1, 4);
	leds.DisplayAndWait();
	delay(COMMAND_DELAY);

	Serial.println("GREEN");
	leds.SetOnePalette(2, 3);
	leds.DisplayAndWait();
	delay(COMMAND_DELAY);


	for (int roll = 0; roll < NUM_LEDS; roll++)
	{
		leds.WipeRightPalette(0);
		leds.DisplayAndWait();
		delay(WIPE_DELAY);
	}

	Serial.println("RESET");
	leds.Clear();
	leds.DisplayAndWait();
	delay(COMMAND_DELAY);


	Serial.println("RED");
	leds.SetAllPalette(2);
	leds.DisplayAndWait();
	delay(COMMAND_DELAY);

	Serial.println("BLUE");
	leds.SetAllPalette(4);
	leds.DisplayAndWait();
	delay(COMMAND_DELAY);

	Serial.println("WHITE");
	leds.SetAllPalette(1);
	leds.DisplayAndWait();
	delay(COMMAND_DELAY);


	Serial.println("RESET");
	leds.Clear();
	leds.DisplayAndWait();
	delay(COMMAND_DELAY);



#if defined( _TEST_ALL ) || defined(_TEST_EVERYTHING)
	Serial.println("RED");
	// all red
	if(!leds.SetAll(FULL_BRIGHT,0,0))
	{
		Serial.println("err");
		delay(ERROR_DELAY);
		return;
	}

	leds.DisplayAndWait();

	delay(COMMAND_DELAY);

	Serial.println("GREEN");
	// all green
	if (!leds.SetAll(0, FULL_BRIGHT, 0))
	{
		Serial.println("err");
		delay(ERROR_DELAY);
		return;

	}
	leds.DisplayAndWait();
	delay(COMMAND_DELAY);

	Serial.println("BLUE");
	// all blue
	if(!leds.SetAll(0, 0, FULL_BRIGHT))
	{
		Serial.println("err");
		delay(ERROR_DELAY);
		return;

	}
	leds.DisplayAndWait();
	delay(COMMAND_DELAY);

	// all white
	Serial.println("WHITE");
	if (!leds.SetAll(FULL_BRIGHT, FULL_BRIGHT, FULL_BRIGHT))
	{
		Serial.println("err");
		delay(ERROR_DELAY);
		return;

	}
	leds.DisplayAndWait();
	delay(COMMAND_DELAY);

#endif


#if defined(_TEST_ONE) || defined(_TEST_EVERYTHING)

	// all white
	Serial.println("WHITE");
	if (!leds.SetAll(FULL_BRIGHT, FULL_BRIGHT, FULL_BRIGHT))
	{
		Serial.println("err");
		delay(ERROR_DELAY);
		return;

	}
	leds.DisplayAndWait();
	delay(COMMAND_DELAY);

	Serial.println("SETONE BLACK");
	for (unsigned each = 0; each < NUM_LEDS; each++)
	{
		if (!leds.SetOne(each, 0, 0, 0))
		{
			Serial.println("err");
			delay(ERROR_DELAY);
			return;
		}
		leds.DisplayAndWait();
		delay(WIPE_DELAY);
	}
	delay(COMMAND_DELAY);


#endif

#if defined(_TEST_WIPE) || defined(_TEST_EVERYTHING)

	// idx0 green
	Serial.println("ALT CYAN");
	for(int alt=0;alt<NUM_LEDS;alt+=3)
	{
		if (!leds.SetOne(alt, 0, FULL_BRIGHT, FULL_BRIGHT))
		{
			Serial.println("err");
			delay(ERROR_DELAY);
			return;
		}
		leds.DisplayAndWait();
		delay(WIPE_DELAY);
	}
	leds.DisplayAndWait();
	delay(COMMAND_DELAY);

#if defined (_TEST_INVERT_) && defined(_TEST_EVERYTHING)

	leds.Invert(FULL_BRIGHT);
	leds.DisplayAndWait();
	delay(COMMAND_DELAY);

	leds.Invert(FULL_BRIGHT);
	leds.DisplayAndWait();
	delay(COMMAND_DELAY);

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
		delay(WIPE_DELAY);
	}
	leds.DisplayAndWait();
	delay(COMMAND_DELAY);

	// idx0 green
	Serial.println("ALT CYAN");
	for (int alt = 0; alt<NUM_LEDS; alt += 3)
	{
		if (!leds.SetOne(alt, 0, FULL_BRIGHT, FULL_BRIGHT))
		{
			Serial.println("err");
			delay(ERROR_DELAY);
			return;
		}
		//leds.DisplayAndWait();
		//delay(WIPE_DELAY);
	}
	leds.DisplayAndWait();
	delay(COMMAND_DELAY);


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
		delay(WIPE_DELAY);
	}
	leds.DisplayAndWait();
	delay(COMMAND_DELAY);
#endif

#if defined(_TEST_ONE) || defined(_TEST_EVERYTHING)

	Serial.println("SETONE YELLOW");
	for (unsigned each = 0; each < NUM_LEDS; each++)
	{
		if (!leds.SetOne(each, FULL_BRIGHT, FULL_BRIGHT, 0))
		{
			Serial.println("err");
			delay(ERROR_DELAY);
			return;

		}
		leds.DisplayAndWait();
		delay(WIPE_DELAY);

	}
	delay(COMMAND_DELAY);

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
		delay(WIPE_DELAY);
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
				delay(WIPE_DELAY);
			}
#endif

#ifdef _XSISTOR_FOR_ON
	leds.On();
#endif

#if defined(_TEST_INVERT_) || defined(_TEST_EVERYTHING)

	Serial.println("INVERT");
	leds.SetAll(FULL_BRIGHT, 0, 0);
	leds.DisplayAndWait();
	delay(WIPE_DELAY);
	leds.Invert(15);
	leds.DisplayAndWait();
	delay(WIPE_DELAY);
	delay(COMMAND_DELAY);

#endif

#if defined(_TEST_RESET) || defined(_TEST_EVERYTHING)

	Serial.println("RESET");

	if (!leds.Clear())
	{
		Serial.println("err");
		delay(ERROR_DELAY);
		return;
	}

	leds.DisplayAndWait();
	delay(COMMAND_DELAY);

#endif


	
	Serial.println("DONE");

#ifdef _XSISTOR_FOR_ON	
	leds.Off();
#endif


	delay(2000);
	

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