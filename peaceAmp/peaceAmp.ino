/*  based on work from mozzi examples by 
    Tim Barrass 2012, CC by-nc-sa.
*/

#include <MozziGuts.h>
#include <Oscil.h>
#include <tables/cos8192_int8.h>
#include <mozzi_rand.h>
#include <mozzi_midi.h>
#include <mozzi_fixmath.h> // for Q16n16 fixed-point fractional number type

// harmonics
Oscil<COS8192_NUM_CELLS, AUDIO_RATE> aCos1(COS8192_DATA);
Oscil<COS8192_NUM_CELLS, AUDIO_RATE> aCos2(COS8192_DATA);
Oscil<COS8192_NUM_CELLS, AUDIO_RATE> aCos3(COS8192_DATA);
Oscil<COS8192_NUM_CELLS, AUDIO_RATE> aCos4(COS8192_DATA);
Oscil<COS8192_NUM_CELLS, AUDIO_RATE> aCos5(COS8192_DATA);
//Oscil<COS8192_NUM_CELLS, AUDIO_RATE> aCos6(COS8192_DATA);
//Oscil<COS8192_NUM_CELLS, AUDIO_RATE> aCos7(COS8192_DATA); // used to work smoothly in Arduino 1.05

// duplicates but slightly off frequency for adding to originals
Oscil<COS8192_NUM_CELLS, AUDIO_RATE> aCos1b(COS8192_DATA);
Oscil<COS8192_NUM_CELLS, AUDIO_RATE> aCos2b(COS8192_DATA);
Oscil<COS8192_NUM_CELLS, AUDIO_RATE> aCos3b(COS8192_DATA);
Oscil<COS8192_NUM_CELLS, AUDIO_RATE> aCos4b(COS8192_DATA);
Oscil<COS8192_NUM_CELLS, AUDIO_RATE> aCos5b(COS8192_DATA);
//Oscil<COS8192_NUM_CELLS, AUDIO_RATE> aCos6b(COS8192_DATA);
//Oscil<COS8192_NUM_CELLS, AUDIO_RATE> aCos7b(COS8192_DATA);

// base pitch frequencies in Q16n16 fixed int format (for speed later)
Q16n16 f1,f2,f3,f4,f5; //,f6;//,f7;


Q16n16 variation() {
  // 32 random bits & with 524287 (b111 1111 1111 1111 1111)
  // gives between 0-8 in Q16n16 format
  return  (Q16n16) (xorshift96() & 524287UL);
}


//setup sketch 1
#include <tables/sin2048_int8.h> // sine table for oscillator
#include <RollingAverage.h>
#include <ControlDelay.h>


unsigned int echo_cells_1 = 32;
unsigned int echo_cells_2 = 60;
unsigned int echo_cells_3 = 127;

ControlDelay <128, int> kDelay; // 2seconds

// oscils to compare bumpy to averaged control input
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aSin0(SIN2048_DATA);
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aSin1(SIN2048_DATA);
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aSin2(SIN2048_DATA);
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aSin3(SIN2048_DATA);

// use: RollingAverage <number_type, how_many_to_average> myThing
RollingAverage <int, 32> kAverage; // how_many_to_average has to be power of 2
int averaged;

//sketch 3

#include <tables/triangle_valve_2048_int8.h>
#include <Line.h>
// audio oscillator
Oscil<TRIANGLE_VALVE_2048_NUM_CELLS, AUDIO_RATE> aSig(TRIANGLE_VALVE_2048_DATA);
// control oscillator for tremelo
Oscil<SIN2048_NUM_CELLS, CONTROL_RATE> kTremelo(SIN2048_DATA);
// a line to interpolate control tremolo at audio rate
Line <unsigned int> aGain;


//sketch4

#include <Phasor.h>

Phasor <AUDIO_RATE> aPhasor1;
Phasor <AUDIO_RATE> aPhasor2;
Phasor <AUDIO_RATE> aPhasor3;
Phasor <AUDIO_RATE> aPhasor4;
Phasor <AUDIO_RATE> aPhasor5;
Phasor <AUDIO_RATE> aPhasor6;


float pfreq1 = 55.f;
float pfreq2 = 55.f;
float pfreq3 = 55.f;

//SKETCH 5
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aGain2(SIN2048_DATA); // to fade audio signal in and out before waveshaping
Q16n16 s5freq1 = Q8n0_to_Q16n16(440);


//setup control
#include <Bounce2.h>
#define BUTTON_PIN 6
Bounce b = Bounce();
 
