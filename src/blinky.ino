/*
 * Project particle-neopixel-ring-blinky
 * Description: round blinky thing
 * Author: Gabe Conradi <gabe.conradi@gmail.com>
 * Date: idklol
 */
#include "FastLED.h"
#include "hsv.h"

#define PIXEL_COUNT 24
#define PIXEL_PIN 6
#define PIXEL_TYPE NSFastLED::NEOPIXEL
#define HUE_STEP 10 // 1..255, each loop increments hue by this value
#define LOOP_DELAY 100 //ms
#define HSV_BRIGHTNESS 10
#define HSV_SATURATION 255
#define BASE_BRIGHTNESS 2 //0-255
/* set this to match the number of patterns you flip
between when holding the setup button */
#define N_PATTERNS 5
#define SETUP_BUTTON_HOLD_DURATION 800 // ms

// PORNJ!!!! These values are selected to look decent on RBG LEDs
#define DISORIENT_PINK_R 252
#define DISORIENT_PINK_G 0
#define DISORIENT_PINK_B 210
#define DISORIENT_ORANGE_R 255
#define DISORIENT_ORANGE_G 68
#define DISORIENT_ORANGE_B 0
#define CRGB_DIS_PINK NSFastLED::CRGB(DISORIENT_PINK_R, DISORIENT_PINK_G, DISORIENT_PINK_B)
#define CRGB_DIS_ORAN NSFastLED::CRGB(DISORIENT_ORANGE_R, DISORIENT_ORANGE_G, DISORIENT_ORANGE_B)

// disorient_1 pattern params
#define DISORIENT_1_ILLUMINATION_PROBABILITY 60
#define DISORIENT_1_SPARKLE_PROBABILITY 5

NSFastLED::CRGB leds[PIXEL_COUNT];
uint8_t base_hue = 0;
uint8_t pattern = 0;
// this is ghetto debouncing, and will ignore input to the pattern button
// for X cycles of LOOP_DELAY once triggered
uint8_t ignore_button_cycles = 0;

NSFastLED::CFastLED* gLED; // global CFastLED object

// make sure we disable wifi cause we dont need that shit!
SYSTEM_MODE(MANUAL);

// setup() runs once, when the device is first turned on.
void setup() {
  Serial.println("Booting up");
  randomSeed(analogRead(0));
  WiFi.disconnect();
  gLED = new NSFastLED::CFastLED();
  gLED->addLeds<PIXEL_TYPE, PIXEL_PIN>(leds, PIXEL_COUNT);
  gLED->setBrightness(BASE_BRIGHTNESS);

  RGB.control(true); // take over the LED
  RGB.color(DISORIENT_PINK_R, DISORIENT_PINK_G, DISORIENT_PINK_B);
  Serial.println("Finished setup");
}

// pattern 0
void pattern_disorient_0(){
  long t = millis()*0.25;
  uint8_t offset = t % PIXEL_COUNT;
  uint8_t new_i = 0;
  for (int i = 0 ; i < PIXEL_COUNT ; ++i){
    new_i = (i + offset) % PIXEL_COUNT;
    if (i >= PIXEL_COUNT/2) {
      leds[new_i] = CRGB_DIS_PINK;
    } else {
      leds[new_i] = CRGB_DIS_ORAN;
    }
    if (random(100) <= 5) {
      // random sparkles
      leds[new_i] = NSFastLED::CRGB::White;
    }
  }
}

// pattern 1
void pattern_disorient_1(){
  // illumination probability
  // -> random sparkle probability (white)
  // -> 50/50 pink orange
  for (int i = 0 ; i < PIXEL_COUNT ; ++i){
    if (random(100) <= DISORIENT_1_ILLUMINATION_PROBABILITY) {
      // pixel is illuminated!
      if (random(100) <= DISORIENT_1_SPARKLE_PROBABILITY) {
        // pixel is sparkly
        leds[i] = NSFastLED::CRGB::White;
      } else {
        if (random(2) == 0) {
          leds[i] = CRGB_DIS_PINK;
        } else {
          leds[i] = CRGB_DIS_ORAN;
        }
      }
    } else {
      leds[i] = NSFastLED::CRGB::Black;
    }
  }
}

