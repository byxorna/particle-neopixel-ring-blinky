//https://raw.githubusercontent.com/edabot/perlin_wheel/master/perlin_wheel_neopixel_trinket.ino
// https://www.instructables.com/id/LED-Flame-Controlled-by-Noise/
// https://hackaday.com/2019/12/28/led-flame-illuminates-the-beauty-of-noise/#more-390357
#include <FastLED.h>
#include "math.h"

FASTLED_USING_NAMESPACE;
#define PI 3.1415962
#define LED_PIN 6
#define NUM_LEDS 24
#define BRIGHTNESS 96
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

const float circleRadius = ((float)NUM_LEDS) / PI;
const float angle = 2 * PI / ((float)NUM_LEDS);
float xOffsets[NUM_LEDS];
float yOffsets[NUM_LEDS];

const int speedH = 4;   //Hue value is 16-bit
const int scaleH = 50; // 50
const int speedS = 7;
const int scaleS = 30;
const int speedV = 10;
const int scaleV = 50; // 50

float colorStart = 0;
float colorRange = .7;  //Range of each section of color 1 = 100%
float colorSpeed = .1;    //Speed of color cycling (def: .1)
const int colorMax = 255;

String mode = "none"; // options 1: normal, 2: complementary, 3: triad

// The leds
CRGB leds[NUM_LEDS];

float x, y, zH, zS, zV;

uint8_t colorLoop = 1;

void setup()
{
  Serial.begin(9600);
  LEDS.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
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
    if (mode == "TRIAD") {
      if (noiseValH > colorMax * 5 / 8) {
        hue = hue + colorMax * 2 / 3 - colorMax * colorRange * 5 / 8;
      } else if (noiseValH > colorMax * 3 / 8) {
        hue = hue + colorMax / 3 - colorMax * colorRange * 3 / 8;
      }
    }
    if (mode == "COMPLEMENTARY") {
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
  if (colorStart > colorMax) {
    colorStart -= colorMax;
  }

}


void loop()
{
  fillnoise();

  LEDS.show();
  delay(20);
}
