/**************************************************************************
  This is a library for several Adafruit displays based on ST77* drivers.

 
  Check out the links above for our tutorials and wiring diagrams.
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional).

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 **************************************************************************/

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
// #include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>


// For the breakout board, you can use any 2 or 3 pins.
// These pins will also work for the 1.8" TFT shield.
#define TFT_CS        10
#define TFT_RST        9 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC         8


// OPTION 1 (recommended) is to use the HARDWARE SPI pins, which are unique
// to each board and not reassignable. For Arduino Uno: MOSI = pin 11 and
// SCLK = pin 13. This is the fastest mode of operation and is required if
// using the breakout board's microSD card.

// For 1.44" and 1.8" TFT with ST7735 use:
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// For 1.14", 1.3", 1.54", 1.69", and 2.0" TFT with ST7789:
//Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);


// OPTION 2 lets you interface the display using ANY TWO or THREE PINS,
// tradeoff being that performance is not as fast as hardware SPI above.
//#define TFT_MOSI 11  // Data out
//#define TFT_SCLK 13  // Clock out

// For ST7735-based displays, we will use this call
//Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

// OR for the ST7789-based displays, we will use this call
//Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

// Configuration
const int arraySize = 128;    // Number of data points
const int inputPin = A1;      // Analog input pin
int dataArray[arraySize];     // Array to store input values

const float frequency = 1.0; // Frequency of the sine wave
const int amplitude = 512;   // Amplitude of the sine wave (0-1023 range)
const int offset = 512;      // Center of the sine wave (0-1023 range)

void readAndPlotInput();
void shiftAndAdd(int);
void init_dataArray();

void setup(void) {
  Serial.begin(9600);
  Serial.print(F("Hello! ST77xx TFT Test"));

  // Use this initializer if using a 1.8" TFT screen:
  //tft.initR(INITR_BLACKTAB);      // Init ST7735S chip, black tab

  // OR use this initializer if using a 1.8" TFT screen with offset such as WaveShare:
  // tft.initR(INITR_GREENTAB);      // Init ST7735S chip, green tab

  // OR use this initializer (uncomment) if using a 1.44" TFT:
  tft.initR(INITR_144GREENTAB);   // Init ST7735R chip, green tab
  tft.setRotation(1);             // Set rotation: 0 = portrait, 1 = landscape
  tft.fillScreen(ST77XX_BLACK);   // Clear screen
  tft.setTextColor(ST77XX_WHITE); // Set text color

  
  // SPI speed defaults to SPI_DEFAULT_FREQ defined in the library, you can override it here
  // Note that speed allowable depends on chip and quality of wiring, if you go too fast, you
  // may end up with a black screen some times, or all the time.
  // tft.setSPISpeed(40000000);

  // Serial.println(F("Initialized"));

  // uint16_t time = millis();
  // tft.fillScreen(ST77XX_BLACK);
  // time = millis() - time;

  // Serial.println(time, DEC);
  // delay(500);

  // tft print function!
  // tftPrintTest();
  // delay(2000);

  // tft.setCursor(0, 0);
  // Serial.println("done");
  // delay(1000);

  // Initialize array
  init_dataArray();
}

void loop() {  

  // Clear the screen
  tft.fillScreen(ST77XX_BLACK);
  readAndPlotInput();
  delay(50); // Adjust for sampling rate
}

void init_dataArray() {
  for (int i = 0; i < arraySize; i++) {
    float angle = (2.0 * PI * frequency * i) / arraySize; // Calculate angle for sine wave
    dataArray[i] = offset + amplitude * sin(angle);       // Calculate sine value
  }
}


void readAndPlotInput() {
  int screenWidth = tft.width();  // Get the display width (landscape mode)
  int screenHeight = tft.height(); // Get the display height (landscape mode)
  int centerY = screenHeight / 2;  // Vertical center of the display
  int scaleY = screenHeight / 100; // Map analog input to display height
  
  // Read input and add to the array
  int newValue = analogRead(inputPin); // Read analog input (0-1023 range)
  shiftAndAdd(newValue);               // Shift array and add new value


  // Draw horizontal center line
  tft.drawFastHLine(0, centerY, screenWidth, ST77XX_WHITE);

  // Draw bounding box
  tft.drawFastHLine(0, centerY, screenWidth, ST77XX_WHITE);

  // Plot the data
  for (int i = 0; i < arraySize - 1; i++) {
    int x1 = map(i, 0, arraySize - 1, 0, screenWidth);  // Scale index to width
    int y1 = centerY - (dataArray[i] * scaleY);         // Scale data to height
    int x2 = map(i + 1, 0, arraySize - 1, 0, screenWidth);
    int y2 = centerY - (dataArray[i + 1] * scaleY);
    tft.drawLine(x1, y1, x2, y2, ST77XX_RED);          // Draw line
  }
}

void shiftAndAdd(int newValue) {
  // Shift all elements to the left
  for (int i = 0; i < arraySize - 1; i++) {
    dataArray[i] = dataArray[i + 1];
  }
  // Add new value to the end
  dataArray[arraySize - 1] = newValue;
}