         /////////////////////////////////////////////  
        //     AI-driven LoRaWAN Fertilizer        //
       //     Pollution Detector w/ WhatsApp      //
      //             ---------------             //
     //             (Arduino Nano)              //           
    //             by Kutluhan Aktar           // 
   //                                         //
  /////////////////////////////////////////////

//
// Collect data to train a NN to detect fertilizer contamination. Then, get informed of the results via WhatsApp over LoRaWAN.
//
// For more information:
// https://www.theamplituhedron.com/projects/AI_driven_LoRaWAN_Fertilizer_Pollution_Detector_w_WhatsApp/
//
//
// Connections
// Arduino Nano :
//                                SH1106 OLED Display (128x64)
// D11  --------------------------- SDA
// D13  --------------------------- SCK
// D8   --------------------------- RST
// D9   --------------------------- DC
// D10  --------------------------- CS
//                                Control Button (A)
// A0   --------------------------- +
//                                Control Button (B)
// A1   --------------------------- +
//                                Control Button (C)
// A2   --------------------------- +
//                                5mm Common Anode RGB LED
// D3   --------------------------- R
// D5   --------------------------- G
// D6   --------------------------- B  


// Include the required libraries:
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>

// Define the SH1106 screen settings:
#define OLED_MOSI      11  // MOSI (SDA)
#define OLED_CLK       13  // SCK
#define OLED_DC        9
#define OLED_CS        10
#define OLED_RESET     8
Adafruit_SH1106 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

