# BelaDrumMachine
 
This code is an implementation of a sequencer-based drum machine designed to run on the Bela embedded audio platform. It utilizes sampled drum sounds and plays them in loops according to specified patterns, which can be dynamically altered based on user interaction and sensor inputs. Below is a detailed documentation of the code, including an overview of its structure, functions, and key variables.

### Overview

The program initializes with pre-defined drum patterns and loads drum samples from WAV files. It allows dynamic interaction through analog inputs, enabling the user to change patterns, modify playback behavior (e.g., reverse samples, play fills), and control playback tempo. The system's operation is responsive to an external button for start/stop functionality and utilizes a debouncer to manage button press states effectively. The implementation showcases the integration of digital and analog I/O, real-time audio processing, and effective resource management on the Bela platform.

### Key Components

#### Main Components
- **Drum Sample Management**: Pre-loads drum samples into buffers and manages these samples for playback.
- **Pattern Management**: Defines and initializes drum patterns, which dictate the sequence and timing of drum sample playback.
- **User Interaction**: Handles inputs from sensors and a button to change patterns, reverse sample playback, and control playback state.
- **Playback Control**: Manages the timing of drum events and sequences through the main audio loop, including tempo adjustments and pattern transitions.

#### Global Variables
- `gDrumSampleBuffers[]`: Stores pointers to the pre-loaded drum sample buffers.
- `gDrumSampleBufferLengths[]`: Stores the length of each drum sample buffer.
- `gPatterns[]`: Stores pointers to the arrays defining drum playback patterns.
- `gPatternLengths[]`: Stores the length of each pattern array.
- `gCurrentPattern`, `gCurrentIndexInPattern`: Control the current playback pattern and position within that pattern.
- `gEventIntervalMilliseconds`, `gCountSamples`: Used to manage the timing of drum events based on the specified tempo.
- `gIsPlaying`: Indicates whether the drum machine is currently playing.
- `gPlaysBackwards`, `gShouldPlayFill`: Flags for controlling special playback modes (e.g., reverse playback, fill patterns).

#### Functions
- **`initDrums()`**: Loads drum samples from WAV files into memory.
- **`cleanupDrums()`**: Frees allocated memory for drum samples.
- **`initPatterns()`**: Initializes the drum patterns used for playback.
- **`cleanupPatterns()`**: Frees allocated memory for drum patterns.
- **`interrupt_handler(int)`**: Handles SIGINT and SIGTERM signals for graceful shutdown.
- **`usage(const char*)`**: Prints usage information for the program.
- **`setup(BelaContext*, void*)`**: Initializes the Bela context and configures I/O settings.
- **`render(BelaContext*, void*)`**: Main audio processing loop, handling drum sample playback and user interactions.
- **`cleanup(BelaContext*, void*)`**: Cleans up resources upon program termination.
- **`startPlayingDrum(int)`**: Initiates playback of a specified drum sample.
- **`startNextEvent()`**: Triggers the next set of drum samples based on the current pattern and position.
- **`updatePattern(int)`**: Changes the current playback pattern.
- **`eventContainsDrum(int, int)`**: Checks if a specific drum should play in the current event.
- **`reverseSamples(bool, int)`**: Reverses sample playback if enabled.

#### Debouncer Implementation
The debouncer is implemented in a separate class, `Debouncer`, to manage the state of a button press, effectively filtering out the noise or bounces in the signal. It employs a simple state machine to track the current and previous states of the button, using a specified debounce interval to stabilize the input signal.

### Acknowledgements
This code was developed as part of a course on Music and Audio Programming taught at Queen Mary University of London by Adan Benito Temprano. Some elements of the code were written following tutorials given in this course.
