#include <Wire.h>
#include <myWifi.h>

#define CMD_RESET	0	// turn it all off
#define CMD_SIZE	1	// actual number of LEDS
#define CMD_SETALL	2	// set all leds to RGB
#define CMD_SETONE	3	// set a single led - offset(0) RGB
#define CMD_SHIFT	4	// shift current set - signed byte (for L and R) RGB replace

#define NUM_LEDS	15

//#define SCANI2C



void setup()
{
	Serial.begin(9600);
	delay(2000);

	myWifiClass::TurnOff();

	Wire.begin();
	Serial.println("LED toy");
	delay(2000);

#ifndef SCANI2C

/*
	// tell it how many leds it has
	Wire.beginTransmission(0x10);
	Wire.write((byte)CMD_SIZE);
	Wire.write((byte)NUM_LEDS);
	byte error = Wire.endTransmission();
	if (error == 4)
	{
		Serial.println("err size");
		delay(1000);
	}
	delay(1000);


	Wire.beginTransmission(0x10);
	Wire.write((byte)CMD_RESET);
	error = Wire.endTransmission();
	if (error == 4)
	{
		Serial.println("err reset");
		delay(1000);
	}

	*/

#endif

}


void loop()
{
	byte error;
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


	// tell it how many leds it has
	Wire.beginTransmission(0x10);
	Wire.write((byte)CMD_SIZE);
	Wire.write((byte)NUM_LEDS);
	error = Wire.endTransmission();
	if (error == 4)
	{
		Serial.println("err size");
		delay(1000);
	}
	delay(1000);


	Serial.println("RED");
	// all red
	Wire.beginTransmission(0x10);
	Wire.write((byte)CMD_SETALL);
	if (!Wire.write((byte)16))
		Serial.println("err 16");
	if (!Wire.write((byte)0))
		Serial.println("err 0 1");
	if (!Wire.write((byte)0))
		Serial.println("err 0 2");
	error=Wire.endTransmission();
	if (error == 4)
	{
		Serial.println("err");
		delay(1000);
		return;
	}
	delay(500);

	Serial.println("GREEN");
	// all green
	Wire.beginTransmission(0x10);
	Wire.write((byte)CMD_SETALL);
	Wire.write((byte)0);
	Wire.write((byte)16);
	Wire.write((byte)0);
	error = Wire.endTransmission();
	if (error == 4)
	{
		Serial.println("err");
		return;

	}
	delay(500);

	Serial.println("BLUE");
	// all blue
	Wire.beginTransmission(0x10);
	Wire.write((byte)CMD_SETALL);
	Wire.write((byte)0);
	Wire.write((byte)0);
	Wire.write((byte)16);
	error = Wire.endTransmission();
	if (error == 4)
	{
		Serial.println("err");
		return;

	}
	delay(500);

	// all white
	Serial.println("WHITE");
	Wire.beginTransmission(0x10);
	Wire.write((byte)CMD_SETALL);
	Wire.write((byte)16);
	Wire.write((byte)16);
	Wire.write((byte)16);
	error = Wire.endTransmission();
	if (error == 4)
	{
		Serial.println("err");
		return;

	}

	delay(500);

	Serial.println("SETONE YELLOW");
	for (unsigned each = 0; each < NUM_LEDS; each ++ )
	{
		Wire.beginTransmission(0x10);
		Wire.write((byte)CMD_SETONE);
		Wire.write((byte)each);
		Wire.write((byte)16);
		Wire.write((byte)16);
		Wire.write((byte)0);
		error = Wire.endTransmission();
		if (error == 4)
		{
			Serial.println("err");
			return;

		}
		delay(100);

	}

	// idx0 green
	for(int alt=0;alt<NUM_LEDS;alt+=2)
	{
		Wire.beginTransmission(0x10);
		Wire.write((byte)CMD_SETONE);
		Wire.write((byte)alt);
		Wire.write((byte)0);
		Wire.write((byte)16);
		Wire.write((byte)16);
		error = Wire.endTransmission();
		delay(200);
	}
	if (error == 4)
	{
		Serial.println("err");
		return;

	}

#ifdef DOSHIFT

	// shift right, replace red
	Serial.println("SHIFT RIGHT");
	for (unsigned each = 0; each < 5; each++)
	{
		Wire.beginTransmission(0x10);
		Wire.write((byte)CMD_SHIFT);
		Wire.write((byte)1);
		Wire.write((byte)16);
		Wire.write((byte)0);
		Wire.write((byte)0);
		error = Wire.endTransmission(false);
		if (error == 4)
		{
			Serial.println("err");
			return;

		}
		delay(500);
	}

	// shift left - replace blue
	Serial.println("SHIFT RIGHT");
	for (unsigned each = 0; each < 5; each++)
	{
		Wire.beginTransmission(0x10);
		Wire.write((byte)CMD_SHIFT);
		Wire.write((byte)1);
		Wire.write((byte)0);
		Wire.write((byte)0);
		Wire.write((byte)0);
		error = Wire.endTransmission(false);
		if (error == 4)
		{
			Serial.println("err");
			return;

		}
		delay(500);
	}

	Serial.println("RESET");

	Wire.beginTransmission(0x10);
	Wire.write((byte)CMD_RESET);
	error = Wire.endTransmission();
	if (error == 4)
	{
		Serial.println("err");
		return;

	}
#endif
	Serial.println("DONE");

	delay(500);


#endif

	

}