// Define monochrome graphics:
static const unsigned char PROGMEM _home [] {
0x00, 0x00, 0x00, 0x00, 0x00, 0x7E, 0x01, 0xF8, 0x07, 0xC0, 0x7E, 0x81, 0xFA, 0x07, 0xC0, 0x7E,
0xC1, 0xFB, 0x07, 0xC8, 0x66, 0x01, 0x98, 0x06, 0x40, 0x66, 0x01, 0x98, 0x06, 0x60, 0x7F, 0xE1,
0xFF, 0x87, 0xFE, 0x60, 0x61, 0x81, 0x86, 0x06, 0x7F, 0xE1, 0xFF, 0x87, 0xFE, 0x60, 0x61, 0x81,
0x86, 0x06, 0x60, 0x61, 0x81, 0x86, 0x06, 0x7F, 0xE1, 0xFF, 0x87, 0xFE, 0x60, 0x61, 0xA5, 0x86,
0x06, 0x7F, 0xE1, 0xFF, 0x87, 0xFE, 0x7F, 0xE1, 0xFF, 0x87, 0xFE, 0x02, 0x00, 0x18, 0x00, 0x40,
0x03, 0x00, 0x18, 0x00, 0xC0, 0x01, 0x00, 0x18, 0x00, 0x80, 0x01, 0xF8, 0x18, 0x1F, 0x80, 0x00,
0x0C, 0x18, 0x30, 0x00, 0x00, 0x04, 0x18, 0x20, 0x00, 0x00, 0x15, 0x19, 0xA8, 0x00, 0x00, 0x0F,
0x18, 0xF0, 0x00, 0x00, 0x06, 0x18, 0x60, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x24,
0x00, 0x00, 0x00, 0x01, 0xE7, 0x80, 0x00, 0x00, 0x1F, 0xA5, 0xF8, 0x00, 0x00, 0xFF, 0xC3, 0xFF,
0x00, 0x03, 0xFF, 0xE7, 0xFF, 0xC0, 0x00, 0x7F, 0xFF, 0xFE, 0x00, 0x07, 0x0F, 0xFF, 0xF0, 0xE0,
0x07, 0xE0, 0xFF, 0x07, 0xE0, 0x07, 0xFE, 0x18, 0x7F, 0xE0, 0x07, 0xFF, 0xC3, 0xFF, 0xE0, 0x00,
0x7F, 0xE7, 0xFE, 0x00, 0x00, 0x0F, 0xE7, 0xF0, 0x00, 0x00, 0x00, 0xE7, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
};
static const unsigned char PROGMEM enriched [] = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x7F, 0xC0, 0x00, 0x00, 0x00, 0x3F, 0xF8, 0x00, 0x00, 0x00, 0x3F,
0xFE, 0x00, 0x00, 0x00, 0x3F, 0xFF, 0x80, 0x00, 0x00, 0x3F, 0xFF, 0x80, 0x0F, 0xFE, 0x3F, 0xBF,
0xC0, 0x3F, 0xFE, 0x3F, 0xDF, 0xC0, 0xFF, 0xBC, 0x1F, 0xCF, 0xC0, 0xFE, 0x7C, 0x1F, 0xE7, 0xC1,
0xFD, 0xFC, 0x1F, 0xF7, 0xC1, 0xF3, 0xF8, 0x0F, 0xFB, 0xC1, 0xE7, 0xF8, 0x07, 0xF9, 0xC1, 0xDF,
0xF0, 0x03, 0xFD, 0xC1, 0x9F, 0xF0, 0x00, 0xFC, 0x83, 0x3F, 0xE0, 0x00, 0x00, 0xC6, 0x7F, 0x80,
0x00, 0x00, 0x44, 0x00, 0x00, 0x00, 0x00, 0x6C, 0x00, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00,
0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x03,
0xBB, 0x80, 0x00, 0x00, 0x1F, 0xB9, 0xF8, 0x00, 0x00, 0x38, 0x18, 0x1C, 0x00, 0x00, 0x70, 0x18,
0x0E, 0x00, 0x00, 0x3C, 0x00, 0x3C, 0x00, 0x00, 0x3F, 0xFF, 0xFC, 0x00, 0x00, 0x3F, 0xFF, 0xFC,
0x00, 0x00, 0x1F, 0xFF, 0xF8, 0x00, 0x00, 0x1F, 0xFF, 0xF8, 0x00, 0x00, 0x1F, 0xFF, 0xF8, 0x00,
0x00, 0x0F, 0xFF, 0xF0, 0x00, 0x00, 0x0F, 0xFF, 0xF0, 0x00, 0x00, 0x0F, 0xFF, 0xF0, 0x00, 0x00,
0x07, 0xFF, 0xE0, 0x00, 0x00, 0x07, 0xFF, 0xE0, 0x00, 0x00, 0x07, 0xFF, 0xE0, 0x00, 0x00, 0x03,
0xFF, 0xC0, 0x00, 0x00, 0x00, 0x3C, 0x00, 0x00, 
};
static const unsigned char PROGMEM unsafe [] = {
0x10, 0x00, 0x3C, 0x0C, 0x00, 0x38, 0x01, 0xFF, 0x8E, 0x00, 0x18, 0x03, 0xFF, 0xCC, 0x00, 0x02,
0x73, 0xFF, 0x80, 0x00, 0x00, 0xF9, 0xFF, 0xBC, 0x00, 0x01, 0xF9, 0xFF, 0xBE, 0x00, 0x01, 0xF9,
0xFF, 0xBC, 0x80, 0x00, 0xF9, 0x81, 0x9C, 0x30, 0x00, 0x73, 0x3E, 0xC0, 0x00, 0x01, 0x07, 0x00,
0xFC, 0x00, 0x03, 0x27, 0x00, 0xFC, 0x00, 0x06, 0x21, 0xBD, 0xF0, 0x00, 0x04, 0x2D, 0x81, 0xE6,
0x00, 0x10, 0x2D, 0xBD, 0xEF, 0x4C, 0x30, 0x21, 0xBD, 0xEF, 0x08, 0x00, 0x1F, 0x8D, 0xE6, 0x00,
0x00, 0x1F, 0xBD, 0xF0, 0x00, 0x00, 0x0F, 0x8D, 0xF2, 0x00, 0x00, 0x07, 0x8D, 0xE2, 0x00, 0x00,
0x03, 0xBD, 0xC0, 0x00, 0x00, 0xF8, 0x8D, 0x18, 0x00, 0x03, 0xDE, 0x3C, 0x7B, 0x40, 0x1F, 0xDF,
0xBD, 0xFC, 0x70, 0x7C, 0xFD, 0xBD, 0xBF, 0xFE, 0x1F, 0xFF, 0x9D, 0xBF, 0xF8, 0xC7, 0xDF, 0xC3,
0xEC, 0xE3, 0x71, 0xFF, 0xE7, 0xEF, 0x8F, 0x7C, 0x7C, 0xFE, 0x7E, 0x3F, 0x7F, 0x1F, 0xFF, 0xF8,
0xF7, 0x4F, 0xC3, 0xE7, 0xC3, 0xF7, 0x7F, 0xB8, 0xFF, 0x1F, 0xFE, 0x1F, 0xBE, 0x3C, 0x7C, 0xFC,
0x07, 0xFF, 0x81, 0xFF, 0xE0, 0x01, 0xF7, 0xE7, 0xA7, 0x80, 0x00, 0x77, 0xFF, 0xBE, 0x00, 0x00,
0x1F, 0xBF, 0xF8, 0x00, 0x00, 0x07, 0xBB, 0xE0, 0x00, 0x00, 0x01, 0xFF, 0x80, 0x00, 0x00, 0x00,
0x3E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
};
static const unsigned char PROGMEM toxic [] = {
0x00, 0x00, 0x81, 0x00, 0x00, 0x00, 0x03, 0x00, 0xC0, 0x00, 0x00, 0x06, 0x00, 0x60, 0x00, 0x00,
0x0C, 0x00, 0x30, 0x00, 0x00, 0x18, 0x00, 0x18, 0x00, 0x00, 0x38, 0x00, 0x1C, 0x00, 0x00, 0x30,
0x00, 0x0C, 0x00, 0x00, 0x70, 0x00, 0x0E, 0x00, 0x00, 0x70, 0x00, 0x0E, 0x00, 0x00, 0x70, 0x00,
0x0E, 0x00, 0x00, 0x70, 0x7E, 0x0E, 0x00, 0x00, 0x71, 0xFF, 0x8E, 0x00, 0x00, 0x73, 0xFF, 0xCE,
0x00, 0x00, 0x7B, 0x81, 0xDE, 0x00, 0x00, 0xFD, 0x00, 0xBF, 0x00, 0x01, 0xFC, 0x00, 0x3F, 0x80,
0x07, 0xFF, 0x00, 0xFF, 0xE0, 0x0F, 0xFF, 0xC3, 0xFF, 0xF0, 0x0F, 0xFF, 0xFF, 0xFF, 0xF0, 0x1E,
0x07, 0xE7, 0xE0, 0x78, 0x18, 0x31, 0xC3, 0x8C, 0x18, 0x30, 0x30, 0xC3, 0x0C, 0x0C, 0x30, 0x30,
0x00, 0x0C, 0x0C, 0x20, 0x30, 0x24, 0x0C, 0x04, 0x60, 0x38, 0x3C, 0x1C, 0x06, 0x40, 0x38, 0x3C,
0x1C, 0x02, 0x40, 0x1C, 0x3C, 0x38, 0x02, 0x40, 0x1C, 0x3C, 0x38, 0x02, 0x40, 0x0F, 0x3C, 0xF0,
0x02, 0x00, 0x07, 0xBD, 0xE0, 0x00, 0x00, 0x03, 0xBD, 0xC0, 0x00, 0x00, 0x01, 0x7E, 0x80, 0x00,
0x00, 0x00, 0x7E, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x08, 0x01, 0xFF, 0x80, 0x10, 0x04,
0x03, 0xFF, 0xC0, 0x20, 0x03, 0x9F, 0xE7, 0xF9, 0xC0, 0x00, 0xFF, 0x81, 0xFF, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
};

