/*
 * Project particle-neopixel-ring-blinky
 * Description: round blinky thing
 * Author: Gabe Conradi <gabe.conradi@gmail.com>
 * Date: idklol
 */
#include "FastLED.h"
#include "hsv.h"

struct DeckSettings {
  uint8_t label;
  float crossfadePositionActive;
  uint8_t pattern;
  uint8_t palette;
  uint8_t animationIndex;
  NSFastLED::CRGBPalette16 currentPalette; // current color palette
  unsigned long tPatternStart;  // time last pattern changed
  unsigned long tPaletteStart;  // time last palette changed
};

DeckSettings deckSettingsA;
DeckSettings deckSettingsB;
DeckSettings* deckSettingsAll[] = {&deckSettingsA, &deckSettingsB};

typedef void (*FP)(NSFastLED::CRGB*, DeckSettings*);

// Use qsuba for smooth pixel colouring and qsubd for non-smooth pixel colouring
#define qsubd(x, b)  ((x>b)?b:0)
#define qsuba(x, b)  ((x>b)?x-b:0)
#define NUM_PATTERNS sizeof(patternBank) / sizeof(FP)
#define RGB_COLOR_CONTROL false // should we control the center LED or not?
#define NUM_LEDS 24
#define PIXEL_PIN 6
#define PIXEL_TYPE NSFastLED::NEOPIXEL
#define HUE_STEP 2 // 1..255, each loop increments hue by this value
#define UPDATES_PER_SECOND 40
/* set this to match the number of patterns you flip
between when holding the setup button */
#define N_PATTERNS 3
#define SETUP_BUTTON_HOLD_DURATION_MS 600
#define MAX_BRIGHTNESS 255
#define BRIGHTNESS_OFF 0
#define BRIGHTNESS_LO 25
#define BRIGHTNESS_HI 200

#define BOOTUP_ANIM_DURATION_MS 6000
#define PATTERN_CHANGE_INTERVAL_MS 30000
#define PALETTE_CHANGE_INTERVAL_MS 30000
#define VJ_CROSSFADING_ENABLED 1
#define VJ_CROSSFADE_DURATION_MS 6000
#define VJ_NUM_DECKS 2
// switch between deck a and b with this interval
#define VJ_DECK_SWITCH_INTERVAL_MS 15000
#define AUTO_CHANGE_PALETTE 1
bool AUTO_PATTERN_CHANGE = true;
unsigned long t_now;                // time now in each loop iteration
unsigned long t_boot;               // time at bootup

/* state for controlling user-mode button for pattern changes */
uint8_t button_state = 0;
unsigned long button_timer = 0;

// PORNJ!!!! These values are selected to look decent on RBG LEDs
#define DISORIENT_PINK_R 252
#define DISORIENT_PINK_G 0
#define DISORIENT_PINK_B 210
#define DISORIENT_ORANGE_R 255
#define DISORIENT_ORANGE_G 68
#define DISORIENT_ORANGE_B 0
#define CRGB_DIS_PINK NSFastLED::CRGB(DISORIENT_PINK_R, DISORIENT_PINK_G, DISORIENT_PINK_B)
#define CRGB_DIS_ORAN NSFastLED::CRGB(DISORIENT_ORANGE_R, DISORIENT_ORANGE_G, DISORIENT_ORANGE_B)

uint8_t base_hue = 0;
//unsigned long GLOBAL_BRIGHTNESS = 10;
unsigned long GLOBAL_BRIGHTNESS = 10;
// this is ghetto debouncing, and will ignore input to the pattern button
// for X cycles of LOOP_DELAY_MS once triggered
uint8_t ignore_button_cycles = 0;

NSFastLED::CFastLED* gLED; // global CFastLED object


