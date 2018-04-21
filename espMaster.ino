#include <Wire.h>
#include <myWifi.h>

#define CMD_RESET	0	// turn it all off
#define CMD_SIZE	1	// actual number of LEDS
#define CMD_SETALL	2	// set all leds to RGB
#define CMD_SETONE	3	// set a single led - offset(0) RGB
#define CMD_SHIFT	4	// shift current set - signed byte (for L and R) RGB replace
#define CMD_DISPLAY	6	// shunt out to the LEDS - beware, interrupts get cleared, so I2C will fail
#define CMD_INVERT	7	// invert all rgbs

#define NUM_LEDS	15

//#define SCANI2C

#define _AT85_ADDR	0x10

#define FULL_BRIGHT		15
#define EXTRA_BRIGHT	31

class ATleds
{
protected:

	int m_addr;

public:
	ATleds(int addr) :m_addr(addr)
	{
	};

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

	bool SetOne(byte offset, byte r, byte g, byte b)
	{
		byte data[] = { CMD_SETONE,offset, r,g,b };
		return SendData(&data[0], sizeof(data));
	}

	bool WipeRight(byte r, byte g, byte b, byte step=1)
	{
		byte data[] = { CMD_SHIFT,step, r,g,b };
		return SendData(&data[0], sizeof(data));
	}

	bool WipeLeft(byte r, byte g, byte b, byte step = 1)
	{
		byte data[] = { CMD_SHIFT,(byte)(signed char)(-(signed char)step), r,g,b };
		return SendData(&data[0], sizeof(data));
	}


	bool Clear()
	{
		byte data[] = { CMD_RESET };
		return SendData(&data[0], sizeof(data));
	}

	void DisplayAndWait()
	{
		byte data[] = { CMD_DISPLAY };
		bool ret = SendData(&data[0], sizeof(data), true);
	}

	bool Invert(byte mask)
	{
		byte data[] = { CMD_INVERT,mask };
		return SendData(&data[0], sizeof(data));
	}

protected:

	bool SendData(byte *data, unsigned size, bool waitIfDisplayed=false)
	{
		Wire.beginTransmission(m_addr);
		for (unsigned each = 0; each<size; each++)
			Wire.write(data[each]);
		byte error = Wire.endTransmission();
		if (error == 4)
		{
			return false;
		}
		// we suffer because the at turns off interrupts when it shunts to LED
		// so - take breath 
		if(waitIfDisplayed)
			delay(50);
		waitForSpace(waitIfDisplayed);
		return true;
	}

	// flushed means wait until Display has run really
	void waitForSpace(bool flushed=1)
	{
		do {
			byte ack;
			while (!(ack=Wire.requestFrom(m_addr, 1))) 
			{
				Serial.printf(".", ack);
				//delayMicroseconds(50);
				delay(1);
			}
			//Serial.printf("=%d ", ack);
			ack = Wire.read();
			//Serial.printf("%c%03d ", (ack & 128) ? '+' : '!', ack & 127);
			if (flushed)
			{
				if (ack == 0x80)
					break;
			}
			if (ack & 0x80)
				break;
			delayMicroseconds(50);
		} while (true);
		//Serial.println();

	}

};

ATleds leds(_AT85_ADDR);

#define COMMAND_DELAY	1000
#define WIPE_DELAY		200
#define ERROR_DELAY		1000

void setup()
{
	Serial.begin(9600);
	delay(2000);

	myWifiClass::TurnOff();

	Wire.begin();
	Serial.println("LED toy");
	delay(2000);

	// set the size
	// tell it how many leds it has
	Serial.println("SIZE");
	if (!leds.SetSize(NUM_LEDS))
	{
		Serial.println("err size");
		delay(ERROR_DELAY);
	}
}


#define _TEST_ONE
#define _TEST_ALL
#define _TEST_WIPE
#define _TEST_INVERT_

void loop()
{

#ifdef SCANI2C

	byte address;
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
			if (address<16)
				Serial.print("0");
			Serial.print(address, HEX);
			Serial.println("  !");

			nDevices++;
		}
		else if (error == 4)
		{
			Serial.print("Unknown error at address 0x");
			if (address<16)
				Serial.print("0");
			Serial.println(address, HEX);
		}
	}
	if (nDevices == 0)
		Serial.println("No I2C devices found\n");
	else
		Serial.println("done\n");

	delay(2000);

#else

	Serial.println("starting ...");

#ifdef _TEST_ALL
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

#ifdef _TEST_ONE

	Serial.println("SETONE YELLOW");
	for (unsigned each = 0; each < NUM_LEDS; each ++ )
	{
		if(!leds.SetOne(each, FULL_BRIGHT, FULL_BRIGHT,0))
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

#endif

#ifdef _TEST_WIPE

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

#if defined (_TEST_INVERT_) && defined(_TEST_WIPE)

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

#ifdef _TEST_ONE
	// loops
	Serial.println("LOOPS");
	for (unsigned each = 0; each < NUM_LEDS; each++)
	{
		leds.Clear();
		if(each>0)
			leds.SetOne(each-1, FULL_BRIGHT, 0, 0);
		leds.SetOne(each, EXTRA_BRIGHT, FULL_BRIGHT, 0);
		leds.SetOne(each+1, FULL_BRIGHT, 0, 0);

		leds.DisplayAndWait();
		delay(WIPE_DELAY);
	}
#endif

#ifdef _TEST_INVERT_

	leds.SetAll(FULL_BRIGHT, 0, 0);
	leds.DisplayAndWait();
	delay(WIPE_DELAY);
	leds.Invert(15);
	leds.DisplayAndWait();
	delay(WIPE_DELAY);

#endif

	Serial.println("RESET");

	if(!leds.Clear())
	{
		Serial.println("err");
		delay(ERROR_DELAY);
		return;

	}

	
	Serial.println("DONE");

	delay(COMMAND_DELAY);



#endif

	

}