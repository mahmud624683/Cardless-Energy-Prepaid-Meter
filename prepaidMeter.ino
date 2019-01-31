/*  
     PINOUT: 
         _____________________________
        |  ARDUINO UNO >>>   SIM800L  |
         -----------------------------
             GND      >>>   GND
         RX  10       >>>   TX    
         TX  11       >>>   RX
        RESET 2       >>>   RST 
                  
    POWER SOURCE 4.2V >>> VCC
    Credit: Mahmudul Hasan
    email : mhb1406077@gmail.com
 
*/

#include <SIM800lM.h>
#include <SoftwareSerial.h> //SoftwareSerial library for serial operation
Sim800lM Sim800lM;  //to declare the Object for GSM
#include "PowerAnalyzer.h" // including power measuring library created by shuvonkor shuvo 
#define VOLT_PIN 0                     //Voltage signal is connected with Anaog pin 0
#define CURRENT_PIN 1                  //current Signal ic connected with Analog Pin 1
Measure measure(VOLT_PIN,CURRENT_PIN); //Creating the Object for measuring power 
unsigned long time1=0; 				// used to 
unsigned long time2=0;				// measure time difference between two measurement of power 
unsigned long day2=0;
float Power =0;					// power measuring variable
double wh=0,whr=0;					//variable to measure Kilo Watt Hour 
uint8_t day =0;

String textSms,numberSms;		//String to get SMS content and number 
uint8_t cntrl=13,bf=0;//to control relay
double blnc=100.00;//Initial balance of this meter
bool error;



void setup(){
    Serial.begin(9600);
    digitalWrite(cntrl,HIGH);

    Sim800lM.begin(); // initializate the library. 
    error=Sim800lM.delAllSms(); //clean memory of sms;
	Serial.println(error);
    time2=millis();
}

