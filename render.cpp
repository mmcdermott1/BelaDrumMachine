// Queen Mary University of London
// ECS7012 - Music and Audio Programming
// Spring 2022
// Assignment 2: Drum Machine
// Mitchell McDermott
// Student ID: 210848991

#include <Bela.h>
#include <cmath>
#include "drums.h"
#include "Debouncer.h"
#include <libraries/Trill/Trill.h>

extern float *gDrumSampleBuffers[NUMBER_OF_DRUMS];
extern int gDrumSampleBufferLengths[NUMBER_OF_DRUMS];
int gReadPointers[16] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
int gDrumBufferForReadPointer[16] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
int gChanCount = 0;
extern int *gPatterns[NUMBER_OF_PATTERNS];		// patterns indicate which drum(s) should play on which beat.
extern int gPatternLengths[NUMBER_OF_PATTERNS]; // Each element of gPatterns is an array, whose length is given by gPatternLengths.
int gCurrentPattern = 0;						// which pattern we're playing
int gCurrentIndexInPattern = 0;					// location within pattern
int gEventIntervalMilliseconds = 250;			// interval between events in **milliseconds**, must convert to samples
int gCountSamples = 0;							// how many samples have elapsed since last event
bool gIsPlaying;								// whether samples should be triggered or not
int gPlaysBackwards = 0;						// whether we should play the samples backwards
int gShouldPlayFill = 0;						// whether we should play the fill
int gPreviousPattern = 0;						// the previous pattern
Debouncer gDebouncer;							// Debouncer object
bool gButtonState = false;						// current state of the button
bool gPreviousButtonState = false;				// previous state of the button
int gAudioFramesPerAnalogFrame = 0;				// how many audio frames per analog frame
void updatePattern(int newPattern);				// function to update the pattern
void reverseSamples(bool reverse, int i);		// function to reverse the samples

bool setup(BelaContext *context, void *userData)
{
	if (context->analogFrames == 0 || context->analogFrames > context->audioFrames) // Check if analog channels are enabled
	{
		rt_printf("ERROR: Enable analog input with 4 or 8 channels\n");
		return false;
	}

	gAudioFramesPerAnalogFrame = context->audioFrames / context->analogFrames; // how many audio frames per analog frame
	pinMode(context, 0, 0, INPUT);											   // set digital pin for push button
	pinMode(context, 0, 3, OUTPUT);											   // set digital pin for LED
	gDebouncer.setup(context->audioSampleRate, .05);						   // Initialise the debouncer with 5ms interval

	return true;
}

void render(BelaContext *context, void *userData)
{
	for (unsigned int n = 0; n < context->audioFrames; n++)
	{
		//** HANDLE ANALOG **//
		if (gAudioFramesPerAnalogFrame && !(n % gAudioFramesPerAnalogFrame)) // if we are at the start of an analog frame
		{
			float x = analogRead(context, n / gAudioFramesPerAnalogFrame, 0); // read accelerometer values
			float y = analogRead(context, n / gAudioFramesPerAnalogFrame, 1);
			float z = analogRead(context, n / gAudioFramesPerAnalogFrame, 2);

			x = map(x - 0.4, -0.2, 0.2, -1.0, 1.0); // normalize accelerometer values
			y = map(y - 0.43, -0.2, 0.2, -1.0, 1.0);
			z = map(fabs(z - 0.54), 0, 0.4, 0, 1.0);

			//** CHANGE PATTERN **//
			if (y > -1.0 && y < 1.0 && x > -1.0 && x < 1.0 && gShouldPlayFill == 0) // set boundaries to protect against fill
			{
				if (y < -0.75 && z > 0.3) // LEFT
					updatePattern(1);

				else if (y > 0.75 && z > 0.3) // RIGHT
					updatePattern(2);

				else if (x < -0.75 && z > 0.3) // UP
					updatePattern(3);

				else if (x > -0.75 && z > 0.3) // DOWN
					updatePattern(4);

				else if (y > -0.25 && y < 0.25 && x > -0.25 && x < 0.25 && z < 0.1) // FLAT
					updatePattern(0);
			}

			//** REVERSE SAMPLES **//
			if (z > 0.9)
				gPlaysBackwards = 1;

			else if (z < 0.5)
				gPlaysBackwards = 0;

			//** PLAY FILL **//
			if (x > 1.5 || y > 1.5)
			{
				gShouldPlayFill = 1;
				updatePattern(FILL_PATTERN);
			}
		}

		//** SET TEMPO **//
		gEventIntervalMilliseconds = map(analogRead(context, n / gAudioFramesPerAnalogFrame, 3), 0, 0.82, 50, 1000);

		//** START + STOP **//
		gButtonState = !gDebouncer.process(digitalRead(context, 0, 1)); // read the button
		if (gButtonState && !gPreviousButtonState)						// if the button has just been pressed
		{
			gIsPlaying = !gIsPlaying; // toggle the state
		}
		gPreviousButtonState = gButtonState; // update the previous state
		if (gIsPlaying)						 // if we are playing
		{
			if (gCountSamples >= gEventIntervalMilliseconds * context->audioSampleRate / 1000) // if the interval has elapsed
			{
				startNextEvent();  // start the next event
				gCountSamples = 0; // reset the count
			}
			else
			{
				gCountSamples++;																	  // increment the count
				if (gCountSamples < gEventIntervalMilliseconds * context->audioSampleRate / 1000 / 4) // light the led for 1/4 of the interval
					digitalWrite(context, n, 3, HIGH);												  // Turn the LED on
				else
					digitalWrite(context, n, 3, LOW); // Turn the LED off
			}
		}

		//** RENDER AUDIO **//
		float outBuffer[2] = {0, 0}; // buffer for the output
		for (int i = 0; i < 16; i++) // loop through all the read pointers
		{
			if (gReadPointers[i] >= 0 && gDrumBufferForReadPointer[i] != -1) // if the read pointer is active
			{
				reverseSamples(gPlaysBackwards, i); // reverse the samples if gPlaysBackwards is true
			}

			if (gReadPointers[i] >= 0) // if the read pointer is active
				for (unsigned int channel = 0; channel < context->audioOutChannels; channel++)
				{
					outBuffer[channel] += gDrumSampleBuffers[gDrumBufferForReadPointer[i]][gReadPointers[i]];
				}
		}

		for (unsigned int channel = 0; channel < context->audioOutChannels; channel++) // write the output
		{

			audioWrite(context, n, channel, outBuffer[channel]); // write the output
		}
	}
}