/* custom color palettes */
// orange 255,102,0 FF6600
// pink 255,0,255 #ff00ff
// pornj 255,51,51 #ff3333
extern const NSFastLED::TProgmemRGBGradientPalette_byte Disorient_gp[] = {
      0,   0,   0,   0,    // black
     75, 255,  26, 153,    // pink
    147, 255,  51,  51,    // pornj
    208, 255, 111,  15,    // orange
    255, 255, 255, 255, }; // white
extern const NSFastLED::TProgmemRGBGradientPalette_byte es_pinksplash_08_gp[] = {
    0, 126, 11,255,
  127, 197,  1, 22,
  175, 210,157,172,
  221, 157,  3,112,
  255, 157,  3,112};
extern const NSFastLED::TProgmemRGBGradientPalette_byte es_pinksplash_07_gp[] = {
    0, 229,  1,  1,
   61, 242,  4, 63,
  101, 255, 12,255,
  127, 249, 81,252,
  153, 255, 11,235,
  193, 244,  5, 68,
  255, 232,  1,  5};
extern const NSFastLED::TProgmemRGBGradientPalette_byte Sunset_Real_gp[] {
    0, 120,  0,  0,
   22, 179, 22,  0,
   51, 255,104,  0,
   85, 167, 22, 18,
  135, 100,  0,103,
  198,  16,  0,130,
  255,   0,  0,160};
extern const NSFastLED::TProgmemRGBGradientPalette_byte rgi_15_gp[] {
    0,   4,  1, 31,
   31,  55,  1, 16,
   63, 197,  3,  7,
   95,  59,  2, 17,
  127,   6,  2, 34,
  159,  39,  6, 33,
  191, 112, 13, 32,
  223,  56,  9, 35,
  255,  22,  6, 38};
extern const NSFastLED::TProgmemRGBGradientPalette_byte BlacK_Red_Magenta_Yellow_gp[] {
    0,   0,  0,  0,
   42,  42,  0,  0,
   84, 255,  0,  0,
  127, 255,  0, 45,
  170, 255,  0,255,
  212, 255, 55, 45,
  255, 255,255,  0};

// for effects that are palette based
NSFastLED::CRGBPalette16 palettes[] = {
  Disorient_gp,
  NSFastLED::CloudColors_p,
  es_pinksplash_08_gp,
  BlacK_Red_Magenta_Yellow_gp,
  es_pinksplash_07_gp,
  Sunset_Real_gp,
  rgi_15_gp,
  NSFastLED::ForestColors_p,
  NSFastLED::OceanColors_p,
  NSFastLED::LavaColors_p,
};
#define PALETTES_COUNT (sizeof(palettes)/sizeof(*palettes))

NSFastLED::TBlendType currentBlending = NSFastLED::LINEARBLEND;
NSFastLED::CRGB masterOutput[NUM_LEDS];
NSFastLED::CRGB deckA[NUM_LEDS];
NSFastLED::CRGB deckB[NUM_LEDS];
float crossfadePosition = 1.0;  // 0.0 is deckA, 1.0 is deckB
int crossfadeDirection = (crossfadePosition == 1.0) ? -1 : 1; // start going B -> A
uint8_t crossfadeInProgress = 0;
unsigned long tLastCrossfade = 0;

// make sure we disable wifi cause we dont need that shit!
SYSTEM_MODE(SEMI_AUTOMATIC);


/*vars for pattern_phase_shift_palette*/
int wave1=0;
void pattern_phase_shift_palette(NSFastLED::CRGB* leds, DeckSettings* s) {
  // phase shift
  wave1 += 8;
  int phase2 = NSFastLED::beatsin8(7,-64,64);

  for (int k=0; k<NUM_LEDS; k++) {
    int phase1 = NSFastLED::sin8(3*k + wave1/128);
    int colorIndex = NSFastLED::cubicwave8((k)+phase1)/2 + NSFastLED::cos8((k*3)+phase2)/2;

    //int bri8 = NSFastLED::cubicwave8(t_now/10.0 + k*10.0); // nice pulsy one direction intensity modulator
    // generate undulating intensity phases
    int bri8 = NSFastLED::cubicwave8(t_now/10.0 + NSFastLED::cubicwave8(k*10.0));

    //Serial.printlnf("%d %d", k, bri8);
    leds[k] = ColorFromPalette(s->currentPalette, colorIndex, bri8, currentBlending);
  }
}

