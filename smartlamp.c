/*
 * Copyright 2015 Trung Huynh
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define MAJOR_VERSION 1
#define MINOR_VERSION 1

#define START_CMD_CHAR '*'
#define END_CMD_CHAR '#'
#define DIV_CMD_CHAR '|'

#define RED_PIN     3 // red
#define GREEN_PIN   5 // green
#define BLUE_PIN    6 // blue

//Configuration
// 50ms (.05 second) delay; shorten for faster fades
#define WAIT_TIME   50
// max light
#define MAX_VAL     255

// extended value
#define VARY_REQ    1 // color variation auto change
#define FADE_REQ    2 // color fade auto change
#define TIMER_REQ   3 // fade off timer
#define VERS_REQ    4 // version request
#define VERS_RES    5 // version response

// auto changeing status
#define AUTO_OFF    0 // auto changing is disabled
#define COLOR_VARY  0x01 // color variation auto change
#define TO_VARY     0x02 // auto changing to color variation
#define COLOR_FADE  0x04 // color fade auto change
#define TO_FADE     0x08 // auto changing to color fade
#define FADE_OFF    0x10 // fade off the light

#define ANALOG_WRITE() \
        { \
            analogWrite(RED_PIN,    redVal); \
            analogWrite(GREEN_PIN,  greenVal); \
            analogWrite(BLUE_PIN,   blueVal); \
        }
#define INIT_TIMER() \
        { \
            time = millis(); \
            timerValue = timerValTemp; \
        }

// Variables to store the values to send to the pins
unsigned int redVal = MAX_VAL; // Initial values is Red full
unsigned int greenVal = 0; // Initial values is Green off
unsigned int blueVal = 0;  // Initial values is Blue off

// Variables to store the values get from serial
unsigned int redValTemp = 0;   // Temporary values of Red
unsigned int greenValTemp = 0; // Temporary values of Green
unsigned int blueValTemp = 0;  // Temporary values of Blue
unsigned int extendVal = 0;    // Auto changing values
unsigned char charVal = ' ';   // read serial
unsigned int timerValTemp = 0; // Temporary values of timer
unsigned long timeTemp = 0;    // Temporary time from system

// State of color variation
unsigned int varyState = 0;
// State of color fade
unsigned int fadeState = 0;
// auto changing color switcher
unsigned char autEnabled = COLOR_VARY;
// fade off timer value
unsigned int timerValue = 0;
// time from system
unsigned long time;

void colorVariation()
{
    switch (varyState)
    {
        case 0:             // 1th phase of variation
            greenVal += 1;  // Green up
            if (greenVal == MAX_VAL) varyState = 1;
            break;
        case 1:             // 2nd phase of variation
            redVal -= 1;    // Red down
            if (!redVal) varyState = 2;
            break;
        case 2:             // 3rd phase of variation
            blueVal += 1;   // Blue up
            if (blueVal == MAX_VAL) varyState = 3;
            break;
        case 3:             // 4th phase of variation
            greenVal -= 1;  // Green down
            if (!greenVal) varyState = 4;
            break;
        case 4:             // 5th phase of variation
            redVal += 1;    // Red up
            if (redVal == MAX_VAL) varyState = 5;
            break;
        case 5:             // 6th phase of variation
            blueVal -= 1;   // Blue down
            if (!blueVal) varyState = 0;
            break;
    }
  
    // Write current values to LED pins
    ANALOG_WRITE();
    
    // Pause for 'wait' milliseconds before resuming the loop
    delay(WAIT_TIME);
}

void colorFade()
{
    switch (fadeState)
    {
        case 0:             // 1st phase of fades
            redVal -= 1;    // Red fades down
            if (!redVal) fadeState = 1;
            break;
        case 1:             // 2nd phase of fades
            redVal += 1;    // Yellow fades up
            greenVal += 1;
            if (redVal == MAX_VAL) fadeState = 2;
            break;
        case 2:             // 3rd phase of fades
            redVal -= 1;    // Yellow fades down
            greenVal -= 1;
            if (!redVal) fadeState = 3;
            break;
        case 3:             // 4th phase of fades
            greenVal += 1;  // green fades up
            if (greenVal == MAX_VAL) fadeState = 4;
            break;
        case 4:             // 5th phase of fades
            greenVal -= 1;  // green fades down
            if (!greenVal) fadeState = 5;
            break;
        case 5:             // 6th phase of fades
            greenVal += 1;  // Cyan fades up
            blueVal += 1;
            if (greenVal == MAX_VAL) fadeState = 6;
            break;
        case 6:             // 7th phase of fades
            greenVal -= 1;  // Cyan fades down
            blueVal -= 1;
            if (!greenVal) fadeState = 7;
            break;
        case 7:             // 8th phase of fades
            blueVal += 1;   // Blue fade up
            if (blueVal == MAX_VAL) fadeState = 8;
            break;
        case 8:             // 9th phase of fades
            blueVal -= 1;   // Blue fade down
            if (!blueVal) fadeState = 9;
            break;
        case 9:             // 10th phase of fades
            blueVal += 1;   // Magenta fades up
            redVal += 1;
            if (blueVal == MAX_VAL) fadeState = 10;
            break;
        case 10:            // 11th phase of fades
            blueVal -= 1;   // Magenta fades down
            redVal -= 1;
            if (!blueVal) fadeState = 11;
            break;
        case 11:            // 12th phase of fades
            redVal += 1;    // Red fades up
            if (redVal == MAX_VAL) fadeState = 0;
            break;
    }

    // Write current values to LED pins
    ANALOG_WRITE();

    // Pause for 'wait' milliseconds before resuming the loop
    delay(WAIT_TIME);
}

void toFadeOff()
{
    // get to start spot
    if (redVal > 0) redVal -= 1;
    if (greenVal > 0) greenVal -= 1;
    if (blueVal > 0) blueVal -= 1;

    // if came to start spot
    if ((redVal + greenVal + blueVal) == 0)
    {
        autEnabled = AUTO_OFF;
    }

    // Write current values to LED pins
    ANALOG_WRITE();

    // Pause for 'wait' milliseconds before resuming the loop
    delay(WAIT_TIME);
}

void toAutoChange()
{
    // get to start spot
    if (redVal < MAX_VAL) redVal += 1;
    if (greenVal > 0) greenVal -= 1;
    if (blueVal > 0) blueVal -= 1;

    // if came to start spot
    if ((redVal == MAX_VAL) && ((greenVal + blueVal) == 0))
    {
        if (autEnabled == TO_VARY)
        {
            autEnabled = COLOR_VARY;
            varyState = 0; // start from 1th phase of variation
        }
        else if (autEnabled == TO_FADE)
        {
            autEnabled = COLOR_FADE;
            fadeState = 0; // start from 1st phase of fades
        }
    }

    // Write current values to LED pins
    ANALOG_WRITE();

    // Pause for 'wait' milliseconds before resuming the loop
    delay(WAIT_TIME);
}

void setup()
{
    Serial.begin(9600);
    Serial.flush();
}

void loop()
{
    Serial.flush();

    // wait for incoming data
    // if serial empty
    if (Serial.available() < 1)
    {
        if (timerValue)
        {
            timeTemp = millis();
            if(timeTemp - time > timerValue * 60000)
            {
                autEnabled  = FADE_OFF;
                timerValue  = 0;
            }
        }

        // if auto changing is enabled
        // call autochange() and return to loop()
        switch (autEnabled)
        {
            case COLOR_VARY:
                colorVariation();
                return;
            case COLOR_FADE:
                colorFade();
                return;
            case TO_VARY:
            case TO_FADE:
                toAutoChange();
                return;
            case FADE_OFF:
                toFadeOff();
                return;
            default:
                return;
        }
    }
    
    // parse incoming command start flag 
    charVal = Serial.read();

    // if no command start flag, return to loop().
    if (charVal != START_CMD_CHAR)
        return;

    // parse incoming red value
    redValTemp = Serial.parseInt();

    // parse incoming green value
    greenValTemp = Serial.parseInt();

    // parse incoming blue value
    blueValTemp = Serial.parseInt();

    // parse incoming command end flag 
    charVal = Serial.read();
    
    // if command end flag is divider
    if (charVal == DIV_CMD_CHAR)
    {
        // parse incoming extended value 
        extendVal = Serial.parseInt();

        switch (extendVal)
        {
            case VARY_REQ: // if extended value is color variation auto change,
                if (!(autEnabled & (COLOR_VARY | TO_VARY)))
                    autEnabled = TO_VARY;
                break;
            case FADE_REQ: // if extended value is color fade auto change,
                if (!(autEnabled & (COLOR_FADE | TO_FADE)))
                    autEnabled = TO_FADE;
                break;
            case TIMER_REQ:// if extended value is timer to fade off,
                timerValTemp = Serial.parseInt();
                if (timerValue != timerValTemp)
                {
                    INIT_TIMER();
                }
                return;
            case VERS_REQ: // if extended value is configuration request,
                Serial.print('*');
                Serial.print('*');
                Serial.print(VERS_RES);
                Serial.print('|');
                Serial.print(MAJOR_VERSION);
                Serial.print('|');
                Serial.print(MINOR_VERSION);
                Serial.print('#');
                Serial.println();
                return;
            default:
                return;
        }

        if (!timerValue && timerValTemp)
        {
            INIT_TIMER();
        }
        return;
    }
    // if command end flag is actual, auto changing is disabled
    else if (charVal == END_CMD_CHAR)
    {
        redVal      = redValTemp;
        greenVal    = greenValTemp;
        blueVal     = blueValTemp;
        autEnabled  = AUTO_OFF;

        if (!timerValue && timerValTemp)
        {
            INIT_TIMER();
        }
    }

    // Write current values to LED pins
    ANALOG_WRITE();

    // Done. return to loop()
    return;
}