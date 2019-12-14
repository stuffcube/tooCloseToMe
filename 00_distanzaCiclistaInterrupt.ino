/*
 * 
  04dic19 | aggiunto wifi, led e tasto.
  12dic19 | non prendeva cambiao ssid
          | pulsante su D3 attiva invio dati
  
 * esp8266 wemos min D1 R1:
 * 
 * the lidar must be disconneted to program the chip
 * the serial1 is connected to D4 pin
 * 
 * http://arduino.esp8266.com/Arduino/versions/2.3.0/doc/reference.html
 * 
 */
#include <Arduino.h>

#include <ESP8266WiFi.h>
//#include <ESP8266WiFiMulti.h>
//ESP8266WiFiMulti WiFiMulti;


#include <ESP8266HTTPClient.h>

// parti da definire, password e server dati
char ssid[] = "";
char password[] = "";
#define yourServer "http://yourserver/"

#define LED_OFF   HIGH
#define LED_ON    LOW
#define TASTO_INVIO 0   // con D3 non funziona

int verbose = 1;
int riga = 0;
String url;

#define debugPort Serial1

  
#define CROSS_DISTANCE 250
#define INIT            -1
#define NO_CAR          0
#define CAR_INIT        1
#define CAR_ASIDE       2
#define CAR_LEAVE       3
#define CAR_PRINT       5
#define POINTS     1000

uint distance = 0;
uint strength = 0;
//boolean receiveComplete = false;



// collection of measurements during the crossing
struct _point{
    long tm;
    int  d;
};

struct _point points[1000];
int pointIndex = 0;

int sm = INIT;


//this structure cointains data about a car intersection
struct _carCrossing{
  long tInit;   // init time
  long tEnd;    // end time
  int  initDistance;
  int  endDistance;
  int  minDistance;
  int  averageDistance;
} ;

struct _carCrossing carCrossing[1000];

int carIndex = 0;

void sendData() {


    for (int index=0; index < carIndex; index++){
        // wait for WiFi connection
        delay(1000);
//        if ((WiFiMulti.run() == WL_CONNECTED)) {
          if (1){      
            WiFiClient client;
            
            HTTPClient http;
    
            debugPort.print("[HTTP] begin...\n");
            digitalWrite(LED_BUILTIN, digitalRead(LED_BUILTIN)); 
              
            url = yourServer;
            url += "comandi.php?cmd=add_riga";
            url += "&start_time=";
            url += carCrossing[index].tInit;
            url += "&end_time=";
            url += carCrossing[index].tEnd;
            url += "&start_distance=";
            url += carCrossing[index].initDistance;
            url += "&end_distance=";
            url += carCrossing[index].endDistance;
            debugPort.println(url);
    
  
            if (http.begin(client, url)) {  // HTTP
                debugPort.print("[HTTP] GET...\n");
                // start connection and send HTTP header
                int httpCode = http.GET();
          
                // httpCode will be negative on error
                if (httpCode > 0) {
                  // HTTP header has been send and Server response header has been handled
                  debugPort.printf("[HTTP] GET... code: %d\n", httpCode);
          
                  // file found at server
                  if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
                    String payload = http.getString();
                    debugPort.println(payload);
                  }
                } else {
                  debugPort.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
                }
          
                http.end();
              } else {
                debugPort.printf("[HTTP} Unable to connect\n");
           }
        }
    }
    debugPort.println("data sent");
    while(1){
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));   
        delay(1000); // wait to dead
    }

}

void initWiFi(void){
    /*
      WiFi.mode(WIFI_STA);
      WiFiMulti.addAP(ssid, password);
    */
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED){
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));   
        debugPort.println("trying WiFi connectionc");
        delay(1000); 
      
    }
    digitalWrite(LED_BUILTIN, LED_ON);   
    debugPort.println("WiFi connected");

}
//------------------------------------------------------------------------
void setup() {
    pinMode(LED_BUILTIN, OUTPUT);         // Initialize the LED_BUILTIN pin as an output
    pinMode(TASTO_INVIO, INPUT);          // Initialize the TASTO_INVIO pin as an input
    digitalWrite(TASTO_INVIO, HIGH);   
    digitalWrite(LED_BUILTIN, LED_OFF);   

    debugPort.begin(115200);
    Serial.begin(115200);
    debugPort.println("\n \n port1 initialized");
    //Serial.println("port0 initialized");

    distance = 0;
    carIndex = 0;
    
    
}

