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
#define HUE_STEP 2 // 1..255, each loop increments hue by this value
#define LOOP_DELAY_MS 50 //ms
/* set this to match the number of patterns you flip
between when holding the setup button */
#define N_PATTERNS 3
#define SETUP_BUTTON_HOLD_DURATION_MS 800

// PORNJ!!!! These values are selected to look decent on RBG LEDs
#define DISORIENT_PINK_R 252
#define DISORIENT_PINK_G 0
#define DISORIENT_PINK_B 210
#define DISORIENT_ORANGE_R 255
#define DISORIENT_ORANGE_G 68
#define DISORIENT_ORANGE_B 0
#define CRGB_DIS_PINK NSFastLED::CRGB(DISORIENT_PINK_R, DISORIENT_PINK_G, DISORIENT_PINK_B)
#define CRGB_DIS_ORAN NSFastLED::CRGB(DISORIENT_ORANGE_R, DISORIENT_ORANGE_G, DISORIENT_ORANGE_B)

NSFastLED::CRGB leds[PIXEL_COUNT];
uint8_t base_hue = 0;
uint8_t pattern = 0;
unsigned long gBrightness = 10;
// this is ghetto debouncing, and will ignore input to the pattern button
// for X cycles of LOOP_DELAY_MS once triggered
uint8_t ignore_button_cycles = 0;

NSFastLED::CFastLED* gLED; // global CFastLED object

// for palette support
uint8_t gAnimIndex = 0;
uint8_t gPalette = 0;
NSFastLED::CRGBPalette16 currentPalette = NSFastLED::RainbowColors_p;
NSFastLED::TBlendType currentBlending = NSFastLED::LINEARBLEND;

// make sure we disable wifi cause we dont need that shit!
SYSTEM_MODE(SEMI_AUTOMATIC);

// setup() runs once, when the device is first turned on.
void setup() {
  Serial.println("Booting up");
  randomSeed(analogRead(0));
  gLED = new NSFastLED::CFastLED();
  gLED->addLeds<PIXEL_TYPE, PIXEL_PIN>(leds, PIXEL_COUNT);
  gLED->setBrightness(gBrightness);

  RGB.control(true); // take over the LED
  RGB.color(DISORIENT_PINK_R, DISORIENT_PINK_G, DISORIENT_PINK_B);
  Serial.println("Finished setup");
}

// pattern 0
void pattern_disorient_0(){
  long t = millis()*0.01;
  uint8_t offset = t % PIXEL_COUNT;
  uint8_t new_i = 0;
  for (int i = 0 ; i < PIXEL_COUNT ; ++i){
    new_i = (i + offset) % PIXEL_COUNT;
    if (random(PIXEL_COUNT*2) == 0) {
      leds[new_i] = NSFastLED::CRGB::White;
    } else {
      if (i >= PIXEL_COUNT/2) {
        leds[new_i] = CRGB_DIS_PINK;
      } else {
        leds[new_i] = CRGB_DIS_ORAN;
      }
    }
  }
}

// random disorient sparkles
void pattern_disorient_sparkles(){
  for (int i = 0 ; i < PIXEL_COUNT ; ++i){
    if (random(PIXEL_COUNT*3) == 0) {
      // pixel is sparkly
      leds[i] = NSFastLED::CRGB::White;
    } else {
      if (random(2) == 0) {
        leds[i] = CRGB_DIS_PINK;
      } else {
        leds[i] = CRGB_DIS_ORAN;
      }
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
    hsv = NSFastLED::CHSV(base_hue+offset, 255, 255);
    NSFastLED::hsv2rgb_rainbow(hsv, rgb);
    //if (random(PIXEL_COUNT*2) == 0) {
    //  leds[i] = NSFastLED::CRGB::White;
    //} else {
    leds[i] = rgb;
    //}
  }

  // set the center pixel to mirror what pixel 0 is
  hsv = NSFastLED::CHSV(base_hue+offset, 255, 255);
  hsv2rgb_rainbow(hsv, rgb);
  RGB.color(rgb.r, rgb.g, rgb.b);

  base_hue += HUE_STEP;
}

// cycle thru color palettes
void pattern_color_palettes() {
  for( int i = 0; i < PIXEL_COUNT; ++i) {
    uint8_t pIndex = (i + 6*i*PIXEL_COUNT/PIXEL_COUNT + gAnimIndex)%256;
    leds[i] = NSFastLED::ColorFromPalette(currentPalette, pIndex, 255, currentBlending);
  }
  gAnimIndex = (gAnimIndex+3)%256;
}

void pattern_color_palettes_with_sparkle() {
  for( int i = 0; i < PIXEL_COUNT; ++i) {
    if (random(PIXEL_COUNT*2) == 0) {
      leds[i] = NSFastLED::CRGB::White;
    } else {
      uint8_t pIndex = (i + 6*i*PIXEL_COUNT/PIXEL_COUNT + gAnimIndex)%256;
      leds[i] = NSFastLED::ColorFromPalette(currentPalette, pIndex, 255, currentBlending);
    }
  }
  gAnimIndex = (gAnimIndex+3)%256;
}


// rainbow pulsing with varied breathing cycles
void pattern_phase_rainbow_pulse() {
  uint8_t cBrightness = NSFastLED::beatsin8(128, 0, 255);
  NSFastLED::CHSV hsv = NSFastLED::CHSV(base_hue, 255, cBrightness);
  NSFastLED::CRGB rgb;
  NSFastLED::hsv2rgb_rainbow(hsv, rgb);
  for (int i = 0 ; i < PIXEL_COUNT ; ++i){
    leds[i] = rgb;
  }
  // set the center pixel to mirror what pixel 0 is
  RGB.color(rgb.r, rgb.g, rgb.b);
  base_hue += 2;
}

// pattern 3
void pattern_hsv_circle_loop(){
  NSFastLED::CHSV hsv = NSFastLED::CHSV(base_hue, 255, 255);
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
  uint8_t offset_0, offset_1, offset_2, offset_3;
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
  if (HAL_Core_Mode_Button_Pressed(SETUP_BUTTON_HOLD_DURATION_MS)) {
    if (ignore_button_cycles > 0) {
      --ignore_button_cycles;
      RGB.color(0, 255, 0); // green when ignoring press
    } else {
      RGB.color(255, 0, 0); // red on pattern change
      ignore_button_cycles = 20;
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
    pattern_color_palettes_with_sparkle();
  }

  gLED->setBrightness(gBrightness);
  gLED->show();
  // TODO(gabe) this doesnt work apparently? Contraray to documentation,
  // FastLED::delay() doesnt perform any delay...
  //gLED->delay(LOOP_DELAY_MS);
  delay(LOOP_DELAY_MS);
}
