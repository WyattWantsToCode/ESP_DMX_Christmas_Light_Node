/*
 * ESP8266 (NodeMCU) Handling Web form data basic example
 * https://circuits4you.com
 */
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <Preferences.h>
#include <NeoPixelBus.h>

#include <ArtnetnodeWifi.h>
#include <WifiConfigure.h>
WifiConfigure wifiConfigure;

Preferences preferences;
ArtnetnodeWifi artnetnode;

int pixelCount = 10;
int maxPixelCount = 500;
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(maxPixelCount);
// GPIO 3 or RX pin on ESP8266


int dmxAdress = 1;

//Server on port 80
ESP8266WebServer server(80); 

// Function run when / is called 
void handleRoot() { 

//HTML that is sent
String page = "<!DOCTYPE html><html>";
page += "<body>";
page += "<h3> HTML Form ESP8266</h3>";
page += "<p><a href='/restart'> RESTART ESP8266 </a></p>";
page += "<form action='/control'>";
page += "DMX Adress:<br>";
page += "  <input type='number' name='dmxAdress' value=";
page += dmxAdress;
page += "> <br>";
page += "PixelCount: <br>";
page += "  <input type='number' name='pixelCount' value=";
page += pixelCount;
page += "> <br>";
page += "  <input type='submit' value='Submit'>";
page += "</form></body></html>";

server.send(200, "text/html", page);
}

// Saves the new DMX address in storage
void updateDMXAdress(int adress){
  preferences.begin("my-app", false);
  preferences.putInt("dmxAdress", adress);
  preferences.end();
}

// Updates the number of pixels in the strip in storage
void updatePixelCount(int pixelCount){
  preferences.begin("my-app", false);
  preferences.putInt("pixelCount", pixelCount);
  preferences.end();
}

// Function that runs when submit is hit
void handleForm() {
// Get HTTP argument for dmxAddress
// http://192.168.1.1/control?dmxAddress=21
 String newDMXAdress = server.arg("dmxAdress");
 dmxAdress = newDMXAdress.toInt();
 updateDMXAdress(dmxAdress);

// Get HTTP argument for dmxAddress
// http://192.168.1.1/control?pixelCount=21
 String newPixelCount = server.arg("pixelCount");
 pixelCount = newPixelCount.toInt();
 updatePixelCount(pixelCount);

 String page = "<a href='/'> Go Back </a>";
 server.send(200, "text/html", page);
}

void setup(void){
  Serial.begin(115200);

  //Gets Information from storage
  preferences.begin("my-app", false);
  dmxAdress = preferences.getInt("dmxAdress", 1);
  pixelCount = preferences.getInt("pixelCount", 10);
  preferences.end();

  // Custom WifiConfigure Library setup function
  wifiConfigure.begin();

  //Define artnet setup information
  artnetnode.setName("ESP8266 - Artnet");
  artnetnode.setNumPorts(1);
  artnetnode.enableDMXOutput(0);
  artnetnode.setStartingUniverse(1);
  artnetnode.begin();
  
  // Setup and start LED Strip
  strip.Begin();
  strip.Show();
 
  // Define HTTP Server routes
  server.on("/", handleRoot); //Home Page
  server.on("/control", handleForm); //Controlling the storage information (DMX Adress start and Number of pixels)
  server.on("/restart", ESP.restart); //Restart ESP remotely
  server.begin();

}

// Set Entire Strip to specific color based off of RGB
void updateSolidColors(int r, int g, int b) {
  RgbColor color(r,g,b);
  for (uint16_t pixel = 0; pixel < pixelCount; pixel++)
    {
      strip.SetPixelColor(pixel, color);   
    }
  strip.Show();    
}

// LEDs chase around in a cricle
int rotateHeadIndex = 5;
int rotateTailIndex = 0;
void rotateSolidColor(RgbColor color, int bpm){

  //Figure out which direction it needs to rotate
  if(bpm > 0){
    // Set the color of the head and make the tail off
    strip.SetPixelColor(rotateHeadIndex, color);
    strip.SetPixelColor(rotateTailIndex, RgbColor(0,0,0));
    // Show the new lights
    strip.Show();

    // Make the Head and tail rotate in the possitive direction
    rotateHeadIndex ++;
    rotateTailIndex ++;

    // Make sure that we do not try and show any light that is not on the strip
    // Nothing above pixelCount or below 0
    if(rotateHeadIndex > pixelCount) rotateHeadIndex = 0;
    if(rotateHeadIndex < 0) rotateHeadIndex = pixelCount;
    if(rotateTailIndex > pixelCount) rotateTailIndex = 0;
    if(rotateTailIndex < 0) rotateTailIndex = pixelCount;
  } else {

    strip.SetPixelColor(rotateTailIndex, color);
    strip.SetPixelColor(rotateHeadIndex, RgbColor(0,0,0));
    strip.Show();

    // Make it rotate in the Negative direction
    rotateHeadIndex --;
    rotateTailIndex --;

    if(rotateHeadIndex > pixelCount) rotateHeadIndex = 0;
    if(rotateHeadIndex < 0) rotateHeadIndex = pixelCount;
    if(rotateTailIndex > pixelCount) rotateTailIndex = 0;
    if(rotateTailIndex < 0) rotateTailIndex = pixelCount;
  }
  // Wait for BPM so it looks nice
  delay((60000 / abs(bpm))/ pixelCount);
}