//** FUNCTION DEFINITIONS **//
void startPlayingDrum(int drumIndex) // Start playing a particular drum sound given by drumIndex
{
	for (int i = 0; i < 16; i++)
	{
		if (gReadPointers[i] == -1) // Find a free read pointer
		{
			if (gPlaysBackwards)											// If we are playing backwards, start at the end of the buffer
				gReadPointers[i] = gDrumSampleBufferLengths[drumIndex] - 1; // Set the read pointer to the end of the buffer
			else
				gReadPointers[i] = 0;				  // Set the read pointer to the beginning of the buffer
			gDrumBufferForReadPointer[i] = drumIndex; // Set the drum buffer for the read pointer
			break;
		}
	}
	return; // If no read pointers are free, return without playing.
}

void startNextEvent()
{
	if (gCurrentIndexInPattern >= gPatternLengths[gCurrentPattern]) // check if we've reached the end of the pattern
	{
		if (gCurrentPattern == FILL_PATTERN) // check if current pattern is a fill pattern
		{
			gShouldPlayFill = 0;			 // reset gShouldPlayFill
			updatePattern(gPreviousPattern); // switch back to previous pattern
		}
		gCurrentIndexInPattern = 0;
	}

	for (int i = 0; i < NUMBER_OF_DRUMS; i++) // check if event contains a drum for each drum sound
	{
		if (eventContainsDrum(gPatterns[gCurrentPattern][gCurrentIndexInPattern], i)) // if it does
			startPlayingDrum(i);													  // if it does, start playing the drum sound
	}

	gCurrentIndexInPattern++; // increment the index in the pattern
}

void updatePattern(int newPattern) // Update the current pattern to a new pattern
{
	if (gPreviousPattern != newPattern)
	{
		gPreviousPattern = gCurrentPattern;										// Store the previous pattern
		gCurrentPattern = newPattern;											// Set the current pattern to the new pattern
		int newPatternLength = gPatternLengths[gCurrentPattern];				// Get the length of the new pattern
		if (gCurrentPattern != FILL_PATTERN)									// If the new pattern is not a fill pattern
			gCurrentIndexInPattern = gCurrentIndexInPattern % newPatternLength; // Use modulo arithmetic to ensure gCurrentIndexInPattern is within bounds
		else																	// If the new pattern is a fill pattern
			gCurrentIndexInPattern = 0;											// Reset the index in the pattern to 0
	}
}

int eventContainsDrum(int event, int drum) // Check if an event contains a particular drum sound
{
	if (event & (1 << drum)) // Check if the bit corresponding to the drum is set
		return 1;			 // If it is, return 1
	return 0;
}

void reverseSamples(bool reverse, int i) // Reverse the sample if reverse is true
{
	if (reverse) // if we are playing backwards
	{
		gReadPointers[i]--; // decrement the read pointer

		if (gReadPointers[i] < 0) // if the read pointer is less than 0
		{
			gReadPointers[i] = -1;			   // reset the read pointer
			gDrumBufferForReadPointer[i] = -1; // reset the drum buffer for the read pointer
		}
	}
	else
	{
		gReadPointers[i]++;																// increment the read pointer
		if (gReadPointers[i] >= gDrumSampleBufferLengths[gDrumBufferForReadPointer[i]]) // if the read pointer is greater than the length of the buffer
		{
			gReadPointers[i] = -1;			   // reset the read pointer
			gDrumBufferForReadPointer[i] = -1; // reset the drum buffer for the read pointer
		}
	}
}

void cleanup(BelaContext *context, void *userData)
{
}