// Define the fertilizer hazard class (label) names and color codes.
String classes[] = {"Enriched", "Unsafe", "Toxic"};
int color_codes[3][3] = {{0,255,0}, {255,255,0}, {255,0,0}};

// Define the RGB LED pins:
#define redPin     3
#define greenPin   5
#define bluePin    6

// Define the control button pins:
#define button_A   A0
#define button_B   A1
#define button_C   A2

void setup(){
  Serial.begin(115200);

  pinMode(button_A, INPUT_PULLUP);
  pinMode(button_B, INPUT_PULLUP);
  pinMode(button_C, INPUT_PULLUP);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  adjustColor(0,0,0);

  // Initialize the SH1106 screen:
  display.begin(SH1106_SWITCHCAPVCC);
  display.display();
  delay(1000);

  // If successful:  
  display.clearDisplay();   
  display.setTextSize(2); 
  display.setTextColor(BLACK, WHITE);
  display.setCursor(0,0);
  display.println("AIoT");
  display.println("Fertilizer");
  display.println("Detector");
  display.display();
  delay(2000);
  adjustColor(255,0,255);
}

void loop(){
  home_screen();

  // If one of the control buttons (A, B, or C) is pressed, transmit the selected fertilizer hazard class (label)
  // to LattePanda 3 Delta via serial communication.
  if(!digitalRead(button_A)){ Serial.println("Label: Enriched"); data_screen(0); delay(2000); }
  if(!digitalRead(button_B)){ Serial.println("Label: Unsafe"); data_screen(1); delay(2000); }
  if(!digitalRead(button_C)){ Serial.println("Label: Toxic"); data_screen(2); delay(2000); }

}

void home_screen(){
  display.clearDisplay();   
  display.drawBitmap((128 - 40), 20, _home, 40, 40, WHITE);
  display.setTextSize(1); 
  display.setTextColor(BLACK, WHITE);
  display.setCursor(10,5);
  display.println(" Select Label: ");
  display.setTextColor(WHITE);
  display.setCursor(10,25);
  display.println("A) Enriched");  
  display.setCursor(10,40);
  display.println("B) Unsafe");  
  display.setCursor(10,55);
  display.println("C) Toxic");  
  display.display();
  delay(100);
}

void data_screen(int i){
  display.clearDisplay(); 
  if(i==0) display.drawBitmap((128 - 40) / 2, 0, enriched, 40, 40, WHITE);
  if(i==1) display.drawBitmap((128 - 40) / 2, 0, unsafe, 40, 40, WHITE);
  if(i==2) display.drawBitmap((128 - 40) / 2, 0, toxic, 40, 40, WHITE);
  // Print:
  int str_x = classes[i].length() * 11;
  display.setTextSize(2); 
  display.setTextColor(WHITE);
  display.setCursor((128 - str_x) / 2, 48);
  display.println(classes[i]);
  display.display();
  adjustColor(color_codes[i][0], color_codes[i][1], color_codes[i][2]);
  delay(4000);
  adjustColor(255,0,255);
}

void adjustColor(int r, int g, int b){
  analogWrite(redPin, (255-r));
  analogWrite(greenPin, (255-g));
  analogWrite(bluePin, (255-b));
}