long sum;
int i, counter;

void loop() {

  //Serial.println(digitalRead(TASTO_INVIO));
  if(digitalRead(TASTO_INVIO) == 0) {
    digitalWrite(LED_BUILTIN, LED_ON);   
    
    debugPort.println("start wifi connection");
    initWiFi();
    debugPort.println("start send data");
    sendData();
    // never exit
    
  }

  
  if(dataAvalilable(&distance, &strength)) {
    //debugPort.println(distance);
  }
  else return; // questo fa esegueire la SM solo con nuovi dati






  switch (sm){
    case INIT:
      if (distance != 0) sm = NO_CAR;
      counter = 0;
      break;
      

    // wait distance goes below CROSS_DISTANCE
    case NO_CAR:
  		if (distance < CROSS_DISTANCE){

        counter++;
        if (counter > 10){
            carIndex++;
            if(carIndex >= 1000) carIndex = 0;
      			
      			carCrossing[carIndex].tInit 		= millis();
      			carCrossing[carIndex].initDistance= distance;
      			sm = CAR_ASIDE;
            digitalWrite(LED_BUILTIN, LED_ON);   

      			if (verbose){
      				debugPort.println("CAR_ASIDE");
      			}
  			}
      }
      else counter = 0;
      break;


    case CAR_ASIDE:
  		if (distance > CROSS_DISTANCE){
        
  			carCrossing[carIndex].tEnd 		= millis();

        // indice indietro di n posizioni
  			carCrossing[carIndex].endDistance	= points[pointIndex - 10].d;
  			pointIndex = 0;
  			sm = CAR_LEAVE;
  			if (verbose){
  				debugPort.println("CAR_LEAVE");
  			}
  		}
  		else{
  			points[pointIndex].tm = millis();
  			points[pointIndex].d  = distance;	
  			pointIndex++;
  			if (pointIndex >= POINTS)  pointIndex = POINTS;
  		}
    
      break;

    case CAR_LEAVE:
    
  		  // do calculation abaout average or other
  			sm    = CAR_PRINT;
        riga  = 0;
        sum   = 0;
        for (i=0; i< pointIndex; i++){
            sum += points[i].d;
        }
        //carCrossing[carIndex].averageDistance = (int)(sum/pointIndex);
  
        digitalWrite(LED_BUILTIN, LED_OFF);   

  
        break;
    

      case CAR_PRINT:
  			if (verbose){
  	      switch (riga){
  				    case 0:   debugPort.print("CAR: ");                       break;
              case 1:   debugPort.println(carIndex);                    break;
          
              case 2:   debugPort.print("start time:");                 break;
              case 3:   debugPort.println(carCrossing[carIndex].tInit );break;
        			case 4:   debugPort.print("aside time:")         ;        break;
        			case 5:   debugPort.println(carCrossing[carIndex].tEnd - carCrossing[carIndex].tInit );break;
        			case 6:   debugPort.print("init, end, average distance:");break;
        			case 7:   debugPort.print(carCrossing[carIndex].initDistance);break;
              case 8:   debugPort.print(", ");                          break;
              case 9:   debugPort.print(carCrossing[carIndex].endDistance );break;
              case 10:  debugPort.print(", ");                          break;
              case 11:
                    debugPort.println(carCrossing[carIndex].averageDistance );
                    sm = INIT;
                    break;
  	      }
         riga++;
  			}
      break;
  }
  
}


char dataAvalilable(uint* _distance, uint* _strength){
    while(Serial.available()>=9)
    {
        if((0x59 == Serial.read()) && (0x59 == Serial.read())) //Byte1 & Byte2
        {
            
            unsigned int t1 = Serial.read(); //Byte3
            unsigned int t2 = Serial.read(); //Byte4

            t2 <<= 8;
            t2 += t1;

            *_distance = t2;
            //Serial1.print(t2);
            //Serial1.print('\t');

            t1 = Serial.read(); //Byte5
            t2 = Serial.read(); //Byte6

            t2 <<= 8;
            t2 += t1;
            *_strength = t2;
            //Serial1.println(t2);

            for(int i=0; i<3; i++) 
            { 
                Serial.read(); ////Byte7,8,9
            }

            return(1);
        }
    }
    return(0);
}
