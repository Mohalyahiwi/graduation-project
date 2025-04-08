//the timer/ppm signal generation was taken from Spychalski, P. (2021) How to generate PPM signal with ESP32 and Arduino, QuadMeUp. Available at: https://quadmeup.com/how-to-generate-ppm-signal-with-esp32-and-arduino/. 

// you get these values from the blynk Templates tab
#define BLYNK_TEMPLATE_ID "TMPL6IfDiKLi7"
#define BLYNK_TEMPLATE_NAME "WiFiCtlDrone"
//this you get from the devices tab
#define BLYNK_AUTH_TOKEN "Xd-rtbWL5DX70OBPrhu-HaK0hUY6TrHA"
//library needed 
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

#define PPM_FRAME_LENGTH 22500 //the lentgh of ppm frame in µs
//#define PPM_FRAME_LENGTH 30000 //the lentgh of ppm frame in µs
#define PPM_PULSE_LENGTH 300//the lentgh of ppm pulse
#define PPM_CHANNELS 8//number of channels 
#define DEFAULT_CHANNEL_VALUE 1500
#define OUTPUT_PIN 21
//initializing wifi and blynk
char auth[] = "Xd-rtbWL5DX70OBPrhu-HaK0hUY6TrHA";
//char ssid[] = "MOAB_Guest";
//char pass[] = "A38244444a";
char ssid[] = "Xiaomi 11 Lite 5G NE";
char pass[] = "155155155";
//valuse for the 8 channel ppm
uint16_t channelValue[PPM_CHANNELS] = {1501, 1500, 1000, 1501, 1500, 1500, 1500, 1500};//def values 

hw_timer_t *timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

enum ppmState_e
{
    PPM_STATE_IDLE,
    PPM_STATE_PULSE,
    PPM_STATE_FILL,
    PPM_STATE_SYNC
};

void IRAM_ATTR onPpmTimer()
{

    static uint8_t ppmState = PPM_STATE_IDLE;
    static uint8_t ppmChannel = 0;
    static uint8_t ppmOutput = LOW;
    static int usedFrameLength = 0;
    int currentChannelValue;

    portENTER_CRITICAL(&timerMux);

    if (ppmState == PPM_STATE_IDLE)
    {
        ppmState = PPM_STATE_PULSE;
        ppmChannel = 0;
        usedFrameLength = 0;
        ppmOutput = LOW;
    }

    if (ppmState == PPM_STATE_PULSE)
    {
        ppmOutput = HIGH;
        usedFrameLength += PPM_PULSE_LENGTH;
        ppmState = PPM_STATE_FILL;

        timerAlarmWrite(timer, PPM_PULSE_LENGTH, true);
    }
    else if (ppmState == PPM_STATE_FILL)
    {
        ppmOutput = LOW;
        currentChannelValue = channelValue[ppmChannel];

        ppmChannel++;
        ppmState = PPM_STATE_PULSE;

        if (ppmChannel >= PPM_CHANNELS)
        {
            ppmChannel = 0;
            timerAlarmWrite(timer, PPM_FRAME_LENGTH - usedFrameLength, true);
            usedFrameLength = 0;
        }
        else
        {
            usedFrameLength += currentChannelValue - PPM_PULSE_LENGTH;
            timerAlarmWrite(timer, currentChannelValue - PPM_PULSE_LENGTH, true);
        }
    }
    portEXIT_CRITICAL(&timerMux);
    digitalWrite(OUTPUT_PIN, ppmOutput);
}

//setting up virtual pins VX v stands for virtual pin  X stand for the number of the v pin function to connect blynk interface to arduino IDE
BLYNK_WRITE(V0) {
    int input = param.asInt(); // Get the current state of the button
    
    if (input == 1&&channelValue[0]<=1900) {
        channelValue[0] += 25; // increase 'Roll' by 50 when the button is pressed. You can control movement speed by increasing/decreasing this value from the code
        Serial.print("Roll: ");
        Serial.println(channelValue[0]);//serial print will be printed to the serial monitor 
        
    }else
     channelValue[0] = 1500;
      Serial.print("RollNoIF: ");
        Serial.println(channelValue[0]);
}
BLYNK_WRITE(V1) {
    int input = param.asInt(); // same as before 
    
    if (input == 1&&channelValue[0]>=1000) {
        channelValue[0] -= 25; // decreeses  Roll by 50 when the button is pressed 
        Serial.print("Roll: ");
        Serial.println(channelValue[0]);
        
    }else
     channelValue[0] = 1500;
      Serial.print("RollNoIF: ");
        Serial.println(channelValue[0]);
}
BLYNK_WRITE(V2) {
    int input = param.asInt(); //same as roll function 
    
    if (input == 1&&channelValue[1]<=1900) {
        channelValue[1] += 25; 
        Serial.print("Pitch: ");
  Serial.println(channelValue[1]);
        
    }else{
     channelValue[1] = 1500;
       Serial.print("PitchNoIF: ");
  Serial.println(channelValue[1]);}
}
BLYNK_WRITE(V3) {
    int input = param.asInt(); 
    
    if (input == 1&&channelValue[1]>=1000) {
        channelValue[1] -= 25; 
         Serial.print("Pitch: ");
  Serial.println(channelValue[1]);
        
    }else{
     channelValue[1] = 1500;
       Serial.print("PitchNoIF: ");
  Serial.println(channelValue[1]);}
}
BLYNK_WRITE(V4) {
    int input = param.asInt();
    channelValue[2] = map(input, 0, 250, 1000, 2000);
    Serial.print("Throttle: ");
  Serial.println(channelValue[2]);


}
// control yaw ccw and cw direction 
BLYNK_WRITE(V5) {
    int input = param.asInt();
    if(input==1){
    channelValue[3]=1500-150;
  Serial.println(channelValue[3]);}
  else {
  channelValue[3]=1500;
   Serial.println(channelValue[3]);
  }}
  BLYNK_WRITE(V6) {
    int input = param.asInt();
    if(input==1){
    channelValue[3]=1500+150;
  Serial.println(channelValue[3]);}
  else {
  channelValue[3]=1500;
   Serial.println(channelValue[3]);
  }
  


}

// setting up the modes ARM,FailSafe,AngleMode

BLYNK_WRITE(V7) {
     channelValue[4] =param.asInt();
    Serial.print("arm: ");
  Serial.println(channelValue[4]);


}
//set FailSafe mode
BLYNK_WRITE(V8) {
     channelValue[5] =param.asInt();
    Serial.print("Fail Safe: ");
  Serial.println(channelValue[5]);
}
// set ANGLEMODE
BLYNK_WRITE(V9) {
     channelValue[6] =param.asInt();
    Serial.print("ANGLEMODE: ");
  Serial.println(channelValue[6]);


}


void setup()
{
Serial.begin(115200);
Blynk.begin(auth, ssid, pass);

    pinMode(OUTPUT_PIN, OUTPUT);//set the desierd ouput pin 
    //seting up the timer
   timer = timerBegin(0, 80, true);
    //timer = timerBegin(0, 240, true);
    timerAttachInterrupt(timer, &onPpmTimer, true);
    timerAlarmWrite(timer, 12000, true);
    timerAlarmEnable(timer);
  
}

void loop()
{
  
 
     Blynk.run();

    
}