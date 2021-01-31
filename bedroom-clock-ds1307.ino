// Code by Brian Ernst 2021
// Based on example code clock_sevenseg_ds1307 written by Tony DiCola for Adafruit Industries.
//    from the Adafruit LED backpack library
//    https://github.com/adafruit/Adafruit_LED_Backpack
// Released under a MIT license: https://opensource.org/licenses/MIT

// Wiring: Arduino Uno: SDA is A4. SCL is A5
//         Wemos LOLIN D1 mini: SDA is D2. SCL is D1

// Clock example using a seven segment display & DS1307 real-time clock.
//
// Must have the Adafruit RTClib library installed too!  See:
//   https://github.com/adafruit/RTClib
//
// Designed specifically to work with the Adafruit LED 7-Segment backpacks
// and DS1307 real-time clock breakout:
// ----> http://www.adafruit.com/products/881
// ----> http://www.adafruit.com/products/880
// ----> http://www.adafruit.com/products/879
// ----> http://www.adafruit.com/products/878
// ----> https://www.adafruit.com/products/264

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <RTClib.h>
#include "Adafruit_LEDBackpack.h"

// INCLUDE CHRONO LIBRARY : http://github.com/SofaPirate/Chrono
#include <Chrono.h> 



// Set to false to display time in 12 hour format, or true to use 24 hour:
#define TIME_24_HOUR      false

// I2C address of the display.  Stick with the default address of 0x70
// unless you've changed the address jumpers on the back of the display.
#define DISPLAY_ADDRESS   0x70

# define BRIGHTNESS_HOW_OFTEN 250 // how often (in ms) do we check the brightness knob. 
     // we only display once a second so nothing will be displayed until then anyway. 
     // maybe add display update when the knob changes logic

// Create display and DS1307 objects.  These are global variables that
// can be accessed from both the setup and loop function below.
Adafruit_7segment clockDisplay = Adafruit_7segment();
RTC_DS1307 rtc = RTC_DS1307();

// Keep track of the hours, minutes, seconds displayed by the clock.
// Start off at 0:00:00 as a signal that the time should be read from
// the DS1307 to initialize it.
// Specifically, minuites is checked in the main loop and when is 0
//    time is pulled from the real time clock chip. rtc.now()
int hours = 0;
int minutes = 0;
int seconds = 0;

// Remember if the colon was drawn on the display so it can be blinked
// on and off every second.
bool blinkColon = false;

// instantiate a Chrono object.
Chrono loopChrono; 
Chrono brightnessChrono;


void setup() {
  // Setup function runs once at startup to initialize the display
  // and DS1307 clock.

  // Setup Serial port to print debug output.
  Serial.begin(115200);
  Serial.println("Clock starting!");

  // Setup the display.
  clockDisplay.begin(DISPLAY_ADDRESS);
  // clockDisplay.setBrightness(0); // set display to dimmest setting

  // Setup the DS1307 real-time clock.
  rtc.begin();

  // Set the DS1307 clock if it hasn't been set before.
  bool setClockTime = !rtc.isrunning();
  // Alternatively you can force the clock to be set again by
  // uncommenting this line:
  //setClockTime = true;
  if (setClockTime) {
    Serial.println("Setting DS1307 time!");
    // This line sets the DS1307 time to the exact date and time the
    // sketch was compiled:
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // Alternatively you can set the RTC with an explicit date & time, 
    // for example to set January 21, 2014 at 3am you would uncomment:
    //rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
}

void loop() {
  // Loop function runs over and over again to implement the clock logic.

  // Is it time to read the brightnessKnob?	  
  if (brightnessChrono.hasPassed(BRIGHTNESS_HOW_OFTEN)) {
	  brightnessChrono.restart(); // starts restarts the chgronometer
  }
  // Read an analog voltage from a pot and use it to set the brightness of the display	  


	  
  // check to see if it is time to update the display and add a second
  if (loopChrono.hasPassed(1000) ) { // is true if 1000ms have passed since chronometer was started
      loopChrono.restart();  // start/restarts the chronometer

	  // Check if it's the top of the hour and get a new time reading
	  // from the DS1307.  This helps keep the clock accurate by fixing
	  // any drift.
	  if (minutes == 0) {
	    // Get the time from the DS1307.
	    DateTime now = rtc.now();
	    // Print out the time for debug purposes:
	    Serial.print("Read date & time from DS1307: ");
	    Serial.print(now.year(), DEC);
	    Serial.print('/');
	    Serial.print(now.month(), DEC);
	    Serial.print('/');
	    Serial.print(now.day(), DEC);
	    Serial.print(' ');
	    Serial.print(now.hour(), DEC);
	    Serial.print(':');
	    Serial.print(now.minute(), DEC);
	    Serial.print(':');
	    Serial.print(now.second(), DEC);
	    Serial.println();
	    // Now set the hours and minutes.
	    hours = now.hour();
	    minutes = now.minute();
	  }
	
	  // Show the time on the display by turning it into a numeric
	  // value, like 3:30 turns into 330, by multiplying the hour by
	  // 100 and then adding the minutes.
	  int displayValue = hours*100 + minutes;
	
	  // Do 24 hour to 12 hour format conversion when required.
	  if (!TIME_24_HOUR) {
	    // Handle when hours are past 12 by subtracting 12 hours (1200 value).
	    if (hours > 12) {
	      displayValue -= 1200;
	    }
	    // Handle hour 0 (midnight) being shown as 12.
	    else if (hours == 0) {
	      displayValue += 1200;
	    }
	  }
	
	  // Now print the time value to the display.
	  clockDisplay.print(displayValue, DEC);
	
	  // Add zero padding when in 24 hour mode and it's midnight.
	  // In this case the print function above won't have leading 0's
	  // which can look confusing.  Go in and explicitly add these zeros.
	  if (TIME_24_HOUR && hours == 0) {
	    // Pad hour 0.
	    clockDisplay.writeDigitNum(1, 0);
	    // Also pad when the 10's minute is 0 and should be padded.
	    if (minutes < 10) {
	      clockDisplay.writeDigitNum(2, 0);
	    }
	  }
	
	  // Blink the colon by flipping its value every loop iteration
	  // (which happens every second).
	  blinkColon = !blinkColon;
	  clockDisplay.drawColon(blinkColon);
	
	  // Now push out to the display the new values that were set above.
	  clockDisplay.writeDisplay();
	
	  // Now increase the seconds by one.
	  seconds += 1;
	  // If the seconds go above 59 then the minutes should increase and
	  // the seconds should wrap back to 0.
	  if (seconds > 59) {
	    seconds = 0;
	    minutes += 1;
	    // Again if the minutes go above 59 then the hour should increase and
	    // the minutes should wrap back to 0.
	    if (minutes > 59) {
	      minutes = 0;
	      hours += 1;
	      // Note that when the minutes are 0 (i.e. it's the top of a new hour)
	      // then the start of the loop will read the actual time from the DS1307
	      // again.  Just to be safe though we'll also increment the hour and wrap
	      // back to 0 if it goes above 23 (i.e. past midnight).
	      if (hours > 23) {
	        hours = 0;
	      }
	    }
	  }
	  
	} // endif check for chrono

  // Loop code is finished, it will jump back to the start of the loop
  // function again!
}
