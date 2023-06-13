//+--------------------------------------------------------------------------
//
// NightDriver - (c) 2020 Dave Plummer.  All Rights Reserved.
//
// File:        
//
// Description:
//
//   
//
// History:     Sep-15-2020     davepl      Created
//
//---------------------------------------------------------------------------

#include <Arduino.h>
#define FASTLED_INTERNAL
#include <FastLED.h>

#include "ledgfx.h"

static const CRGB TwinkleColors [] = 
{
    CRGB::Red,
    CRGB::Blue,
    CRGB::Purple,
    CRGB::Green,
    CRGB::Yellow,
    CRGB::Orange
};

void DrawTwinkle()
{
    const int fadeAmt = 32;
    EVERY_N_MILLISECONDS(250)
    {
        FastLED.leds()[random(NUM_LEDS)] = TwinkleColors[random(0, ARRAYSIZE(TwinkleColors))];
        fadeToBlackBy(FastLED.leds(), NUM_LEDS, fadeAmt);
    }      
}

/******************************************************************************
 * Function:    DrawTwinkleTails
 * Date:        09 Aug 2021
 * Created by:  Roger Banks
 * Description: Creates a random string of pixels with a random color from a
 *              table and moves that string of pixels along the strip until
 *              it fades out.
 *****************************************************************************/
void DrawTwinkleTails()
{
    const int fadeAmt = 64;            // amount to fade pixels by every pass
    const int tailSize = 3;            // length of the string of pixels
    EVERY_N_MILLISECONDS(250)
    {
        // choose a random position within the strip
        int random_pos = random(tailSize, NUM_LEDS-1);
        // choose a random color from the table
        CRGB random_color = TwinkleColors[random(0, ARRAYSIZE(TwinkleColors))];
        // assign the chosen color to the chosen pixel and the pixels following
        // it according the tail_size
        for(int j = random_pos; j > random_pos - tailSize; j--) {
            FastLED.leds()[j] = random_color;
        }
        // animate the pixels to move forward along the strip
        for(int i = NUM_LEDS-1; i > 1; i--) {
            FastLED.leds()[i] = FastLED.leds()[i-1];
        }
        // fade all the pixels by the fadeAmt
        fadeToBlackBy(FastLED.leds(), NUM_LEDS, fadeAmt);
    }      
}