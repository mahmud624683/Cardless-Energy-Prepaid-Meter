//Credit : Cristian Steib
//Modified by : Mahmudul Hasan


#ifndef Sim800lM_h
#define Sim800lM_h
#include <SoftwareSerial.h>
#include "Arduino.h"


#define RX_PIN 10
#define TX_PIN 11
#define RESET_PIN 2   // pin to the reset pin sim800l

#define LED true // used for indicator led, in case that you don want set to false .
#define LED_PIN 13 //pin to indicate states.




class Sim800lM
{
  private:
	int _timeout;
	String _buffer;
		String _readSerial();


  public:

 	void begin();
 	void reset();
 	//Arithmetic operation
 	String toString(double num);
 	double toDouble(String numS);

 	// Methods for calling || Funciones de llamadas.
 	bool answerCall();
 	void callNumber(char* number);
 	bool hangoffCall();
 	uint8_t getCallStatus();
 	//Methods for sms || Funciones de SMS.
	bool sendSms(String number,String text);
	String readSms(uint8_t index); //return all the content of sms
	String getNumberSms(uint8_t index); //return the number of the sms..
	bool delAllSms();     // return :  OK or ERROR ..

	void signalQuality();
	void setPhoneFunctionality();
	void activateBearerProfile();
	void deactivateBearerProfile();
	//get time with the variables by reference
	void RTCtime(int *day,int *month, int *year,int *hour,int *minute, int *second);
	String dateNet(); //return date,time, of the network
	bool updateRtc(int utc);  //Update the RTC Clock with de Time AND Date of red-.
};

#endif
