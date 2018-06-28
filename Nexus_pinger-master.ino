/******************************************************************************************
  Copyright 2017 Ideetron B.V.

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************************/
/****************************************************************************************
  File:     Nexus_pinger.ino
  Author:   Adri Verhoef & Gerben den Hartog for the original Nexus Sketch
  Company:	Ideetron B.V.
  Website:  http://www.ideetron.nl/LoRa
  E-mail:   info@ideetron.nl
  Created on:         13-09-2017
  Supported Hardware: ID150119-02 Nexus board with RFM95

  Description

  This firmware demonstrates a LoRaWAN pinger which send a message each minute.
****************************************************************************************/

/*
*****************************************************************************************
  INCLUDE FILES
*****************************************************************************************
*/
#include <avr/power.h>
#include <avr/sleep.h>
#include <stdbool.h>


#include <SPI.h>
#include <Wire.h>
#include <EEPROM.h>


#include "AES-128.h"
#include "Encrypt.h"
#include "Nexus_LoRaWAN.h"
#include "RFM95.h"
#include "LoRaMAC.h"
#include "timers.h"
#include "DS2401.h"
#include "lorawan_def.h"
#include "I2C.h"
#include "mcp7940.h"
#include "th06.h"
#include "spi_flash.h"
#include "nexus_demoboard.h"
//#include "Cayenne_LPP.h"


/******************************************************************************************
									GLOBAL VARIABLES
******************************************************************************************/

// Interrupt variables.
uint16_t  rainCounter; //make variable 
volatile bool buttonPressed; //make volitile 
volatile bool	RTC_ALARM;
int16_t val; //make a int16 valriable


sAPP		app;		//	Application variables
sLoRaWAN	lora;		//	See the Nexus_Lorawan.h file for the settings of this structure.
sTimeDate	TimeDate;	//	RTC time and date variables

// Initialize the LoRaWAN stack.
LORAMAC lorawan (&lora);

// Initialize the Low Power Protocol functions
//CayenneLPP LPP(&(lora.TX));


/******************************************************************************************
									INTERRUPTS
******************************************************************************************/
/*
  @Brief  Interrupt vector for the alarm of the MCP7940 Real Time Clock.
*/
ISR(INT1_vect)
{
  // Set the boolean to true to indicate that the RTC alarm has occurred. Do not use I2C functions or long delays here, but handle that in the main loop.
  RTC_ALARM = true;
}

ISR(PCINT1_vect) //the interrupt for when the button is pressed
{

  buttonPressed = true; //change to false if you want to do a pull down circuit
  


}

/*
  @Brief  Interrupt vector for Timer1 which is used to time the Join and Receive windows for timeslot 1 and timelsot 2
*/
ISR(TIMER1_COMPA_vect)
{
  // Increment the timeslot counter variable for timing the JOIN and receive delays.
  lora.timeslot++;
}




/******************************************************************************************
									MAIN
******************************************************************************************/
void setup(void)
{

  //Initialize the UART on 9600 baud 8N1
  Serial.begin(9600);//Start serial monitor on baud rate 9600
  Serial.println("Setup Started");//just part of the de-bugger
  ////Initialize the SPI port
  SPI.begin();
  SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));

  //Initialize I/O pins for the RFM95, DS2401 and MCP
  pinMode(A6,				OUTPUT);
  pinMode(A7,				OUTPUT);
  pinMode(DS2401, 		OUTPUT);
  pinMode(MOVEMENT,		OUTPUT);
  pinMode(RTC_MFP, 		INPUT);
  pinMode(RFM_DIO0, 		INPUT);
  pinMode(RFM_DIO1, 		INPUT);
  pinMode(RFM_DIO5, 		INPUT);
  pinMode(RFM_DIO2, 		INPUT);
  pinMode(RFM_NSS,		OUTPUT);
  pinMode(SPI_FLASH_CS,	OUTPUT);
  pinMode(LED,  			OUTPUT);

  digitalWrite(DS2401,		LOW);
  digitalWrite(SPI_FLASH_CS,	HIGH);
  digitalWrite(MOVEMENT,		HIGH);

  // Disable Pin change interrupts
  PCMSK1 = 0;//mask

  // Disable Pin Change Interrupts
  PCICR = 0;//interrupt with button press

  // Power on delay for the RFM module
  delay(3000);

  // Initialize the I2C bus
  I2C_init();

  lorawan.init();

  flash_power_down();

  sei();// Enable Global Interrupts
}



