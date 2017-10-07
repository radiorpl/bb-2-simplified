/*
9/29/17
removing automaton and going back to one sketch because i don't want to use 
seven seg display or multiple parameters per knob any more, 
so might as well simplify the whole thing.
This version has delay and modulation (chorus + flanger)
*/

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <Encoder.h>

// GUItool: begin automatically generated code
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioPlaySdWav           playSdWav1;     //xy=236,138
AudioPlaySdWav           playSdWav2;     //xy=238,232
AudioMixer4              mixer1;         //xy=375,153
AudioMixer4              mixer2;         //xy=386,243
AudioMixer4              mixer5;         //xy=576,147
AudioMixer4              mixer3;         //xy=580,236
AudioSynthWaveform       waveform1;      //xy=764,200
AudioEffectMultiply      multiply1;      //xy=828,281
AudioMixer4              mixer4;         //xy=985,199
AudioOutputI2S           i2s1;           //xy=1141,196
AudioConnection          patchCord1(playSdWav1, 0, mixer1, 0);
AudioConnection          patchCord2(playSdWav1, 1, mixer1, 1);
AudioConnection          patchCord3(playSdWav2, 0, mixer2, 0);
AudioConnection          patchCord4(playSdWav2, 1, mixer2, 1);
AudioConnection          patchCord5(mixer1, 0, mixer3, 0);
AudioConnection          patchCord6(mixer1, 0, mixer5, 0);
AudioConnection          patchCord7(mixer2, 0, mixer3, 1);
AudioConnection          patchCord8(mixer2, 0, mixer5, 1);
AudioConnection          patchCord9(mixer5, 0, mixer4, 0);
AudioConnection          patchCord10(mixer3, 0, multiply1, 1);
AudioConnection          patchCord11(waveform1, 0, multiply1, 0);
AudioConnection          patchCord12(multiply1, 0, mixer4, 1);
AudioConnection          patchCord13(mixer4, 0, i2s1, 0);
AudioConnection          patchCord14(mixer4, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=1035,315
// GUItool: end automatically generated code




// Use these with the Teensy Audio Shield
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14

//flanger stuff
// Number of samples in each delay line
#define FLANGE_DELAY_LENGTH (8*AUDIO_BLOCK_SAMPLES)
// Allocate the delay lines for left and right channels
short flange_length[FLANGE_DELAY_LENGTH];
int offset = FLANGE_DELAY_LENGTH/4;
int depth = FLANGE_DELAY_LENGTH/4;
double rate = .5;
//chorus stuff
// Number of samples in each delay line
#define CHORUS_DELAY_LENGTH (16*AUDIO_BLOCK_SAMPLES)
// Allocate the delay lines for left and right channels
short chorus_length[CHORUS_DELAY_LENGTH];
int n_chorus = 3;

//variables
const int ANALOG_READ_AVERAGING = 32;	// Number of samples to average with each ADC reading.
int track_1_level = 1;
int track_2_level = 1;
int last_track_1_level = 0;
int last_track_2_level = 0;
int delay_setting = 0;
int last_delay_setting = 0; // Last position of the delay knob
int fx_param_select = 0;
int enc_debounce = 300;
int encoderPosition [] = {0, 0, 0};      // current state of the encoders
int lastEncoderPosition[] = {0, 0, 0};     // previous state of the buttons

//delay notes:
//fb 650-900, mix 650-1023
//1/8=425, 1/8trip=285, 
//dot1/16=321 1/16=214, 1/16trip=142, 
//dot1/32=160, 1/32=107, 1/32trip=71, 
//dot1/64=80, 1/64=53, 1/64trip=35
//1/128= 27

// Values for the delay
//                   	 	 	Initial/   	1/    	2x    	3x    	4/   	 5/		6/		7x		8x		9		10		11
const int delay_0[] = 			{   0,    	214, 	107, 	54, 	26, 	35,	 	71,		142,	285,	425,	321,	160		};
const int delay_1[] = 			{   0,  	0,  	0,  	0,  	0,  	0,  	54,  	285,  	425,  	425,  	425,  	214		};
const int delay_2[] = 			{   0,  	0,  	0,  	0,  	0,  	0, 		0,  	0,  	214,  	160,  	80,  	80  	};
const int delay_3[] = 			{   0,  	0,  	0,  	0,  	0,  	0, 		0,  	0,  	0,  	107,  	107,  	107		};
const int delay_send_1[] =  	{   0,  	1023, 	1023,	1023, 	1023,	1023,	1023, 	1023,	1023, 	1023,	1023,	1023	};
const int delay_send_2[] =  	{   0,  	1023, 	1023,	1023, 	1023,	1023,	1023, 	1023,	1023, 	1023,	1023,	1023 	};
const int delay_fb[] =    		{   0,  	650, 	650,	650, 	650,	650,	650,  	700, 	700,	900, 	800,	800		};
const int fx_mix[] =			{   0,  	650, 	650,	550, 	550,	550,	650, 	650,	700, 	750,	650,    750		};
const int offset_divider[] = 	{   2,  	3, 		4,		6, 		8,		10,		12, 	14,		16, 	18,		20,    	24		};
const int depth_divider[] = 	{   1,  	2, 		4,		6, 		8,		10,		12, 	14,		16, 	18,		20,    	24		};
const double rate_divider[] = 	{	0.01,	0.05,	0.1,	1,		5,		10, 	20,		30,		50,		70,		100,	150		};
const int sine_freq[] = 		{   196,	261,  	329,	392, 	523, 	782,	1046,	1318,	1567, 	2093,	2637, 	3135	};
//millisecond counters for encoder debouncing
elapsedMillis m_debounce1;
elapsedMillis m_debounce2;
elapsedMillis m_debounce3;
Encoder encoder0(24, 25);
Encoder encoder1(26, 27);
Encoder encoder2(28, 29);

//=================Setup================
void setup() {
	AudioMemory(160);	    
    Serial.begin( 9600 );
    SPI.setMOSI(SDCARD_MOSI_PIN);
    SPI.setSCK(SDCARD_SCK_PIN);
     if (!(SD.begin(SDCARD_CS_PIN))) {              //TODO: change to start in different mode when sd missing
      while (1) {
        Serial.println("Unable to access the SD card");   
      }   
    }
	//sets mixers for 2 wav playback, no effects
	mixer1.gain(0, 0.5); 	//wav1 stereo to mono
    mixer1.gain(1, 0.5);
    mixer2.gain(0, 0.5);	//wav2
    mixer2.gain(1, 0.5);
    mixer3.gain(0, 0.5); 	//wav sends
	mixer3.gain(1, 0.5);
	mixer4.gain(0, 0.5);	//clean/fx mix
	mixer4.gain(1, 0.5);
	mixer5.gain(0, 0.5);	//mix clean wavs
	mixer5.gain(1, 0.5);
    sgtl5000_1.enable();
    sgtl5000_1.volume(0.5);	   //master volume
	//setMod(0);
	//flange1.begin(flange_length, FLANGE_DELAY_LENGTH, offset, depth, rate);
	//chorus1.voices(n_chorus);
	//sine1.amplitude(0.5);
	//sine1.frequency(2093);
	waveform1.begin(0.5, 196, WAVEFORM_SINE);
    analogReadAveraging(ANALOG_READ_AVERAGING);
    delay(1000);
}
//=====================Functions================
//Master Vol 
void setMasterVolume(int level) {
  	// read the knob position for master volume(analog input A2)
  	float vol = (float)level / 1280.0;
  	sgtl5000_1.volume(vol);
  	//Serial.print("volume = ");
  	//Serial.println(vol);
}

//Cross fade between 2 wavs with pot
void setCrossfade(int mixKnob) {
  	// crossfader/mix both wavs
  	// knob = 0 to 1023
  	float gain1 = (float)mixKnob / 1023.0;
  	float gain2 = 1.0 - gain1;
  	mixer5.gain(0, gain2);
  	mixer5.gain(1, gain1);
}

void setMod(int delay_setting) {
	int sine_new = sine_freq[delay_setting];
	if ((last_delay_setting != delay_setting) && (delay_setting >= 0) && (delay_setting <= 11)) {
	    last_delay_setting = delay_setting;
		Serial.print("delay setting ");
		Serial.println(delay_setting);
		waveform1.frequency(sine_new);
		//sine1.frequency(sine_new);
		//flange1.voices(offset_new, depth, rate);	//change a parameter in flanger
	}
}

//Encoder - wav player 1 track selection
void checkEncoder0() {
    //read encoder, if different, increment up to limits
	if (track_1_level > 4) {
		track_1_level = 5;
	}
	if (track_1_level < 2) {
		track_1_level = 1;
	}
	if (m_debounce1 > enc_debounce) {
		encoderPosition[0] = encoder0.read();
		m_debounce1 = 0;
		//if (encoderPosition[0] != lastEncoderPosition[0]) {
		//	m_debounce1 = 0;
		//}
	    if (encoderPosition[0] > lastEncoderPosition[0]) {
			track_1_level++;
			lastEncoderPosition[0] = encoderPosition[0];
	    }
	    else if (encoderPosition[0] < lastEncoderPosition[0]) {
			track_1_level--;
			lastEncoderPosition[0] = encoderPosition[0];
		}
	}
}

//Encoder - wav player 2 track selection
void checkEncoder1() {
    //read encoder, if different, increment up to limits
	if (track_2_level > 4) {
		track_2_level = 5;
	}
	if (track_2_level < 2) {
		track_2_level = 1;
	}
	if (m_debounce2 > enc_debounce) {
		encoderPosition[1] = encoder1.read();
		m_debounce2 = 0;
		//if (encoderPosition[0] != lastEncoderPosition[0]) {
		//	m_debounce1 = 0;
		//}
	    if (encoderPosition[1] > lastEncoderPosition[1]) {
			track_2_level++;
			lastEncoderPosition[1] = encoderPosition[1];
	    }
	    else if (encoderPosition[1] < lastEncoderPosition[1]) {
			track_2_level--;
			lastEncoderPosition[1] = encoderPosition[1];
		}
	}
}

//Encoder - fx parameter
void checkEncoder2() {
    //read encoder, if different, increment up to limits
	if (delay_setting > 10) {
		delay_setting = 11;
	}
	if (delay_setting < 1) {
		delay_setting = 0;
	}
	if (m_debounce3 > enc_debounce) {
		encoderPosition[2] = encoder2.read();
		m_debounce3 = 0;
	    if (encoderPosition[2] > lastEncoderPosition[2]) {
			delay_setting++;
			lastEncoderPosition[2] = encoderPosition[2];
			//Serial.println(delay_setting);
	    }
	    else if (encoderPosition[2] < lastEncoderPosition[2]) {
			delay_setting--;
			lastEncoderPosition[2] = encoderPosition[2];
			//Serial.println(delay_setting);
		}
	}
}

//======================LOOP=======================
void loop() {
//play wav 1
	if ( (playSdWav1.isPlaying() == false) || (track_1_level != last_track_1_level) ) {
		if (track_1_level == 1) {
			playSdWav1.play("DRONE1.WAV"); //play wav file
	      	delay(10); // wait for library to parse WAV info
			last_track_1_level = track_1_level;
	      	Serial.println("PLAYER 1");
			Serial.println("DRONE1.WAV");
	    }
		else if (track_1_level == 2) {
			playSdWav1.play("DRONE2.WAV"); //play wav file
	      	delay(10); // wait for library to parse WAV info
			last_track_1_level = track_1_level;
	      	Serial.println("PLAYER 1");
			Serial.println("DRONE2.WAV");
	    }
		else if (track_1_level == 3) {
			playSdWav1.play("DRONE3.WAV"); //play wav file
	      	delay(10); // wait for library to parse WAV info
			last_track_1_level = track_1_level;
	      	Serial.println("DRONE3.WAV");
	    }
		else if (track_1_level == 4) {
			playSdWav1.play("DRONE4.WAV"); //play wav file
	      	delay(10); // wait for library to parse WAV info
			last_track_1_level = track_1_level;
	      	Serial.println("DRONE4.WAV");
	    }
		else if (track_1_level == 5) {
			playSdWav1.play("DRONE5.WAV"); //play wav file
	      	delay(10); // wait for library to parse WAV info
			last_track_1_level = track_1_level;
	      	Serial.println("DRONE5.WAV");
	    }
	}
    
//play wav 2
	if ( (playSdWav2.isPlaying() == false) || (track_2_level != last_track_2_level) ) {
		if (track_2_level == 1) {
			playSdWav2.play("DRONE1.WAV"); //play wav file
	      	delay(10); // wait for library to parse WAV info
			last_track_2_level = track_2_level;
	      	Serial.println("PLAYER 2");
			Serial.println("DRONE1.WAV");
	    }
		else if (track_2_level == 2) {
			playSdWav2.play("DRONE2.WAV"); //play wav file
	      	delay(10); // wait for library to parse WAV info
			last_track_2_level = track_2_level;
	      	Serial.println("PLAYER 2");
			Serial.println("DRONE2.WAV");
	    }
		else if (track_2_level == 3) {
			playSdWav2.play("DRONE3.WAV"); //play wav file
	      	delay(10); // wait for library to parse WAV info
			last_track_2_level = track_2_level;
	      	Serial.println("DRONE3.WAV");
	    }
		else if (track_2_level == 4) {
			playSdWav2.play("DRONE4.WAV"); //play wav file
	      	delay(10); // wait for library to parse WAV info
			last_track_2_level = track_2_level;
	      	Serial.println("DRONE4.WAV");
	    }
		else if (track_2_level == 5) {
			playSdWav2.play("DRONE5.WAV"); //play wav file
	      	delay(10); // wait for library to parse WAV info
			last_track_2_level = track_2_level;
	      	Serial.println("DRONE5.WAV");
	    }
	}
	//run commands
	//volume
	setMasterVolume(analogRead(2));
	setCrossfade(analogRead(3));
	//encoders
	checkEncoder0();
	checkEncoder1();
	checkEncoder2();
	//effect
	setMod(delay_setting);
		
}