const int numberOfSketches = 5;
int currentSketch = 0;
int buttonState = 0;         // variable for reading the pushbutton status


#define INPUT_POT1 0 // analog control input
#define INPUT_POT2 3 // analog control input
#define INPUT_POT3 7 // analog control input
#define CONTROL_RATE 64

int pot1 = 0;
int pot2 = 0;
int pot3 = 0;



void setup(){
   Serial.begin(9600);
  //setup control
  b.attach(BUTTON_PIN,INPUT_PULLUP); // Attach the debouncer to a pin with INPUT_PULLUP mode
  b.interval(25); // Use a debounce interval of 25 milliseconds
   //start mozzi
  startMozzi(CONTROL_RATE);


  //setup sketch 0
  // select base frequencies using mtof (midi to freq) and fixed-point numbers
  f1 = Q16n16_mtof(Q16n0_to_Q16n16(48));
  f2 = Q16n16_mtof(Q16n0_to_Q16n16(74));
  f3 = Q16n16_mtof(Q16n0_to_Q16n16(64));
  f4 = Q16n16_mtof(Q16n0_to_Q16n16(77));
  f5 = Q16n16_mtof(Q16n0_to_Q16n16(67));
//  f6 = Q16n16_mtof(Q16n0_to_Q16n16(57));
  // f7 = Q16n16_mtof(Q16n0_to_Q16n16(60));

  // set Oscils with chosen frequencies
  aCos1.setFreq_Q16n16(f1);
  aCos2.setFreq_Q16n16(f2);
  aCos3.setFreq_Q16n16(f3);
  aCos4.setFreq_Q16n16(f4);
  aCos5.setFreq_Q16n16(f5);
//  aCos6.setFreq_Q16n16(f6);
  // aCos7.setFreq_Q16n16(f7);

  // set frequencies of duplicate oscillators
  aCos1b.setFreq_Q16n16(f1+variation());
  aCos2b.setFreq_Q16n16(f2+variation());
  aCos3b.setFreq_Q16n16(f3+variation());
  aCos4b.setFreq_Q16n16(f4+variation());
  aCos5b.setFreq_Q16n16(f5+variation());
//  aCos6b.setFreq_Q16n16(f6+variation());
  //aCos7b.setFreq_Q16n16(f7+variation());

  //setup sketch 1
  kDelay.set(echo_cells_1);

  //setup sketch 2
  aSig.setFreq(mtof(65.f));
  kTremelo.setFreq(5.5f);

  //setup sketch 3
  aPhasor1.setFreq(pfreq1);
  aPhasor2.setFreq(pfreq1+0.2f);
  aPhasor3.setFreq(pfreq2);
  aPhasor4.setFreq(pfreq2+0.2f);
  aPhasor5.setFreq(pfreq3);
  aPhasor6.setFreq(pfreq3+0.2f);


  //SETUP SKETCH 4
  aSin1.setFreq_Q16n16(s5freq1); // set the frequency with a Q16n16 fractional number
  aGain2.setFreq(0.2f); // use a float for low frequencies, in setup it doesn't need to be fast
}



void updateControl() {
  pot1 = mozziAnalogRead(INPUT_POT1);
  pot2 = mozziAnalogRead(INPUT_POT2);
  pot3 = mozziAnalogRead(INPUT_POT3);
  b.update(); // Update the Bounce instance 
   if ( b.fell() ) {  // Call code if button transitions from HIGH to LOW
     currentSketch++;
     if (currentSketch >= numberOfSketches){
        currentSketch = 0;
      }
   }
//   Serial.println(currentSketch);
  switch (currentSketch){ 
    case 0:
      updateControlSketch0();
//      Serial.println("current sketch = 0");
//    break;

    case 1:
      updateControlSketch1();
//      Serial.println(pot1);
//    break;

    case 2:
      updateControlSketch2();
    case 3:
      updateControlSketch3();
   case 4:
      updateControlSketch4();
  }
}

void updateControlSketch0(){
//  int bumpy_input = pot1;
  averaged = kAverage.next(pot1);
  aSin0.setFreq(averaged);
  aSin1.setFreq(kDelay.next(averaged));
  aSin2.setFreq(kDelay.read(echo_cells_2));
  aSin3.setFreq(kDelay.read(echo_cells_3));
}


