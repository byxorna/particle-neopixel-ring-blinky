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
#define HSV_BRIGHTNESS 60 //0-255
#define HSV_SATURATION 255
#define BASE_BRIGHTNESS 40 //0-255
/* set this to match the number of patterns you flip
between when holding the setup button */
#define N_PATTERNS 4
#define SETUP_BUTTON_HOLD_DURATION 500

// PORNJ!!!! These values are selected to look decent on RBG LEDs
#define DISORIENT_PINK_R 252
#define DISORIENT_PINK_G 0
#define DISORIENT_PINK_B 210
#define DISORIENT_ORANGE_R 255
#define DISORIENT_ORANGE_G 68
#define DISORIENT_ORANGE_B 0

// disorient_1 pattern params
#define DISORIENT_1_ILLUMINATION_PROBABILITY 60
#define DISORIENT_1_SPARKLE_PROBABILITY 10 // 90% is pink/orange split

Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);
uint8_t base_hue = 0;
uint8_t pattern = 0;

// setup() runs once, when the device is first turned on.
void setup() {
  randomSeed(analogRead(0));
  strip.begin();
  strip.show();
}

// pattern 0
void pattern_disorient_half_circle_rotate(Adafruit_NeoPixel *strip){
  long t = millis()*0.25;
  uint8_t offset = t % PIXEL_COUNT;
  uint8_t new_i = 0;
  for (int i = 0 ; i < PIXEL_COUNT ; ++i){
    new_i = (i + offset) % PIXEL_COUNT;
    if (i >= PIXEL_COUNT/2) {
      (*strip).setPixelColor(new_i, DISORIENT_PINK_R, DISORIENT_PINK_G, DISORIENT_PINK_B);
    } else {
      (*strip).setPixelColor(new_i, DISORIENT_ORANGE_R, DISORIENT_ORANGE_G, DISORIENT_ORANGE_B);
    }
    if (random(100) <= 5) {
      // random sparkles
      (*strip).setPixelColor(new_i, 255, 255, 255);
    }
  }
  (*strip).setBrightness(BASE_BRIGHTNESS);
  (*strip).show();
}

// pattern 1
void pattern_hsv_offset_circle_loop(Adafruit_NeoPixel *strip){
  uint8_t offset = 0;
  RgbColor rgb;
  HsvColor hsv;
  for (int i = 0 ; i < PIXEL_COUNT ; ++i){
    offset = ((float)i/(PIXEL_COUNT-1))*255;
    hsv = HsvColor(base_hue+offset, HSV_SATURATION, HSV_BRIGHTNESS);
    rgb = HsvToRgb(hsv);
    (*strip).setPixelColor(i, rgb.r, rgb.g, rgb.b);
  }
  base_hue += HUE_STEP;
  (*strip).show();
}

// pattern 2
void pattern_hsv_circle_loop(Adafruit_NeoPixel *strip){
  HsvColor hsv(base_hue, HSV_SATURATION, HSV_BRIGHTNESS);
  RgbColor rgb = HsvToRgb(hsv);
  for (int i = 0 ; i < PIXEL_COUNT ; ++i){
    (*strip).setPixelColor(i, rgb.r, rgb.g, rgb.b);
  }
  base_hue += HUE_STEP;
  (*strip).show();
}

// pattern 3
void pattern_disorient_1(Adafruit_NeoPixel *strip){
  // illumination probability
  // -> random sparkle probability (white)
  // -> 50/50 pink orange
  for (int i = 0 ; i < PIXEL_COUNT ; ++i){
    if (random(100) <= DISORIENT_1_ILLUMINATION_PROBABILITY) {
      // pixel is illuminated!
      if (random(100) <= DISORIENT_1_SPARKLE_PROBABILITY) {
        // pixel is sparkly
        (*strip).setPixelColor(i, 255, 255, 255);
      } else {
        if (random(2) == 0) {
          (*strip).setPixelColor(i, DISORIENT_PINK_R, DISORIENT_PINK_G, DISORIENT_PINK_B);
        } else {
          (*strip).setPixelColor(i, DISORIENT_ORANGE_R, DISORIENT_ORANGE_G, DISORIENT_ORANGE_B);
        }
      }
    } else {
      (*strip).setPixelColor(i, 0, 0, 0);
    }
  }
  (*strip).setBrightness(BASE_BRIGHTNESS);
  (*strip).show();
}



// loop() runs over and over again, as quickly as it can execute.
void loop() {
  if (HAL_Core_Mode_Button_Pressed(SETUP_BUTTON_HOLD_DURATION)) {
    ++pattern;
    if (pattern >= N_PATTERNS) {
      pattern = 0;
    }
  }
  if (pattern == 0){
    pattern_disorient_half_circle_rotate(&strip);
  } else if (pattern == 1) {
    pattern_hsv_offset_circle_loop(&strip);
  } else if (pattern == 2) {
    pattern_hsv_circle_loop(&strip);
  } else if (pattern == 3) {
    pattern_disorient_1(&strip);
  }

  delay(LOOP_DELAY);
}
