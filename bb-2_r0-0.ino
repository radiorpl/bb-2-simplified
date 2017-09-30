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

// GUItool: begin automatically generated code
AudioPlaySdWav           playSdWav2;     //xy=88,160
AudioPlaySdWav           playSdWav1;     //xy=89,66
AudioMixer4              mixer1;         //xy=238,84
AudioMixer4              mixer2;         //xy=238,169
AudioMixer4              mixer10;        //xy=304,531
AudioEffectDelay         delay1;         //xy=309,331
AudioMixer4              mixer3;         //xy=495,35
AudioMixer4              mixer4;         //xy=505,131
AudioEffectChorus        chorus1;        //xy=508,585
AudioEffectFlange        flange1;        //xy=514,510
AudioMixer4              mixer7;         //xy=525,381
AudioMixer4              mixer5;         //xy=678,70
AudioMixer4              mixer8;         //xy=750,354
AudioMixer4              mixer6;         //xy=753,208
AudioMixer4              mixer9;         //xy=897,310
AudioMixer4              mixer11;        //xy=1046,240
AudioOutputI2S           i2s1;           //xy=1210,237
AudioConnection          patchCord1(playSdWav2, 0, mixer2, 0);
AudioConnection          patchCord2(playSdWav2, 1, mixer2, 1);
AudioConnection          patchCord3(playSdWav1, 0, mixer1, 0);
AudioConnection          patchCord4(playSdWav1, 1, mixer1, 1);
AudioConnection          patchCord5(mixer1, 0, mixer3, 0);
AudioConnection          patchCord6(mixer1, 0, mixer6, 0);
AudioConnection          patchCord7(mixer1, 0, mixer10, 0);
AudioConnection          patchCord8(mixer2, 0, mixer3, 1);
AudioConnection          patchCord9(mixer2, 0, mixer6, 1);
AudioConnection          patchCord10(mixer2, 0, mixer10, 1);
AudioConnection          patchCord11(mixer10, flange1);
AudioConnection          patchCord12(mixer10, chorus1);
AudioConnection          patchCord13(delay1, 0, mixer3, 2);
AudioConnection          patchCord14(delay1, 0, mixer7, 0);
AudioConnection          patchCord15(delay1, 1, mixer3, 3);
AudioConnection          patchCord16(delay1, 1, mixer7, 1);
AudioConnection          patchCord17(delay1, 2, mixer7, 2);
AudioConnection          patchCord18(delay1, 2, mixer4, 0);
AudioConnection          patchCord19(delay1, 3, mixer7, 3);
AudioConnection          patchCord20(delay1, 3, mixer4, 1);
AudioConnection          patchCord21(mixer3, 0, mixer5, 0);
AudioConnection          patchCord22(mixer4, 0, mixer5, 1);
AudioConnection          patchCord23(chorus1, 0, mixer9, 3);
AudioConnection          patchCord24(flange1, 0, mixer9, 2);
AudioConnection          patchCord25(mixer7, 0, mixer8, 0);
AudioConnection          patchCord26(mixer5, delay1);
AudioConnection          patchCord27(mixer8, 0, mixer9, 1);
AudioConnection          patchCord28(mixer6, 0, mixer11, 0);
AudioConnection          patchCord29(mixer9, 0, mixer11, 1);
AudioConnection          patchCord30(mixer11, 0, i2s1, 0);
AudioConnection          patchCord31(mixer11, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=1075,409
// GUItool: end automatically generated code

// Use these with the Teensy Audio Shield
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14

// Number of samples to average with each ADC reading.
const int ANALOG_READ_AVERAGING = 32;
int track_1_level;
int track_2_level;
int last_track_1_level;
int last_track_2_level;

// Values for the delay
//                   Initial   1    2    3    4    5
const int delay_0[] = {   0, 107, 143, 214, 286, 425 };
const int delay_1[] = {   0,  54,  72, 107, 143, 213 };
const int delay_2[] = {   0,  72, 107, 143, 213, 286 };
const int delay_3[] = {   0,  72, 107, 143, 213, 286 };

void setup() {
	AudioMemory(160);	    
    Serial.begin( 9600 );
    SPI.setMOSI(SDCARD_MOSI_PIN);
    SPI.setSCK(SDCARD_SCK_PIN);
     if (!(SD.begin(SDCARD_CS_PIN))) {              //To do: change to start in different mode when sd missing
      while (1) {
 		    btn1.onPress( encBtn1, encBtn1.EVT_BTN_1 );
        Serial.println("Unable to access the SD card");   
      }   
    }
	//sets mixers for 2 wav playback, no effects
	mixer1.gain(0, 0.5); 	//wav1 stereo to mono
    mixer1.gain(1, 0.5);
    mixer2.gain(0, 0.5);	//wav2
    mixer2.gain(1, 0.5);
	mixer6.gain(0, 0.5);	//mix 1 and 2
	mixer6.gain(0, 0.5);
	mixer11.gain(0, 0.5);	//mix with effects
    sgtl5000_1.enable();
    sgtl5000_1.volume(0.5);	   
}

void SetMasterVolume(int level) {
  // read the knob position for master volume(analog input A2)
  float vol = (float)level / 1280.0;
  sgtl5000_1.volume(vol);
  //Serial.print("volume = ");
  //Serial.println(vol);
}

void SetCrossfade(int mixKnob) {
  // crossfader/mix both wavs
  // knob = 0 to 1023
  float gain1 = (float)mixKnob / 1023.0;
  float gain2 = 1.0 - gain1;
  mixer6.gain(0, gain1);
  mixer6.gain(1, gain2);
}

void SetDelayTime(int range) {
  if ((m_lastRange != range) && (range >= 0) && (range <= 5)) {
    m_lastRange = range;
    delay1.delay(0, delay_0[range]);
    delay1.delay(1, delay_1[range]);
    delay1.delay(2, delay_2[range]);
    delay1.delay(3, delay_3[range]);
  }
}

void SetWav1Gain(int level) {
  // wav 1 effect send
  float gain = (float)level / 1023.0;
  mixer2.gain(0, gain);
}

void loop() {
//play wav 1
	if ( (playSdWav1.isPlaying() == false) || (track_1_level != last_track_1_level) ) {
		if (track_1_level == 1) {
			playSdWav1.play("DRONE1.WAV"); //play wav file
	      	delay(10); // wait for library to parse WAV info
			last_track_1_level = track_1_level;
	      	Serial.println("DRONE1.WAV");
	    }
		else if (track_1_level == 2) {
			playSdWav1.play("DRONE2.WAV"); //play wav file
	      	delay(10); // wait for library to parse WAV info
			last_track_1_level = track_1_level;
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
	      	Serial.println("DRONE1.WAV");
	    }
		else if (track_2_level == 2) {
			playSdWav2.play("DRONE2.WAV"); //play wav file
	      	delay(10); // wait for library to parse WAV info
			last_track_2_level = track_2_level;
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
}