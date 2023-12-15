
#include "Arduino.h"
#include "WiFi.h"
#include "Audio.h"
#include <ezButton.h>
#include <EEPROM.h>

//ESP32 I2S digital output pins
#define I2S_DOUT      25  //GPIO 25 (DATA Output - the digital output. connects to DIN pin on MAX98357A I2S amplifier)
#define I2S_BCLK      27  //GPIO 26 (CLOCK Output - serial clock. connects to BCLK pin on MAX98357A I2S amplifier)
#define I2S_LRC       26  //GPIO 27 (SELECT Output - left/right control. connects to LRC pin on MAX98357A I2S amplifier)

//ESP32 analog input (WARNING: limit voltage to 3.3v maximum)
#define POT_PIN             34  //GPIO 34   //input pin used to control the audio volume

#define CHAN_UP_PIN         5   //increases the channel number
#define DEBOUNCE_TIME 50
#define NUMBER_OF_CHANNELS  5  //this should match the number of URLs found in the connect() function below
#define LED_PIN       2   //GPIO 2    //used by the code to control the ESP32's blue LED 
#define VOLUME_CONTROL_STEPS  100     //100 steps -- the potentiometer (on GPIO34) controls audio volume between zero and 100%
#define WIFI_MAX_TRIES        5      //number of attempts to connect to WiFi during startup
#define EEPROM_SIZE 1

ezButton upButton(CHAN_UP_PIN);

int currentChannelNumber = 1;

const String ssid         = "KrishnaGW";   //wifi network name 
const String password     = "jk88!@#THk215";  //wifi password

Audio audio;  //class from the ESP32-audioI2S library

void setupAudio(){
 
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolumeSteps(VOLUME_CONTROL_STEPS);
  int volume = map(analogRead(POT_PIN), 0, 4095, 0, VOLUME_CONTROL_STEPS);  // map potentiometer value to a volume percentage
  audio.setVolume(volume);
}

void connectWiFi() {
  
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());

  int tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries < WIFI_MAX_TRIES) {
      tries++;
      //blink the LED while we are trying to connect
      digitalWrite(LED_PIN, HIGH);
      delay(500);
      digitalWrite(LED_PIN, LOW);
      delay(500);
  }

  if (WiFi.status() == WL_CONNECTED){
    digitalWrite(LED_PIN, HIGH);
   } else {
    digitalWrite(LED_PIN, LOW);
   }
  }

//  Note: several more test audio files can be found here: 
//  https://github.com/schreibfaul1/ESP32-audioI2S/tree/master/additional_info/Testfiles
void connect(Audio *audio, int channel) {
  switch (channel){
  
    //  *** radio streams ***
    case 1:
          (*audio).connecttohost("http://vis.media-ice.musicradio.com/CapitalMP3");
          break;

    case 2:
          (*audio).connecttohost("http://s2.free-shoutcast.com:18146/");
          break;
      
    case 3:
          (*audio).connecttohost("http://server.geetradio.com:8020/radio.mp3");
          break;
  
    case 4:
          (*audio).connecttohost("http://69.162.124.24:8888/Malayalam");
          break;
  
    case 5:
          (*audio).connecttohost("https://eu10.fastcast4u.com:2140/");
          break;

    case 6:
          (*audio).connecttohost("http://173.249.30.175:8000/");
          break;
    
  }

}

void blinkSOS(int repeats){
  int tries = 0;
  while (tries < repeats){

      for (int i=0; i<3; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(200);
        digitalWrite(LED_PIN, LOW);
        delay(200);
      }

      for (int i=0; i<3; i++) {
        delay(200);
        digitalWrite(LED_PIN, HIGH);
        delay(800);
        digitalWrite(LED_PIN, LOW);
      }

      for (int i=0; i<3; i++) {
        delay(200);
        digitalWrite(LED_PIN, HIGH);
        delay(200);
        digitalWrite(LED_PIN, LOW);
      }
    delay(1000);
    tries++;
  }
}

void setupButtons(){
  pinMode(CHAN_UP_PIN, INPUT_PULLUP);
  upButton.setDebounceTime(DEBOUNCE_TIME);
}

void setup() {

  delay(1000);
  pinMode(LED_PIN, OUTPUT); //use the LED to indicate WiFi status
  connectWiFi();
  setupAudio();
  setupButtons();
  EEPROM.begin(EEPROM_SIZE);

  currentChannelNumber = EEPROM.read(0);
  connect(&audio, currentChannelNumber);

}

void loop() {
  int volume = map(analogRead(POT_PIN), 0, 4095, 0, VOLUME_CONTROL_STEPS);
  audio.setVolume(volume);

  bool changingChannels = false;

  upButton.loop();

   if ( upButton.isReleased() ) { 
    changingChannels = true;
    currentChannelNumber = currentChannelNumber + 1;
    if (currentChannelNumber > NUMBER_OF_CHANNELS){
         currentChannelNumber = 1;
    }
   }

  if (changingChannels) {
    EEPROM.write(0, currentChannelNumber);
    EEPROM.commit();
    connect(&audio, currentChannelNumber);
  }

  audio.loop();
}