void pattern_from_palette(NSFastLED::CRGB* leds, DeckSettings* s) {
  uint8_t b = NSFastLED::beatsin8(4, 0, 255);
  for( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = NSFastLED::ColorFromPalette(s->currentPalette, s->animationIndex + i + b, MAX_BRIGHTNESS, currentBlending);
  }
  // slow down progression by 1/3
  if (t_now%3 == 0) {
    s->animationIndex = NSFastLED::addmod8(s->animationIndex, 1, 255);
  }
}

void pattern_slow_pulse_with_sparkles(NSFastLED::CRGB* leds, DeckSettings* s) {
  // pick a color, and pulse it
  uint8_t cBrightness = NSFastLED::beatsin8(20, 140, 255);
  uint8_t cHue = NSFastLED::beatsin8(4, 0, 255);
  NSFastLED::CHSV hsv_led = NSFastLED::CHSV(cHue, 255, cBrightness);
  NSFastLED::CRGB rgb_led;
  hsv2rgb_rainbow(hsv_led, rgb_led);
  for( int i = 0; i < NUM_LEDS; i++) {
    if (random(NUM_LEDS*3) == 0) {
      leds[i] = NSFastLED::CRGB::White;
    } else {
      leds[i] = rgb_led;
    }
  }
}

// cycle a rainbow, varying how quickly it rolls around the board
void pattern_rainbow_waves_with_sparkles(NSFastLED::CRGB* leds, DeckSettings* s) {
  for(int i = 0; i < NUM_LEDS; ++i) {
    if (random(NUM_LEDS*3) == 0) {
      leds[i] = NSFastLED::CRGB::White;
    } else {
      uint8_t h = (t_now/12+i)%256;
      NSFastLED::CHSV hsv_led = NSFastLED::CHSV(h, 255, 255);
      NSFastLED::CRGB rgb_led;
      hsv2rgb_rainbow(hsv_led, rgb_led);
      leds[i] = rgb_led;
    }
  }
}

// NOTE: lifted and tweaked from https://learn.adafruit.com/rainbow-chakra-led-hoodie/the-code
// This function draws color waves with an ever-changing,
// widely-varying set of parameters, using a color palette.
void pattern_palette_waves(NSFastLED::CRGB* leds, DeckSettings* s) {
  uint8_t numleds = NUM_LEDS;
  static uint16_t sPseudotime = 0;
  static uint16_t sLastMillis = 0;
  static uint16_t sHue16 = 0;

  //uint8_t sat8 = NSFastLED::beatsin88( 87, 220, 250);
  uint8_t brightdepth = NSFastLED::beatsin88( 341, 96, 224);
  uint16_t brightnessthetainc16 = NSFastLED::beatsin88( 203, (25 * 256), (40 * 256));
  uint8_t msmultiplier = NSFastLED::beatsin88(147, 23, 60);

  uint16_t hue16 = sHue16;//gHue * 256;
  uint16_t hueinc16 = NSFastLED::beatsin88(113, 300, 1500);

  uint16_t ms = millis();
  uint16_t deltams = ms - sLastMillis ;
  sLastMillis  = ms;
  sPseudotime += deltams * msmultiplier;
  sHue16 += deltams * NSFastLED::beatsin88( 400, 5,9);
  uint16_t brightnesstheta16 = sPseudotime;

  for( uint16_t i = 0 ; i < numleds; i++) {
    hue16 += hueinc16;
    uint8_t hue8 = hue16 / 256;
    uint16_t h16_128 = hue16 >> 7;
    if( h16_128 & 0x100) {
      hue8 = 255 - (h16_128 >> 1);
    } else {
      hue8 = h16_128 >> 1;
    }

    brightnesstheta16  += brightnessthetainc16;
    uint16_t b16 = NSFastLED::sin16( brightnesstheta16  ) + 32768;

    uint16_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
    uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
    bri8 += (255 - brightdepth);

    uint8_t index = hue8;
    index = NSFastLED::scale8( index, 240);

    NSFastLED::CRGB newcolor = NSFastLED::ColorFromPalette(s->currentPalette, index, bri8);

    uint16_t pixelnumber = i;
    pixelnumber = (numleds-1) - pixelnumber;

    nblend(leds[pixelnumber], newcolor, 128);
  }
}

