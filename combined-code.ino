#include <EEPROM.h>
#include <GravityTDS.h>

#define TdsSensorPin A0
#define SensorPin A1          // the pH meter Analog output is connected with the Arduino's Analog

unsigned long int avgValue;  //Store the average value of the sensor feedback
float b;
int buf[10], temp;

GravityTDS gravityTds;

float temperature = 25, tdsValue = 0;

void setup()
{
    //for tds/temp
    Serial.begin(115200);
    gravityTds.setPin(TdsSensorPin);
    gravityTds.setAref(3.3);  //reference voltage on ADC, default 5.0V on Arduino UNO
    gravityTds.setAdcRange(1024);  //1024 for 10bit ADC;4096 for 12bit ADC
    gravityTds.begin();  //initialization
    
    //for ph
    pinMode(13,OUTPUT);  
    Serial.begin(115200);  
    Serial.println("Ready");    //Test the serial monitor
}

void loop()
{
    temperature = readTemperature();  //add your temperature sensor and read it
    gravityTds.setTemperature(temperature);  // set the temperature and execute temperature compensation
    gravityTds.update();  //sample and calculate
    tdsValue = gravityTds.getTdsValue();  // then get the value, tds value is in ppm
    delay(1000);
    
    for(int i=0;i<10;i++)       //Get 10 sample value from the sensor for smooth the value
    { 
      buf[i]=analogRead(SensorPin);
      delay(10);
    }
    for(int i=0;i<9;i++)        //sort the analog from small to large
    {
      for(int j=i+1;j<10;j++)
      {
        if(buf[i]>buf[j])
        {
          temp=buf[i];
          buf[i]=buf[j];
          buf[j]=temp;
        }
      }
    }
    avgValue=0;
    for(int i=2;i<8;i++)                      //take the average value of 6 center sample
      avgValue+=buf[i];
    float phValue=(float)avgValue*5.0/1024/6; //convert the analog into millivolt
    phValue=3.5*phValue;                      //convert the millivolt into pH value
    digitalWrite(13, HIGH);       
    delay(800);
    digitalWrite(13, LOW); 
    // variables sent back: phValue, temperature, tdsValue
}