void loop(){
     //start block of msg cmnd
     time1=millis();
     Power = 12.0373*measure.power();//12.0373 is our power measuring factor inverse of voltage divider ratio/current measuring resistive value    
     wh=wh+Power*(time1-time2)/3600000;//measuring KWh 
     time2=time1;
     Serial.print("Current Load is : ");Serial.print(Power);Serial.println("Watt");
    textSms=Sim800lM.readSms(1); //read the first sms
    Serial.println("Received SMS :  "+textSms);
    if (textSms.indexOf("OK")!=-1 && textSms.indexOf("UNREAD")!=-1) //first we need to know if the messege is correct and UNREAD NOT an ERROR or READ SMS 
        { 
          Serial.println("entering statements");         
        if (textSms.length() > 7)  // optional you can avoid SMS empty
            {
				/*	Msg Format are :
				*	For Recharge 		: RE 1212(This is Password) %100(This is Balance)
				*	For Statement		: Statement 1212 
				*	For Power Turn OFF	: Off 1212
				*	For Power Turn ON	: On 1212
				*
				*/
                
                numberSms=Sim800lM.getNumberSms(1);  // Here you have the number
                if (numberSms=="+8801718809322" && textSms.indexOf("1212")!=-1)//number and Password verification 
                {
                  textSms.toUpperCase();
                  if (textSms.indexOf("RE 1212")!=-1)//checking recharge format
                  {
                      Serial.println("REcharge");
                      int pstn = textSms.indexOf('%')+1;//recharge amount number should contain % before amount
                      String recrg="";
                      uint8_t i=0,flag=0;
                      while(textSms.charAt(pstn)>'9'  )//extracting amount string
                      {
                          if((textSms.charAt(pstn)>='0' && textSms.charAt(pstn)<='9') || textSms.charAt(pstn)=='.')
                            {
                              recrg.concat(textSms.charAt(pstn));
                              Serial.println(recrg);
                              
                            }
                          else 
                          {
                            flag=1;
                            break;
                          }
						  pstn++;
                      }
                      if (flag==0){
                        blnc=blnc+Sim800lM.toDouble(recrg);//converting amount string to Double 
                        if(blnc>10)
                        {
                          digitalWrite(cntrl,HIGH);//turning on power in case it is off
                          bf=0;// balance warning flag
                        }
                        error=Sim800lM.sendSms(numberSms,"Your Prepaid meter has recharged by Tk."+recrg+" .your current balance is Tk."+blnc);//recharge confirmation message
                        Serial.println("Recharge DONE");
                      }
                      else 
                      {
                        error=Sim800lM.sendSms(numberSms,"Worng format of recharge amount");//feedback message
                        Serial.println("Worng format of recharge amount");
                      }
                      time1=millis();
                      Power = 12.0373*measure.power();
                      wh=wh+Power*(time1-time2)/3600000;
                      time2=time1;
                          
                }
                else if (textSms.indexOf("STATEMENT")!=-1)//checking Statement 
                {
                       
                       error=Sim800lM.sendSms(numberSms,"Your Current Balance Tk."+Sim800lM.toString(blnc)+" .Voltage : "+Sim800lM.toString(measure.voltage())+" V.Unit : "+Sim800lM.toString(wh)+" KWh.Load : "+Sim800lM.toString(Power)+" W");//feedback message
                       
                        Serial.print("Current Balance is : ");Serial.println(blnc);
                }
               
                else if (textSms.indexOf("ON")!=-1)//manual power turn on
                {
                       
                       if(cntrl==LOW)
                       {
                        error=Sim800lM.sendSms(numberSms,"Power Connection Turned ON");//feedback message
                        Serial.print("Power Connection Turned ON");
                        digitalWrite(cntrl,HIGH);
                       }
                       else
                       {
                          error=Sim800lM.sendSms(numberSms,"Power Connection has already Turned ON");//feedback message
                          Serial.print("Power Connection has already Turned ON");
                       }
                }
                else if (textSms.indexOf("OFF")!=-1)//manual power turn off
                {
                       
                       if(cntrl==HIGH)
                       {
                        error=Sim800lM.sendSms(numberSms,"Power Connection Turned OFF");//feedback message
                        Serial.print("Power Connection Turned OFF");
                        digitalWrite(cntrl,LOW);
                       }
                       else
                       {
                          error=Sim800lM.sendSms(numberSms,"Power Connection has already Turned OFF");//feedback message
                          Serial.print("Power Connection has already Turned OFF");
                       }
                }
                    
                else 
                {
                  error=Sim800lM.sendSms(numberSms,"Message format is wrong");//feedback message
                  Serial.println("Message format is wrong");
                }
               time1=millis();
               Power = 12.0373*measure.power();
               wh=wh+Power*(time1-time2)/3600000;
               time2=time1;

            }
            else 
            {
              error=Sim800lM.sendSms(numberSms,"Invalid Number :( or Password");//feedback message
              Serial.println("Invalid Number :( or Password");
            }

            
            }
            
        }

        //end block of msg cmnd
        
        if (textSms.indexOf("OK")!=-1 )
        {
          error=Sim800lM.delAllSms(); //clean memory of sms;
            Serial.println(error); 
        }
		if((millis()-day2)>=86400000)
		{
			day++;
			day2+=86400000;
		}
		if((millis()<day2)&&(millis()+day2-4294965153)>86400000)
		{
			day2=millis();
			day++;
		}
		/*electric power tarrif :
           _____________________________
            Range  >>>   Rate
           -----------------------------
           Category-LT-A : Residential 
            0 to 50 >>>  3.50 
            0 to 75 >>>  4.00
          76 to 200 >>>  5.45
         201 to 300 >>>  5.70
         301 to 400 >>>  6.02
         401 to 600 >>>   9.30
          Above 600 >>>   10.70 
        **Data taken from DPDC website on DEC 26,2017
        */
        if((wh-whr)>0.01)
        {
			if(wh<=75)
			{
				blnc=blnc-(4.00*(wh-whr));
			}
			else if(wh<=200)
			{
				blnc=blnc-(5.45*(wh-whr));
			}
			else if(wh<=300)
			{
				blnc=blnc-(5.70*(wh-whr));
			}
			else if(wh<=400)
			{
				blnc=blnc-(6.02*(wh-whr));
			}
			else if(wh<=600)
			{
				blnc=blnc-(9.30*(wh-whr));
			}
			else
			{
				blnc=blnc-(10.70*(wh-whr));
			}
          if(day==31)
		  {
			  day=0;
			  if(wh<=50)
				 blnc=blnc+(0.50*wh); 
			 error=Sim800lM.sendSms(numberSms,"Your Month end Useage is : "+Sim800lM.toString(wh)+" KWh.Your Current Balance Tk."+Sim800lM.toString(blnc));//feedback message
			 wh=0;
		  }
          whr=wh; 
          Serial.println(blnc);
        } 
		if(millis()>4294963158)
		{
			while(!(millis()));
			Power = 12.0373*measure.power();
            wh=wh+Power*(4294965153-time2)/3600000;
            time2=0;
			
		}
        //end block of power consumption
        if(blnc<=0.01)//power cut off due to low balance
                       {
                        error=Sim800lM.sendSms(numberSms,"Your connection has cut off due to 0 balance.please recharge to get turn on again");//feedback message
                        Serial.print("Power Connection Turned OFF");
                        digitalWrite(cntrl,LOW);
                       }
        else if(blnc<10 && bf==0)//warning message due to low balance
        {
              error=Sim800lM.sendSms(numberSms,"Your current balance is below Tk.10.please recharge Soon");//feedback message
              Serial.print("Your current balance is below Tk.10.please recharge Soon");
              bf++;
        }
        Serial.println("loops end");
             
        
    }
 