void updateControlSketch1(){
  f1 = Q16n16_mtof(Q16n0_to_Q16n16(pot1/16 + 40));
  f2 = Q16n16_mtof(Q16n0_to_Q16n16(pot2/32 + 30));
  f3 = Q16n16_mtof(Q16n0_to_Q16n16(pot3/16 + 20));
  // todo: choose a nice scale or progression and make a table for it
  // or add a very slow gliss for f1-f7, like shephard tones
Serial.println(f1);
  // change frequencies of the b oscillators
  switch (lowByte(xorshift96()) & 7){ // 7 is 0111

    case 0:
      aCos1b.setFreq_Q16n16(f1+variation());
    break;

    case 1:
       aCos2b.setFreq_Q16n16(f2+variation());
    break;

    case 2:
       aCos3b.setFreq_Q16n16(f3+variation());
    break;

    case 3:
       aCos4b.setFreq_Q16n16(f4+variation());
    break;

    case 4:
       aCos5b.setFreq_Q16n16(f5+variation());
    break;

//    case 5:
//       aCos6b.setFreq_Q16n16(f6+variation());
//    break;
    /*
    case 6:
       aCos7b.setFreq_Q16n16(f7+variation());
    break;
    */
  }
}

void updateControlSketch2(){
  // gain shifted up to give enough range for line's internal steps
    aSig.setFreq(mtof(pot1/16.f));
    kTremelo.setFreq(pot2/32.f);
   unsigned int gain = (128u+kTremelo.next())<<8;
   aGain.set(gain, AUDIO_RATE / CONTROL_RATE); // divide of literals should get optimised away
}

void updateControlSketch3(){
  pfreq1 = mtof(pot1/16.f);
  pfreq2 = mtof(pot2/16.f);
  pfreq3 = mtof(pot3/16.f);
  aPhasor1.setFreq(pfreq1);
  aPhasor2.setFreq(pfreq1+0.2f);
  aPhasor3.setFreq(pfreq2);
  aPhasor4.setFreq(pfreq2+0.2f);
  aPhasor5.setFreq(pfreq3);
  aPhasor6.setFreq(pfreq3+0.2f);
  
}


void updateControlSketch4(){
    // change proportional frequency of second tone
    byte harmonic = (byte)(int)pot1/4;
    byte shimmer = (byte)(int)pot2;
    Q16n16 harmonic_step = s5freq1/12;
    Q16n16 freq2difference = harmonic*harmonic_step;
    freq2difference += (harmonic_step*shimmer)>>11;
    Q16n16 freq2 = s5freq1-freq2difference;
    aSin2.setFreq_Q16n16(freq2); // set the frequency with a Q16n16 fractional number
}


AudioOutput_t updateAudio0(){
  return MonoOutput::fromAlmostNBit(12,
    3*((int)aSin0.next()+aSin1.next()+(aSin2.next()>>1)
    +(aSin3.next()>>2))
  );
}

AudioOutput_t updateAudio1(){

  int asig =
    aCos1.next() + aCos1b.next() +
    aCos2.next() + aCos2b.next() +
    aCos3.next() + aCos3b.next() +
    aCos4.next() + aCos4b.next() +
    aCos5.next() + aCos5b.next(); // +
//    aCos6.next() + aCos6b.next();// +
    // aCos7.next() + aCos7b.next();

  return MonoOutput::fromAlmostNBit(12, asig);
}


AudioOutput_t updateAudio2(){
  // cast to long before multiply to give room for intermediate result,
  // and also before shift,
  // to give consistent results for both 8 and 32 bit processors.
  return MonoOutput::fromNBit(24, (int32_t) aSig.next() * aGain.next()); // shifted back to audio range after multiply
}

AudioOutput_t updateAudio3(){
  char asig1 = (char)(aPhasor1.next()>>24);
  char asig2 = (char)(aPhasor2.next()>>24);
  char asig3 = (char)(aPhasor3.next()>>24);
  char asig4 = (char)(aPhasor4.next()>>24);
  char asig5 = (char)(aPhasor5.next()>>24);
  char asig6 = (char)(aPhasor6.next()>>24);
  return MonoOutput::fromNBit(9, (int)1*(((int)asig1-asig2) + ((int)asig3-asig4) + ((int)asig5-asig6) ));
}


AudioOutput_t updateAudio4(){
  int asig = (int)((((uint32_t)aSin1.next()+ aSin2.next())*(200u+aGain2.next()))>>3);
  return MonoOutput::fromAlmostNBit(9, asig).clip();
}


AudioOutput_t updateAudio(){
  switch (currentSketch){ 
    case 0:
    return updateAudio0();
//    break;

    case 1:
    return updateAudio1();
//    break;
  
    case 2:
    return updateAudio2();
//    break;
    case 3:
    return updateAudio3();
    case 4:
    return updateAudio4();
  }
}


void loop(){
  audioHook();
}
