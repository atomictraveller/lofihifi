#pragma once

#include "resource.h"

#define OUT_BUFFER_SIZE		16384

HDC hdcframe;
HBITMAP oldframe;
HBITMAP framebmp;
int* framebuf;

HDC hdcbackground;		HBITMAP oldbackground;	HBITMAP backgroundbmp;	int* backgroundbuf;

HFONT hfont0, hfont1, hfont2, hfont3;

int dxm1 = 1024 - 1;
int dym1 = 576 - 1;
int dxd2 = 1024 / 2;
int dyd2 = 576 / 2;
int dispx = 1026;
int dispy = 594;

short int framerate = 20;

unsigned long int rnd;



unsigned long int samples = 0;
short int* wav0;
short int* wav1;

unsigned char loaded = 0;
unsigned long int mins = 0;
unsigned long int secs = 0;
unsigned long int wavsamplerate = 0;
unsigned long int wgrlen = 0;	//	 used for drawing waveform
unsigned long int wgrprog = 0;
unsigned long int wpos;	//	100 to 900 graphic x position
unsigned long int loadamt = 0;	//	for drawing wav load %

unsigned long int wavposition = 0;
bool playing = 0;
unsigned long int posmins = 0;
unsigned long int possecs = 0;

int logbin[256];	//	x position of each bin on screen

float loopmins = 2.f;
float loopmaxs = 5.f;

bool stop = 0;	//	stop doing stuff


int nm1, nd2;				//	fft variables dependent on buffer length
float zr[20];				//	replaced sin/cos
float zi[20];
const int powtwo[20] = { 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288 };

float wff0[512];	//	fft buffers for wav analysis
float wff1[512];
float wmag[256];
float wpha[256];
float hann[512];	//	window function scaled by 1/512 to avoid post fourier scaling
int magi[256];	//	for drawing mag on screen

float nni[232][420];	//	weights from input to hidden (232 340 180.. experimenting with size of hidden)
float nno[420][180];	//	weights from hidden to output..........640

float nn1[420][420];	//	layers are arranged backwards: input> hid2> hid1> hid> output
float nn2[420][420];	//	i decided that would be easier for rewrites
float nn3[420][420];
float nn4[420][420];


float nnr[180];			//	output layer/result

//float nnh1[420];    float nnhe1[420];	//	moved here because it was busting teh localz
//float nnh2[420];    float nnhe2[420];
//float nnh3[420];    float nnhe3[420];
//float nnh4[420];    float nnhe4[420];

float nnh[420];	//	hidden layer
float nnhe[420];//	hidden error
float nnoe[180];//	output error


float atten[25];	//	an array to fade out the top 20 bins and stop ringing there
int safearr[99999];	//	a big stupid array for stuff ok drawing where artifacts appear
unsigned char shadarr[99999];
int safect = 0;

unsigned char dlearn = 0;	//	display for learn, regs
unsigned char dreg = 1;

float rlearn[5] = {.001f, .0003f, .0001f, .00003f, .00001f};	//	reference array
float rreg[4] = {1e-17f, 1e-18f, 1e-19f, 0.f};

float learn = .001f;	//.01f; tried .04 to .001 ...001 with 1e-8 reg was ok down to 1e-11 .002 for first use, .005 current
float Lreg;
float regs = 1e-18f;
float readm;			//	magnitudes error display
float readph;			//	phases error display
float merr;				//	buffers for computing mean
float perr;
unsigned long int epochs = 0;
unsigned long int hidd = 420;	//	also arrays at top of IDM_TRAIN 640 and beginning of IDM_APPLYMODEL

unsigned long int numtests = 100000;	// tests per epoch
unsigned long int tests = 0;
unsigned long int epos = 0;	//	for drawing test position on screen

//	explains - neural net trains on phase and mag bins of 512 length fourier transforms
//	at 44100, 10kHz is 116 bins. bin 0 is discarded being dc offset since we're training harmonics
//	nn inputs 0 to 115 are mag bins 1 to 116, 116 to 231 are phase bins 1 to 116
//	nn outputs 0 to 89 are mag bins 117 to 206, 90 to 179 are phase bins 117 to 206
//	extending the spectrum to almost 18kHz

//	implementation - the fft code is in the fft.h header (with another routine to draw stuff)
//	in lofihifi.cpp the WM_COMMAND menu section opens and writes waveforms and training set data
//	the processing functions are also written here.
//	the paint section contains a switch on the (loaded) variable (named from "waveform is loaded")
//	performing output tasks for various functions. hth, there's not really any oo code
//	or multidocument craft to peck through.