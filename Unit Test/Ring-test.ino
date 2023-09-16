//---------------- Ring test -----------------------
#include <Adafruit_NeoPixel.h>

#define PIN            12  // Define the pin connected to your Neopixel data input
#define NUMPIXELS      16  // Define the number of Neopixels in your strip

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

void loop() {
  // Test pattern: Rainbow Cycle
  rainbowCycle(20);

  // Test pattern: Theater Chase
  theaterChase(0xFF, 0x00, 0x00, 50); // Red
  theaterChase(0x00, 0xFF, 0x00, 50); // Green
  theaterChase(0x00, 0x00, 0xFF, 50); // Blue
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t color, int wait) {
  for(int i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, color);
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(int wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

//Theatre-style crawling lights
void theaterChase(uint8_t r, uint8_t g, uint8_t b, int wait) {
  for(int j=0; j<10; j++) {  // Repeat 10 times
    for(int q=0; q < 3; q++) {
      for(int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, r, g, b);  // Turn every third pixel on
      }
      strip.show();

      delay(wait);

      for(int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0,0,0);      // Turn every third pixel off
      }
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
    return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
    WheelPos -= 170;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}