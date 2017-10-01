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
int track_1_level = 1;
int track_2_level = 1;
int last_track_1_level = 0;
int last_track_2_level = 0;
int delay_setting = 0;
int last_delay_setting = -1; // Last position of the delay knob
int fx_param_select = 0;
int enc_debounce = 250;
int encoderPosition [] = {0, 0, 0};      // current state of the encoders
int lastEncoderPosition[] = {0, 0, 0};     // previous state of the buttons

// Values for the delay
//                   	 	 Initial   	1    	2    	3    	4   	 5
const int delay_0[] = 		{   0, 		107, 	143, 	214, 	286, 	425 	};
const int delay_1[] = 		{   0,  	54,  	72, 	107, 	143, 	213 	};
const int delay_2[] = 		{   0,  	72, 	107, 	143, 	213, 	286 	};
const int delay_3[] = 		{   0,  	143, 	214, 	286, 	107, 	143 	};
const int delay_send_1[] =  {   0,  	250, 	400,	650, 	850,	1023	};
const int delay_send_2[] =  {   0,  	250, 	400,	650, 	850,	1023	};
const int delay_fb[] =    	{   0,  	250, 	400,	650, 	850,	1023	};

//millisecond counters
elapsedMillis m_debounce1;
elapsedMillis m_debounce2;
elapsedMillis m_debounce3;
Encoder encoder0(25, 24);
Encoder encoder1(27, 26);
Encoder encoder2(29, 28);

//=================Setup================
void setup() {
	AudioMemory(160);	    
    Serial.begin( 9600 );
    SPI.setMOSI(SDCARD_MOSI_PIN);
    SPI.setSCK(SDCARD_SCK_PIN);
     if (!(SD.begin(SDCARD_CS_PIN))) {              //To do: change to start in different mode when sd missing
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
	mixer3.gain(2, 0.25);	//fb sends
	mixer3.gain(3, 0.25);
	mixer4.gain(0, 0.25);
	mixer4.gain(1, 0.25);
	mixer5.gain(0, 0.5);	//mixer to delay
	mixer5.gain(1, 0.5);
	mixer6.gain(0, 0.5);	//mix 1 and 2
	mixer6.gain(1, 0.5);
	mixer7.gain(0, 0.25);	//delay outs
	mixer7.gain(1, 0.25);
	mixer7.gain(2, 0.25);
	mixer7.gain(3, 0.25);
	mixer8.gain(0, 1.0);	//unnecessary
	mixer9.gain(0, 0.25);	//fx mix
	mixer9.gain(1, 0.25);
	mixer9.gain(2, 0.25);
	mixer10.gain(0, 0.0);	//send to modulation
	mixer10.gain(1, 0.0);
	mixer11.gain(0, 0.5);	//mix with effects
	mixer11.gain(1, 0.5);	//mix with effects
    sgtl5000_1.enable();
    sgtl5000_1.volume(0.5);	   //master volume
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
  mixer6.gain(0, gain2);
  mixer6.gain(1, gain1);
}

//Set delay times based on values in array                       !!!!!!!!!!!!!!!
void setDelayTime(int delay_setting) {
  if ((last_delay_setting != delay_setting) && (delay_setting >= 0) && (delay_setting <= 5)) {
    last_delay_setting = delay_setting;
	Serial.print("delay setting ");
	Serial.println(delay_setting);
    delay1.delay(0, delay_0[delay_setting]);
    //delay1.delay(1, delay_1[delay_setting]);
    //delay1.delay(2, delay_2[delay_setting]);
    //delay1.delay(3, delay_3[delay_setting]);
  }
}

//Send wav 1 to delay
void setDelaySendGain1(int level) {
  // wav 1 effect send
  float gain = (float)delay_send_1[level] / 1023.0;
  mixer3.gain(0, gain);
}

//Send wav 2 to delay
void setDelaySendGain2(int level) {
  // wav 1 effect send
  float gain = (float)delay_send_2[level] / 1023.0;
  mixer3.gain(1, gain);
}

//delay fb
void setDelayFb(int level) {
  // wav 1 effect send
  float gain = (float)delay_fb[level] / 1023.0;
  mixer5.gain(0, gain);
  mixer5.gain(1, gain);
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
	//read encoder, if different, increment up or down to limits
	encoderPosition[2] = encoder2.read();
	if ( encoderPosition[2] > 4 ) {
		encoder1.write(4);
	}
	if ( encoderPosition[2] < 0 ) {
		encoder1.write(0);
	}
	if ( encoderPosition[2] > lastEncoderPosition[2] && encoderPosition[2] < 5 && m_debounce3 > enc_debounce ) {
		delay_setting += 1;
		lastEncoderPosition[2] = encoderPosition[2];
		m_debounce3 = 0;
	}
	else if ( encoderPosition[2] < lastEncoderPosition[2] && encoderPosition[2] > -1 && m_debounce3 > enc_debounce ) {
		delay_setting -= 1;
		lastEncoderPosition[2] = encoderPosition[2];
		m_debounce3 = 0;	
	}
}

//======================LOOP=======================
void loop() {
//play wav 1
	//track_1_level = encoderPosition[0];
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
	
	setMasterVolume(analogRead(2));
	setCrossfade(analogRead(3));
	checkEncoder0();
	checkEncoder1();
	//checkEncoder2();
	//setDelayTime(delay_setting);
	//setDelaySendGain1(delay_setting);
	//setDelaySendGain2(delay_setting);
		
}