/*
 * Project particle-neopixel-ring-blinky
 * Description:
 * Author:
 * Date:
 */
#include "neopixel/neopixel.h"
#include "hsv.h"

#define PIXEL_COUNT 24
#define PIXEL_PIN D6
#define PIXEL_TYPE WS2812B
#define HUE_STEP 10 // 1..255, each loop increments hue by this value
#define LOOP_DELAY 100 //ms

Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);
uint8_t base_hue = 0;
uint8_t base_brightness = 100; //0-255
uint8_t base_saturation = 255;

// setup() runs once, when the device is first turned on.
void setup() {
  strip.begin();
  strip.show();
}

void pattern_hsv_circle_loop(Adafruit_NeoPixel *strip){
  HsvColor hsv(base_hue, base_saturation, base_brightness);
  RgbColor rgb = HsvToRgb(hsv);
  for (int i = 0 ; i < PIXEL_COUNT ; ++i){
    (*strip).setPixelColor(i, rgb.r, rgb.g, rgb.b);
  }
  base_hue += HUE_STEP;
}

void pattern_hsv_offset_circle_loop(Adafruit_NeoPixel *strip){
  uint8_t offset = 0;
  RgbColor rgb;
  HsvColor hsv;
  for (int i = 0 ; i < PIXEL_COUNT ; ++i){
    offset = ((float)i/(PIXEL_COUNT-1))*255;
    hsv = HsvColor(base_hue+offset, base_saturation, base_brightness);
    rgb = HsvToRgb(hsv);
    (*strip).setPixelColor(i, rgb.r, rgb.g, rgb.b);
  }
  base_hue += HUE_STEP;
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  pattern_hsv_offset_circle_loop(&strip);
  strip.show();
  delay(LOOP_DELAY);
}
