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
const int arraySize = 128;            // Number of data points
const int inputPin = A1;              // Analog input pin
int readValue = analogRead(inputPin); // asign analog input
int dataArray[arraySize];             // Array to store input values
int userIntputLimit = 150;            // used to detect user input to break waitForInput

const float frequency = 2.0; // Frequency of the sine wave
const int amplitude = 128;   // Amplitude of the sine wave (0-1023 range)
const int offset = 0;      // Center of the sine wave (0-1023 range)

// display params
const uint16_t backgroundColor = ST77XX_WHITE;
const uint16_t textColor = ST77XX_BLACK;
int screenWidth;            // Get the display width (landscape mode)
int screenHeight;           // Get the display height (landscape mode)
int centerY;                // Vertical center of the display
int centerX;                // Vertical center of the display
int scaleY;                 // Map analog input to display height
const int sampleRate = 100; // loop rate

// plotting
int yPrev0 = 0;
int yPrev1 = 0;

// Variables for tracking runtime and loop executions
unsigned long loopStartTime;  // Time when each loop starts
unsigned long loopTimeTotal = 0;  // Sum of loop execution times
int loopCounter = 0;      // Counter for the number of times loop() has executed
float avgLoopTime = 0.0;  // Average loop time in milliseconds


void readAndPlotInput();
void shiftAndAdd(int);
void init_dataArray();
void waitForInput();
void plotParams();

void setup(void) {
  Serial.begin(9600);
  Serial.print("Width: ");
  Serial.println(tft.width());
  Serial.print("Height: ");
  Serial.println(tft.height());

  // Use this initializer if using a 1.8" TFT screen:
  //tft.initR(INITR_BLACKTAB);      // Init ST7735S chip, black tab

  // OR use this initializer if using a 1.8" TFT screen with offset such as WaveShare:
  // tft.initR(INITR_GREENTAB);      // Init ST7735S chip, green tab

  // OR use this initializer (uncomment) if using a 1.44" TFT:
  tft.initR(INITR_144GREENTAB);       // Init ST7735R chip, green tab
  tft.setRotation(0);                 // Set rotation: 0 = portrait, 1 = landscape
  tft.fillScreen(backgroundColor);    // Clear screen
  tft.setTextColor(textColor);        // Set text color

  screenWidth = tft.width();          // Get the display width (landscape mode)
  screenHeight = tft.height();        // Get the display height (landscape mode)
  centerY = screenHeight / 2;         // Vertical center of the display
  centerX = screenWidth / 2;          // Vertical center of the display
  scaleY = screenHeight / 100;        // Map analog input to display height
  
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
  plotParams();
  waitForInput();
  tft.fillScreen(backgroundColor); // Clear the screen
}

void loop() {    
  loopStartTime = millis(); // Record the start time of the loop  

if (loopCounter > 10) {  // Every 1000 ms (1 second)
    // Define the area to clear
    int textX = 5;         // X-coordinate for text
    int textY = 2;         // Y-coordinate for text
    int textWidth = 126;    // Width of the rectangle (adjust to fit your text)
    int textHeight = 10;   // Height of the rectangle (adjust to your text size)

    // Clear the area with a rectangle filled with the background color
    tft.fillRect(textX, textY, textWidth, textHeight, backgroundColor);

    // Display the average loop time
    tft.setCursor(textX, textY);  // Set cursor position
    tft.print("Avg Loop: ");
    tft.print(avgLoopTime);       // Display average loop time in milliseconds
    tft.println(" ms");
}

  // main functionality
  // --------------------------
  readAndPlotInput();
  delay(sampleRate); // Adjust for sampling rate
  // --------------------------
  
  // Calculate the loop execution time and add it to the total time
  loopTimeTotal += millis() - loopStartTime;  // Sum up loop times
  loopCounter++;
  // Calculate the average loop time
  avgLoopTime = (float)loopTimeTotal / loopCounter;  
}

void init_dataArray() {
  for (int i = 0; i < arraySize; i++) {
    float angle = (2.0 * PI * frequency * i) / arraySize; // Calculate angle for sine wave
    dataArray[i] = offset + amplitude * sin(angle);       // Calculate sine value
  }
}

void shiftAndAdd(int value) {
  // Shift all elements to the left
  for (int i = 0; i < arraySize - 1; i++) {
    dataArray[i] = dataArray[i + 1];
  }
  // Add new value to the end
  dataArray[arraySize - 1] = value;
}

void readAndPlotInput() {
  static int previousData[arraySize]; // Store the previous data values to clear lines correctly

  // Read input and add to the array
  readValue = analogRead(inputPin)/10; // Read analog input (0-1023 range)
  shiftAndAdd(readValue);               // Shift array and add new value


  // Draw horizontal center line
  tft.drawFastVLine(centerX, 0, screenHeight, ST77XX_RED);
  tft.drawFastHLine(0, centerY, screenWidth, ST77XX_RED);

  // Plot the data
  for (int i = 0; i < arraySize - 1; i++) {

    int x0 = map(i, 0, arraySize - 1, 0, screenWidth);  // Scale index to width
    int y0 = centerY - (dataArray[i] * scaleY);         // Scale data to height
    int x1 = map(i + 1, 0, arraySize - 1, 0, screenWidth);
    int y1 = centerY - (dataArray[i + 1] * scaleY);

    tft.drawLine(x0, yPrev0, x1, yPrev1, backgroundColor);
    tft.drawLine(x0, y0, x1, y1, ST77XX_RED);          // Draw line

    yPrev0 = y0;
    yPrev1 = y1;
  }
}

void plotParams() {
  tft.setCursor(0, 0);
  tft.fillScreen(backgroundColor);
  tft.setTextSize(1);
  tft.print("Height");
  tft.println(screenHeight);
  tft.setCursor(0, 10);
  tft.print("Width");
  tft.println(screenWidth);
}

// wait until the voltage input is changed by 5 or more
void waitForInput() {
  delay(100);
  int initValue = analogRead(inputPin);
  delay(100);
  readValue = analogRead(inputPin);
  int counter = 0;

  int _x = screenWidth - 50;
  int _y = screenHeight - 50;
  
  while(abs(initValue - readValue) < userIntputLimit) {
    _x = screenWidth - 50;
    _y = screenHeight - 50;
    counter++;
    readValue = analogRead(inputPin);

    // Display the counter on the bottom-right corner
    tft.setCursor(_x, _y); // Set cursor to bottom-right corner
    tft.fillRect(_x, _y, 50, 50, backgroundColor); // Clear previous count
    tft.setTextSize(1);
    tft.println(counter);
    _y+=10;
    tft.setCursor(_x, _y); // Set cursor to bottom-right corner
    tft.println(readValue);
    _y+=10;
    tft.setCursor(_x, _y); // Set cursor to bottom-right corner
    tft.println(initValue);
    _y+=10;
    tft.setCursor(_x, _y); // Set cursor to bottom-right corner
    tft.println(initValue - readValue);

    delay(1000);
  }

}