void loop()
{
  Serial.println("Void Loop Started"); // for de-bugger
  /*
  	Get unique ID from the DS2401 and check the CRC. If the CRC matches, print the contents to the serial output. The unique ID is used
  	as Device EUI to generate a unique ID for each Nexus Board. The following code can be remove and the Device EUI can be set hard coded
  	in the lora structure in lorawan_def.h if a different Dev EUI will be used.
  */
  while (DS_Read(&(lora.OTAA.DevEUI[0])) == false)
  {}
  printStringAndHex("DS2401 DEV EUI: ", &(lora.OTAA.DevEUI[0]), 8);

  // Connect to the back-end with OTAA when OTAA is selected as the activation method.
  lorawan.OTAA_connect();

  // Initialize the RTC to generate an interrupt on the first minute roll-over from the RTC.
  mcp7940_init(&TimeDate, app.LoRaWAN_message_interval);

  // Set the alarm to off
  RTC_ALARM = false;
  app.Counter = 0; //app counter
  buttonPressed = false; // Make variable false
  rainCounter = 0; //make variable for counting the rain
  demoboard_enable_button_wakeup(); // call function

  // Super loop
  while (1)
  {
     //check if button is pressed and resets it 
    if (buttonPressed == true && (digitalRead(BUTTON1) == LOW) )//button one defined in nexus_demoboard {
      rainCounter++; 
      delay(50);
      buttonPressed = false;
    }


    // Catch the minute alarm from the RTC.
    if (RTC_ALARM == true)
    {
      // Clear the boolean.
      RTC_ALARM = false;

      // Provide some user feedback with the LED to indicate that a message is send
      digitalWrite(LED, HIGH);

      // Reconfigure the RTC for an interrupt on the next minute.
      mcp7940_reset_minute_alarm(app.LoRaWAN_message_interval);

      // Read the RTC's time and date then print it to the terminal.
      mcp7940_read_time_and_date(&TimeDate);

      // Print a message to the serial port
      Serial.println();
      mcp7940_print(&TimeDate);

      // Print a text string
      Serial.print("Ping: "); // for the debugger
      Serial.println(app.Counter, DEC);

      // Form a payload according to the LPP standard to
      //LPP.clearBuffer();
      //LPP.addDigitalOutput(0x00, app.Counter);
      //app.Counter += 1;
      
      float moist = analogRead(A1); // set-up read from pin 1 and make the value 
      Serial.println("reading moisture"); // Print value
      float sensor0P = 100.00 - ( ( 100.00 * moist ) / 1023.00 ); // Turn the value into a percentage
      uint8_t sensorInt = (uint8_t) sensor0P; //najes it a int16
      Serial.print("Moisture : "); // print
      Serial.print(sensorInt); // print value
      Serial.println("%"); //print percent sign
      lora.TX.Data[0] = sensorInt; // send it to LoRa
      

     
      float rainValue = rainCounter * 0.15 * 100.0 ; //calculation for 0.15mm per clicks change it for your bucket size
      Serial.println(rainValue); // print raw data
      val = (int16_t) (rainValue); //Turn it in to a 16 bit int
      Serial.println(val); // print value
      
      lora.TX.Data[1] = lowByte(val) ; // send data 
      lora.TX.Data[2] = highByte(val); // send data
      lora.TX.Count = 3; // the amount of data your sending
      
      lorawan.LORA_send_and_receive(); //send the data and check the recieved data
      lora.TX.Confirmation = CONFIRMED; //say that it is confimed it is only visable in TTN

  
      
      // Check whether a reply was received or not.
      if(lora.RX.retVal == RX_MESSAGE)
      {
        // Print the received data and handle the data
        printStringAndHex("Received data:", lora.RX.Data, lora.RX.Count); // this is for when it callibrated for MQTT 
        handle_reply(); //for confimation
        rainCounter = 0; //to do only if confirmed
        Serial.println(rainCounter);// print variable 
      }
      else
      {
        // Print the return value as to why no data is received.
        printStringAndHex("Retval: ", (uint8_t *)&(lora.RX.retVal), 1); // print the retrieved data
      } 


      // Clear the LED
      digitalWrite(LED, LOW);
    }
    else
    {
      // Go to POWER_DOWN_MODE to reduce power consumption when the Arduino has nothing to do.

      go_to_sleep();
    }
  }//While(1)
}


