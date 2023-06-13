#include <Arduino.h>
#define FASTLED_INTERNAL
#include <FastLED.h>

/******************************************************************************
 * Function:    DrawSolidRainbow
 * Date:        09 Aug 2021
 * Created by:  Roger Banks
 * Description: Creates a rainbow pattern that moves along the strip
 *****************************************************************************/
void DrawSolidRainbow()
{
    byte hue = beat8(8);
    fill_solid(g_LEDs, NUM_LEDS, CHSV(hue,255,255));
}