//https://raw.githubusercontent.com/edabot/perlin_wheel/master/perlin_wheel_neopixel_trinket.ino
// https://www.instructables.com/id/LED-Flame-Controlled-by-Noise/
// https://hackaday.com/2019/12/28/led-flame-illuminates-the-beauty-of-noise/#more-390357
//#include "Particle.h"
#include <FastLED.h>
#include "math.h"

FASTLED_USING_NAMESPACE;
SYSTEM_MODE(MANUAL);
#define SETUP_BUTTON_HOLD_DURATION_MS 600
#define BRIGHTNESS_OFF 0
#define BRIGHTNESS_LO 25
#define BRIGHTNESS_MED 125
#define BRIGHTNESS_HI 250
uint8_t button_state = 0;
unsigned long button_timer = 0;
unsigned long t_now;

#define PI 3.1415962
//define LED_PIN D6
#define LED_PIN 6
#define NUM_LEDS 24
int BRIGHTNESS = BRIGHTNESS_MED;
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

const float circleRadius = ((float)NUM_LEDS) / PI;
const float angle = 2 * PI / ((float)NUM_LEDS);
float xOffsets[NUM_LEDS];
float yOffsets[NUM_LEDS];

const int speedH = 2;   //Hue value is 16-bit
const int scaleH = 10; // 50
const int speedS = 3;
const int scaleS = 15;
const int speedV = 1;
const int scaleV = 50; // 50

float colorStart = 0;
float colorRange = .1;  //Range of each section of color 1 = 100%
float colorSpeed = .05;    //Speed of color cycling (def: .1)
const int colorMax = 255;

String mode = "TRIAD"; // options 1: normal, 2: complementary, 3: triad

// The leds
CRGB leds[NUM_LEDS];

float x, y, zH, zS, zV;

uint8_t colorLoop = 1;

void setup()
{
  Serial.begin(9600);
  LEDS.addLeds<LED_TYPE,LED_PIN,COLOR_ORDER>(leds,NUM_LEDS);
  LEDS.setBrightness(BRIGHTNESS);

  x = 0.0;
  y = 0.0;
  zH = 100;
  zS = 50;
  zV = 0;

  for (int i = 0; i < NUM_LEDS; i++)
  {
    float thisAngle = angle * i;
    float xoffset = cos(thisAngle) * circleRadius;
    float yoffset = sin(thisAngle) * circleRadius;
    xOffsets[i] = xoffset;
    yOffsets[i] = yoffset;
  }
}

void fillnoise()
{

  for (int i = 0; i < NUM_LEDS; i++)
  {
    int noiseValH = inoise8(x + xOffsets[i] * scaleH, y + yOffsets[i] * scaleH, zH);
    int noiseValS = inoise8(x + xOffsets[i] * scaleS, y + yOffsets[i] * scaleS, zS);
    int noiseValV = inoise8(x + xOffsets[i] * scaleV, y + yOffsets[i] * scaleS, zV);

    int hue = (int) (noiseValH * colorRange + colorStart);
    if (mode=="TRIAD"){
      if (noiseValH > colorMax * 5 / 8) {
        hue = hue + colorMax * 2 / 3 - colorMax * colorRange * 5 / 8;
      } else if (noiseValH > colorMax * 3 / 8) {
        hue = hue + colorMax / 3 - colorMax * colorRange * 3 / 8;
      }
    }
    if (mode=="COMPLEMENTARY") {
      if (noiseValH > colorMax / 2) {
        hue = hue + colorMax / 2 - colorMax * colorRange / 2;
      }
    }
    hue = hue % colorMax;

    int saturation = constrain(noiseValS + 70, 0, 255);
    int value = constrain(noiseValV - 20, 0, 255);

    leds[i] = CHSV(hue, saturation, value);
  }

  zH += speedH;
  zS += speedS;
  zV += speedV;

  // apply slow drift to X and Y, just for visual variation.
  x += speedS / 8;
  y -= speedS / 16;

  colorStart += colorSpeed;
  if (colorStart > colorMax) { colorStart -= colorMax; }

}

void handle_button() {
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
    switch (BRIGHTNESS) {
    case BRIGHTNESS_OFF:
      BRIGHTNESS = BRIGHTNESS_LO;
      break;
    case BRIGHTNESS_LO:
      BRIGHTNESS = BRIGHTNESS_MED;
      break;
    case BRIGHTNESS_MED:
      BRIGHTNESS = BRIGHTNESS_HI;
      break;
    case BRIGHTNESS_HI:
      BRIGHTNESS = BRIGHTNESS_OFF;
      break;
    default:
      BRIGHTNESS = BRIGHTNESS_LO;
      break;
    }
    LEDS.setBrightness(BRIGHTNESS);
  }

}

void loop()
{
  handle_button();
  fillnoise();

  LEDS.show();
  delay(20);
}