/*
	@brief
		Function which disables all peripherals and enters SLEEP_MODE_PWR_DOWN mode to reduce power consumption to 0.1uA. When an interrupts wakes the MCU
		from sleep, the peripherals and interrupts are enabled again.
*/
void go_to_sleep (void)
{

  
  Serial.println(rainCounter);
  Serial.println("Going to sleep..."); //added for de-bugger

  // Complete serial transfer before going to sleep
  Serial.flush();

  // Power off the peripherals
  ADCSRA &= ~(1 << ADEN);
  power_usart0_disable();
  power_timer0_disable();
  power_timer1_disable();
  power_timer2_disable();
  power_adc_disable();
  power_spi_disable();
  power_twi_disable();

  // Go to sleep mode and wake-up from the RTC's alarm or an Pin change Interrupt from the movement sensor.
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  cli();
  sleep_enable();
  sleep_bod_disable();
  sei();
  sleep_cpu();
  sleep_disable();
  sei();

  // Power on the peripherals
  power_usart0_enable();
  power_timer0_enable();
  power_timer1_enable();
  power_timer2_enable();
  power_adc_enable();
  power_spi_enable();
  power_twi_enable();
  ADCSRA |= (1 << ADEN);
}

void handle_reply (void)
{
  // Check if there is a single byte in the reply message from the back-end, if so retrieve it and copy it's contents to the LoRaWAN message interval
  if(lora.RX.Count == 0)
  {
    // Copy the received byte and reconfigure the LoRaWAN message interval when the received byte isn't zero.
    if(lora.RX.Data[0] != 0)
    {
      app.LoRaWAN_message_interval = lora.RX.Data[1]; 
      
      // Reconfigure the RTC for an interrupt on the next interval.
      mcp7940_reset_minute_alarm(app.LoRaWAN_message_interval);
    }
  }
}

/*
	@brief
		Function to print a string of text and an array of data in hexadecimal format.
	@parameters
		String	String of characters, must be zero terminated.
		data	Pointer to the array that must be printed in hexadecimal format
		n		Number of bytes that must be printed out in hexadecimal
*/
void printStringAndHex(const char *String, uint8_t *data, uint8_t n)
{
  uint8_t i;

  Serial.print(String);
  Serial.flush();
  Serial.print(n, DEC);
  Serial.print(" bytes; ");

  // Print the data as a hexadecimal string
  for ( i = 0 ; i < n ; i++)
  {
    // Print single nibbles, since the Hexadecimal format printed by the Serial.Print function does not print leading zeroes.
    Serial.print((unsigned char) ((data[i] & 0xF0) >> 4), HEX); // Print MSB first
    Serial.print((unsigned char) ((data[i] & 0x0F) >> 0), HEX); // Print LSB second
    Serial.print(' ');
    Serial.flush();
  }
  Serial.println();
}

/*
  @brief
    Function to handle incoming data from an confirmed message up. If there is a single received byte and it's value is not zero, the 
    value will be written to the LoRaWAN message interval and the RTC will be set to send a LoRaWAN message in the given number of minutes.
*/