void  pattern_plasma(NSFastLED::CRGB* leds, DeckSettings* s) {

  int thisPhase = NSFastLED::beatsin8(6,-64,64);
  int thatPhase = NSFastLED::beatsin8(7,-64,64);

  for (int k=0; k<NUM_LEDS; k++) {

    int colorIndex = NSFastLED::cubicwave8((k*23)+thisPhase)/2 + NSFastLED::cos8((k*15)+thatPhase)/2;
    int thisBright = NSFastLED::cubicwave8(t_now/10.0 + k*10.0); // nice pulsy one direction intensity modulator
    //int thisBright = qsuba(colorIndex, NSFastLED::beatsin8(7,0,96));

    leds[k] = ColorFromPalette(s->currentPalette, colorIndex, thisBright, currentBlending);
  }
}

// pattern 0
void pattern_disorient_0(NSFastLED::CRGB* leds, DeckSettings* s){
  long t = t_now*0.01;
  uint8_t offset = t % NUM_LEDS;
  uint8_t new_i = 0;
  for (int i = 0 ; i < NUM_LEDS ; ++i){
    new_i = (i + offset) % NUM_LEDS;
    if (random(NUM_LEDS*2) == 0) {
      leds[new_i] = NSFastLED::CRGB::White;
    } else {
      if (i >= NUM_LEDS/2) {
        leds[new_i] = CRGB_DIS_PINK;
      } else {
        leds[new_i] = CRGB_DIS_ORAN;
      }
    }
  }
}