int alternatingLength = 5;
int alternatingRotationIndex = 0;
// LEDs Chase around in alternating colors
void alternatingColorRotation(RgbColor colors[], int bpm){
  for (int i = 0; i < pixelCount; i++) {
    //Caculate what LEDs should be what color
    int newI = (i + alternatingRotationIndex);    
    if((newI / alternatingLength ) % 2 == 0){
        strip.SetPixelColor(i, colors[0]);
    }
    else
    {
      strip.SetPixelColor(i, colors[1]);
    }
  }
  // Move forward or backwards in the strip for the next call
  // If left too long in the same dirrection the number gets really big
  if(bpm > 0){
      alternatingRotationIndex --;
  }
  else{
    alternatingRotationIndex ++;
  }

  //Wait for the a BPM so it looks nice
  if(( (240000 / (abs(bpm) + 1))/ pixelCount) > 3000){
    delay(3000);
  } else {
  delay((240000 / (abs(bpm) + 1))/ pixelCount);
  }
  strip.Show();
}

// Set the entire Strip to Black
void setAllBlack(){
  RgbColor color(0,0,0);
  // Call every pixel
  for (uint16_t pixel = 0; pixel < pixelCount; pixel++)
    {
        //Set color to black
        strip.SetPixelColor(pixel, color);               
    }
  strip.Show();
}

// Artnet changes these so they run evenly if artnet gets interupted
RgbColor dmxColor;
int bpm = 1;
bool rotateSolidColorMode = false;
bool solidColorMode = false;
bool alternatingColorMode = false;

//Colors for alternating colors
RgbColor possibleColors[2][2] = { {RgbColor(255,0,0), RgbColor(255,255,255)},
                                  {RgbColor(0,255,0), RgbColor(0,0,255)}
};

int alternatingColorIndex = 0;


void loop(void){
  // Custom Wifi Configure loop Function
  wifiConfigure.loop();

  // Read all of the ArtNode information coming in  
  uint16_t code = artnetnode.read();
  if (code) {
    if (code == OpDmx) {

      // Reads channel 5 to determine what color to make Alternating colors
      if(artnetnode.getDmxFrame()[dmxAdress + 5] > 1){

        //Set all other modes to false
        rotateSolidColorMode = false;
        solidColorMode = false;

        //Set alternating color mode to true
        alternatingColorMode = true;

        //Subtract channel 3 - 4 to get amplitude and direction of BPM
        bpm = (artnetnode.getDmxFrame()[dmxAdress + 3]) - (artnetnode.getDmxFrame()[dmxAdress + 4]);

        // Take 0-255 that Artnet sends and map it to pick one of the colors in the possible colors
        alternatingColorIndex = map(artnetnode.getDmxFrame()[dmxAdress + 5], 0, 255, 0, 2);

      } else

      // Reads channel 3 to determine how much in the possitive direction BPM is
      // Also sets rotateSolidColorMode on
      if(artnetnode.getDmxFrame()[dmxAdress + 3] > 1) {
        rotateSolidColorMode = true;

        // Sets all other modes to false after going black to reset all pixels
        if(solidColorMode){
          setAllBlack();
          solidColorMode = false;
        }
        if(alternatingColorMode){
          setAllBlack();
          alternatingColorMode = false;
        }

        // Sets the dmxColor that the pixels will use based off of R G B from channels 0 1 2
        dmxColor = RgbColor(artnetnode.getDmxFrame()[dmxAdress], artnetnode.getDmxFrame()[dmxAdress + 1], artnetnode.getDmxFrame()[dmxAdress + 2]);

        //Subtract channel 3 - 4 to get amplitude and direction of BPM
        bpm = artnetnode.getDmxFrame()[dmxAdress + 3] - artnetnode.getDmxFrame()[dmxAdress + 4];

      } else 
      // Reads channel 4 to determine how much in the negative direction BPM is
      if(artnetnode.getDmxFrame()[dmxAdress + 4] > 1) {
        rotateSolidColorMode = true;

        
        // Sets all other modes to false after going black to reset all pixels
        if(solidColorMode){
          setAllBlack();
          solidColorMode = false;
        }
        if(alternatingColorMode){
          setAllBlack();
          alternatingColorMode = false;
        }
             
        // Sets the dmxColor that the pixels will use based off of R G B from channels 0 1 2
        dmxColor = RgbColor(artnetnode.getDmxFrame()[dmxAdress], artnetnode.getDmxFrame()[dmxAdress + 1], artnetnode.getDmxFrame()[dmxAdress + 2]);

        //Subtract channel 3 - 4 to get amplitude and direction of BPM
        bpm = artnetnode.getDmxFrame()[dmxAdress + 3] - artnetnode.getDmxFrame()[dmxAdress + 4];
      } 
      // Runs when channels 3,4,5 are 0
      else {
        solidColorMode = true;

        // Sets all other modes to false after going black to reset all pixels
        if(alternatingColorMode){
          setAllBlack();
          alternatingColorMode = false;
        }
        if(rotateSolidColorMode){
          setAllBlack();
          rotateSolidColorMode = false;
        }

        // Sets the dmxColor that the pixels will use based off of R G B from channels 0 1 2
        dmxColor = RgbColor(artnetnode.getDmxFrame()[dmxAdress], artnetnode.getDmxFrame()[dmxAdress + 1], artnetnode.getDmxFrame()[dmxAdress + 2]);

      }      
    } // End of code == OpDmx
  } // End of code AKA artnetnode.read()

  // Figure out what mode is on and run it
  // Moved outside artnetnode.read() because this runs every loop to make it smoother
  if(rotateSolidColorMode) rotateSolidColor(dmxColor, bpm);
  else if(alternatingColorMode) alternatingColorRotation(possibleColors[alternatingColorIndex], bpm);
  else updateSolidColors(dmxColor.R, dmxColor.G, dmxColor.B);
  //Handle client requests
  server.handleClient();

}