// pattern 2
void pattern_hsv_offset_circle_loop(){
  uint8_t offset = 0;
  NSFastLED::CRGB rgb;
  NSFastLED::CHSV hsv;
  for (int i = 0 ; i < PIXEL_COUNT ; ++i){
    offset = ((float)i/(PIXEL_COUNT-1))*255;
    hsv = NSFastLED::CHSV(base_hue+offset, HSV_SATURATION, HSV_BRIGHTNESS);
    NSFastLED::hsv2rgb_rainbow(hsv, rgb);
    leds[i] = rgb;
  }

  // set the center pixel to mirror what pixel 0 is
  hsv = NSFastLED::CHSV(base_hue+offset, HSV_SATURATION, HSV_BRIGHTNESS);
  hsv2rgb_rainbow(hsv, rgb);
  RGB.color(rgb.r, rgb.g, rgb.b);

  base_hue += HUE_STEP;
}

// pattern 3
void pattern_hsv_circle_loop(){
  NSFastLED::CHSV hsv = NSFastLED::CHSV(base_hue, HSV_SATURATION, HSV_BRIGHTNESS);
  NSFastLED::CRGB rgb;
  NSFastLED::hsv2rgb_rainbow(hsv, rgb);
  for (int i = 0 ; i < PIXEL_COUNT ; ++i){
    leds[i] = rgb;
  }
  // set the center pixel to mirror what pixel 0 is
  RGB.color(rgb.r, rgb.g, rgb.b);
  base_hue += HUE_STEP;
}

// pattern 4
void pattern_disorient_2(){
  // same as pattern 0, but in quadrants
  uint8_t offset_0, offset_1, offset_2, offset_3, offset_4;
  offset_0 = 0;
  offset_1 = PIXEL_COUNT/4;
  offset_2 = PIXEL_COUNT/2;
  offset_3 = offset_1+offset_2;
  long t = millis()*0.25;
  uint8_t offset = t % PIXEL_COUNT;
  uint8_t new_i = 0;
  for (int i = 0 ; i < PIXEL_COUNT ; ++i){
    new_i = (i + offset) % PIXEL_COUNT;
    if ((i >= offset_0 && i < offset_1) || (i >= offset_2 && i < offset_3)) {
      leds[new_i] = NSFastLED::CRGB(DISORIENT_PINK_R, DISORIENT_PINK_G, DISORIENT_PINK_B);
    } else {
      leds[new_i] = NSFastLED::CRGB(DISORIENT_ORANGE_R, DISORIENT_ORANGE_G, DISORIENT_ORANGE_B);
    }
    if (random(100) <= 5) {
      // random sparkles
      leds[new_i] = NSFastLED::CRGB::White;
    }
  }
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  if (HAL_Core_Mode_Button_Pressed(SETUP_BUTTON_HOLD_DURATION)) {
    if (ignore_button_cycles > 0) {
      --ignore_button_cycles;
      RGB.color(0, 255, 0); // green when ignoring press
    } else {
      RGB.color(255, 0, 0); // red on pattern change
      ignore_button_cycles = 10;
      ++pattern;
      if (pattern >= N_PATTERNS) {
        pattern = 0;
      }
    }
  } else {
    ignore_button_cycles = 0;
    RGB.color(DISORIENT_PINK_R, DISORIENT_PINK_G, DISORIENT_PINK_B);
  }

  if (pattern == 0){
    pattern_disorient_0();
  } else if (pattern == 1) {
    pattern_hsv_offset_circle_loop();
  } else if (pattern == 2) {
    pattern_disorient_1();
  } else if (pattern == 3) {
    pattern_hsv_circle_loop();
  } else if (pattern == 4) {
    pattern_disorient_2();
  }

  gLED->show();
  // TODO(gabe) this doesnt work apparently?
  gLED->delay(LOOP_DELAY);
  delay(LOOP_DELAY);
}