// random disorient sparkles
void pattern_disorient_sparkles(NSFastLED::CRGB* leds, DeckSettings* s){
  for (int i = 0 ; i < NUM_LEDS ; ++i){
    if (random(NUM_LEDS*3) == 0) {
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
void pattern_hsv_offset_circle_loop(NSFastLED::CRGB* leds, DeckSettings* s){
  uint8_t offset = 0;
  NSFastLED::CRGB rgb;
  NSFastLED::CHSV hsv;
  for (int i = 0 ; i < NUM_LEDS ; ++i){
    offset = ((float)i/(NUM_LEDS-1))*255;
    hsv = NSFastLED::CHSV(base_hue+offset, 255, 255);
    NSFastLED::hsv2rgb_rainbow(hsv, rgb);
    leds[i] = rgb;
  }

  // set the center pixel to mirror what pixel 0 is
  hsv = NSFastLED::CHSV(base_hue+offset, 255, 255);
  hsv2rgb_rainbow(hsv, rgb);
  if (RGB_COLOR_CONTROL) {
    RGB.color(rgb.r, rgb.g, rgb.b);
  }

  base_hue += HUE_STEP;
}

// cycle thru color palettes
void pattern_color_palettes(NSFastLED::CRGB* leds, DeckSettings* s) {
  for( int i = 0; i < NUM_LEDS; ++i) {
    uint8_t pIndex = (i + 6*i*NUM_LEDS/NUM_LEDS + s->animationIndex)%256;
    leds[i] = NSFastLED::ColorFromPalette(s->currentPalette, pIndex, 255, currentBlending);
  }
  s->animationIndex = (s->animationIndex+3)%256;
}

void pattern_color_palettes_with_sparkle(NSFastLED::CRGB* leds, DeckSettings* s) {
  for( int i = 0; i < NUM_LEDS; ++i) {
    if (random(NUM_LEDS*2) == 0) {
      leds[i] = NSFastLED::CRGB::White;
    } else {
      uint8_t pIndex = (i + 6*i*NUM_LEDS/NUM_LEDS + s->animationIndex)%256;
      leds[i] = NSFastLED::ColorFromPalette(s->currentPalette, pIndex, 255, currentBlending);
    }
  }
  s->animationIndex = (s->animationIndex+3)%256;
}


// rainbow pulsing with varied breathing cycles
void pattern_phase_rainbow_pulse(NSFastLED::CRGB* leds, DeckSettings* s) {
  uint8_t cBrightness = NSFastLED::beatsin8(128, 0, 255);
  NSFastLED::CHSV hsv = NSFastLED::CHSV(base_hue, 255, cBrightness);
  NSFastLED::CRGB rgb;
  NSFastLED::hsv2rgb_rainbow(hsv, rgb);
  for (int i = 0 ; i < NUM_LEDS ; ++i){
    leds[i] = rgb;
  }
  // set the center pixel to mirror what pixel 0 is
  if (RGB_COLOR_CONTROL) {
    RGB.color(rgb.r, rgb.g, rgb.b);
  }
  base_hue += 2;
}

// pattern 3
void pattern_hsv_circle_loop(NSFastLED::CRGB* leds, DeckSettings* s){
  NSFastLED::CHSV hsv = NSFastLED::CHSV(base_hue, 255, 255);
  NSFastLED::CRGB rgb;
  NSFastLED::hsv2rgb_rainbow(hsv, rgb);
  for (int i = 0 ; i < NUM_LEDS ; ++i){
    leds[i] = rgb;
  }
  // set the center pixel to mirror what pixel 0 is
  if (RGB_COLOR_CONTROL) {
    RGB.color(rgb.r, rgb.g, rgb.b);
  }
  base_hue += HUE_STEP;
}

// pattern 4
void pattern_disorient_2(NSFastLED::CRGB* leds, DeckSettings* s){
  // same as pattern 0, but in quadrants
  uint8_t offset_0, offset_1, offset_2, offset_3;
  offset_0 = 0;
  offset_1 = NUM_LEDS/4;
  offset_2 = NUM_LEDS/2;
  offset_3 = offset_1+offset_2;
  long t = t_now*0.25;
  uint8_t offset = t % NUM_LEDS;
  uint8_t new_i = 0;
  for (int i = 0 ; i < NUM_LEDS ; ++i){
    new_i = (i + offset) % NUM_LEDS;
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

void pattern_disorient_palette_sparkles(NSFastLED::CRGB* leds, DeckSettings* s) {
  uint8_t b = NSFastLED::beatsin8(4, 0, 255);
  for( int i = 0; i < NUM_LEDS; i++) {
    if (random(NUM_LEDS*4) == 0) {
      leds[i] = NSFastLED::CRGB::White;
    } else {
      leds[i] = ColorFromPalette((NSFastLED::CRGBPalette16)Disorient_gp, s->animationIndex + i + b, MAX_BRIGHTNESS, currentBlending);
    }
  }
  // slow down progression by 1/3
  if (t_now%3 == 0) {
    s->animationIndex = NSFastLED::addmod8(s->animationIndex, 1, 255);
  }
}


/** update this with patterns you want to be cycled through **/
const FP patternBank[] = {
  &pattern_phase_shift_palette,
  &pattern_plasma,
  &pattern_from_palette,
  //&pattern_disorient_palette_sparkles,
  //&pattern_slow_pulse_with_sparkles,
  &pattern_palette_waves,
  //&pattern_rainbow_waves_with_sparkles,
};

void randomPattern(DeckSettings* deck, DeckSettings* otherDeck) {
  uint8_t old = deck->pattern;
  while (deck->pattern == old || deck->pattern == otherDeck->pattern) {
    deck->pattern = NSFastLED::random8(0, NUM_PATTERNS);
  }
  deck->tPatternStart = t_now;
}

void randomPalette(DeckSettings* deck, DeckSettings* otherDeck) {
  uint8_t old = deck->palette;
  while (deck->palette == old || deck->palette == otherDeck->palette) {
    deck->palette = NSFastLED::random8(0, PALETTES_COUNT);
  }
  deck->currentPalette = palettes[deck->palette];
  deck->tPaletteStart = t_now;
}


// setup() runs once, when the device is first turned on.
void setup() {
  Serial.println("Booting up");
  randomSeed(analogRead(0));

  t_now = millis();
  t_boot = t_now;
  tLastCrossfade = t_now;

  deckSettingsA = {
    1,
    0.0,
    0,
    0,
    0,
    palettes[0],
    t_now,
    t_now,
  };
  deckSettingsB = {
    2,
    1.0,
    0,
    0,
    0,
    palettes[0],
    t_now,
    t_now,
  };

  randomPattern(&deckSettingsA, &deckSettingsB);
  randomPalette(&deckSettingsA, &deckSettingsB);
  randomPattern(&deckSettingsB, &deckSettingsA);
  randomPalette(&deckSettingsB, &deckSettingsA);

  gLED = new NSFastLED::CFastLED();
  gLED->addLeds<PIXEL_TYPE, PIXEL_PIN>(masterOutput, NUM_LEDS);
  gLED->setBrightness(GLOBAL_BRIGHTNESS);

  RGB.control(true); // take over the LED
  if (RGB_COLOR_CONTROL) {
    RGB.color(DISORIENT_PINK_R, DISORIENT_PINK_G, DISORIENT_PINK_B);
  } else {
    RGB.color(0,0,0);
  }
  Serial.println("Finished setup");
}



// loop() runs over and over again, as quickly as it can execute.
void loop() {
  t_now = millis();

  // handle user interaction with reset button
  if (HAL_Core_Mode_Button_Pressed(SETUP_BUTTON_HOLD_DURATION_MS)) {
    switch (button_state) {
    case 0:
      // we werent pressed before, so start timer!
      button_state = 1;
      button_timer = t_now;
      break;
    case 1:
      if (t_now - button_timer > SETUP_BUTTON_HOLD_DURATION_MS) {
        // we have been held longer than
        button_state = 2;
      }
      break;
    // we are waiting to take action now
    case 2: break;
    // action already taken, do nothing until release!
    case 3: break;
    default:
        button_state = 0;
      break;
    }
  } else {
    button_state = 0;
  }

  if (button_state == 2) {
    button_state = 3;
    switch (GLOBAL_BRIGHTNESS) {
    case BRIGHTNESS_HI:
      GLOBAL_BRIGHTNESS = BRIGHTNESS_OFF;
      break;
    case BRIGHTNESS_LO:
      GLOBAL_BRIGHTNESS = BRIGHTNESS_HI;
      break;
    case BRIGHTNESS_OFF:
      GLOBAL_BRIGHTNESS = BRIGHTNESS_LO;
      break;
    default:
      GLOBAL_BRIGHTNESS = BRIGHTNESS_LO;
      break;
    }
  }


  // increment pattern every PATTERN_CHANGE_INTERVAL_MS, but not when a deck is active!
  if (AUTO_PATTERN_CHANGE) {
    if (t_now > deckSettingsA.tPatternStart+PATTERN_CHANGE_INTERVAL_MS && !crossfadeInProgress) {
      if (crossfadePosition == 1.0) {
        randomPattern(&deckSettingsA, &deckSettingsB);
        Serial.printlnf("deckA.pattern=%d", deckSettingsA.pattern);
      }
    }
    if (t_now > deckSettingsB.tPatternStart+PATTERN_CHANGE_INTERVAL_MS && !crossfadeInProgress) {
      if (crossfadePosition == 0.0) {
        randomPattern(&deckSettingsB, &deckSettingsA);
        Serial.printlnf("deckB.pattern=%d", deckSettingsB.pattern);
      }
    }
  }

  // increment palette every PALETTE_CHANGE_INTERVAL_MS, but not when crossfading!
  if (AUTO_CHANGE_PALETTE && !crossfadeInProgress) {
    for (int x = 0; x < VJ_NUM_DECKS ; x++){
      int xOther = (x == 0) ? 1 : 0;
      DeckSettings* deck = deckSettingsAll[x];
      DeckSettings* otherdeck = deckSettingsAll[xOther];
      if ((deck->crossfadePositionActive != crossfadePosition) &&
          (deck->tPaletteStart + PALETTE_CHANGE_INTERVAL_MS < t_now)) {
        randomPalette(deck, otherdeck);
        Serial.printlnf("deck%d.palette=%d", deck->label, deck->palette);
      }
    }
  }

  // fill in patterns on both decks! we will crossfade master output later
  // NOTE: only render to a deck if its "visible" through the crossfader
  if ( !VJ_CROSSFADING_ENABLED || crossfadePosition < 1.0 ) {
    patternBank[deckSettingsA.pattern](deckA, &deckSettingsA);
  }
  if ( VJ_CROSSFADING_ENABLED && crossfadePosition > 0 ) {
    patternBank[deckSettingsB.pattern](deckB, &deckSettingsB);
  }


    // perform crossfading increment if we are mid pattern change
  if (VJ_CROSSFADING_ENABLED) {
    //Serial.printf("%d %d %d\n", t_now, tLastCrossfade + VJ_DECK_SWITCH_INTERVAL_MS, crossfadeInProgress);
    if (t_now > tLastCrossfade + VJ_DECK_SWITCH_INTERVAL_MS && !crossfadeInProgress) {
      // start switching between decks
      Serial.printf("starting fading to %c\n", (crossfadePosition == 1.0) ? 'A' : 'B');
      crossfadeInProgress = 1;
      tLastCrossfade = t_now;
    }
    if (crossfadeInProgress) {
      float step = (float)1.0/(VJ_CROSSFADE_DURATION_MS/1000*UPDATES_PER_SECOND);
      // Serial.printf("fader increment %f %d\n", step, crossfadeDirection);
      crossfadePosition = crossfadePosition + crossfadeDirection * step;

      // is it time to change decks?
      // we are cut over to deck B, break this loop
      if (crossfadePosition > 1.0) {
        crossfadePosition = 1.0;
        crossfadeDirection = -1; // 1->0
        crossfadeInProgress = 0;
        Serial.printf("finished fading to B\n");
      }
      // we are cut over to deck B
      if (crossfadePosition < 0.0) {
        crossfadePosition = 0.0;
        crossfadeDirection = 1;  // 0->1
        crossfadeInProgress = 0;
        Serial.printf("finished fading to A\n");
      }
    }
  }

  // perform crossfading between deckA and deckB, by filling masterOutput
  // FIXME for now, lets just take a linear interpolation between deck a and b
  for (int i = 0; i < NUM_LEDS; ++i) {
    if (VJ_CROSSFADING_ENABLED) {
      masterOutput[i] = deckA[i].lerp8(deckB[i], NSFastLED::fract8(255*crossfadePosition));
    } else {
      masterOutput[i] = deckA[i];
    }
    if (t_now < + BOOTUP_ANIM_DURATION_MS) {
      // ramp intensity up slowly, so we fade in when turning on
      int8_t bri8 = (uint8_t)((t_now*1.0)/BOOTUP_ANIM_DURATION_MS*255.0);
      masterOutput[i] = masterOutput[i].fadeToBlackBy(255-bri8);
    }
 }

  gLED->setBrightness(GLOBAL_BRIGHTNESS);
  gLED->show();
  delay(1000.0 / UPDATES_PER_SECOND);
}
