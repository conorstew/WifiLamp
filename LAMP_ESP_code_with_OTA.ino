#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include <PubSubClient.h>

#ifndef STASSID
#define STASSID "your wifi ssid"
#define STAPSK  "your wifi password"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;


int bluePin=14;
int greenPin=12;
int redPin=13;


int rgb[3];   //int to store each brightness value (i.e for red, green, blue)
const char* mqtt_server = "192.168.1.26";
const int mqttPort = 1883;

WiFiClient espClient; // set the mqtt client to use wifi (either wifi or ethernet) and give it a unique name.
PubSubClient mqttClient(mqtt_server, mqttPort, espClient); // create instance of pubsub client and configure it.


int On_Off_Tracker = 0;   // int to know if light is on or off (on=1, off=0)


void callback_function(const char* topic, byte* payload, unsigned int length){    // function that is called when a mqtt message arrives for our client
  //String payload_str;    // converted payload to string for easy compare with topic.
  char message[length];  // convert payload to a char[] so that we can input it into number_extractor function.
  for (int i = 0; i < length; i++) {
   // payload_str += (char)payload[i];
    message[i] = (char)payload[i];
  }



  if(strcmp(topic, "Lamp/Color") == 0){

    Serial.print("\n The Lamp color has been changed\n");

    number_extractor(message);    // set rgb[] values

    if(On_Off_Tracker == 1){      //if light is on, then change color
      analogWrite(redPin, rgb[0]);
      analogWrite(greenPin, rgb[1]);
      analogWrite(bluePin, rgb[2]);
    }



  }
  else if(strcmp(topic, "Lamp/Power")==0){
    Serial.print("\nin Topic Lamp/Power\n");
    if(message[0]== 't'){        // ON
      analogWrite(redPin, rgb[0]);
      analogWrite(greenPin, rgb[1]);
      analogWrite(bluePin, rgb[2]);
      Serial.print("\n The Lamp is Now ON");
      On_Off_Tracker = 1;
    }
    else if(message[0]== 'f'){    // OFF
      analogWrite(redPin, 0);
      analogWrite(greenPin, 0);
      analogWrite(bluePin, 0);
      Serial.print("\n The Lamp is Now OFF");
      On_Off_Tracker = 0;
    }
  }






  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();                       //this callback function is just a test
}

void setup_wifi(){    //function that connects ESP to wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  Serial.print("Connected to: "); Serial.print(ssid); Serial.print("!!!!!!!\n\n\n");
}

void mqttServer_connect(){  //function that connects esp to mqtt broker
  mqttClient.connect("NodeMCU"); //start connection to mqtt server and give unique client id
  Serial.print("Connecting to mqtt server: "); Serial.print(mqtt_server); Serial.print(":");Serial.print(mqttPort);
  Serial.print("\nConnecting");
  while(!mqttClient.connected()){
    //Serial.print(mqttClient.state());   //will print error code for troubleshooting
    Serial.print(" .");
    delay(1000);
  }
  Serial.print("\nMQTT Connection Established: "); Serial.print(mqtt_server); Serial.println("!!!!!!!\n\n\n");

}

void number_extractor(char payload[]){     // function to extract the rgb values from the mqtt payload (which is a string)

      int rgbTracker=0; //int to track which color we are on (1=red, 2=green, 3=blue)
      int num_length=0;
      for(int i=0; payload[i]!='\0'; i++){

        if(isdigit(payload[i])){
          num_length++;
        }
        else if(!isdigit(payload[i]) && num_length>0){
          char digits[num_length];
          int k = 0;
          for(int j=num_length; j>0; j--){
            digits[k++]=payload[i-j];
          }

          rgb[rgbTracker++] = atoi(digits);  //convert digits[] into a single int and store that in rgb[] which is an external variable
          num_length=0;

          for(int i=0; i<3; i++){   //reset all of digits[] entries to null terminator '\0'
              digits[i]='\0';
          }

        }   // end of else if
      }
    }





void setup() {
  Serial.begin(115200);
  Serial.println("Booting");
  setup_wifi(); //connect to wifi

//***************** OTA SETUP-CODE START: ***************************

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
   ArduinoOTA.setHostname("LAMP-esp8266");

  // No authentication by default
   ArduinoOTA.setPassword("your custom OTA password");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
//****************** END OF OTA SETUP-CODE**********************************


//*************************************
//your setup code goes here:
//*************************************

  pinMode(bluePin, OUTPUT);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);

  mqttServer_connect();
  mqttClient.setCallback(callback_function);
  mqttClient.subscribe("Lamp/Power");   //Subscribes to the topic "Lamp/Power"
  mqttClient.subscribe("Lamp/Color");   //Subscribes to the topic "Lamp/Color"

}






void loop() {
  ArduinoOTA.handle();  //must be here to have OTA updats

  mqttClient.loop();

}
