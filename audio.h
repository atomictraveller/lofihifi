#pragma once

float denormal = 1.0e-18f;

void FillAudioBuffer(PBYTE pBuffer) {

	for (int buffercount = 0; buffercount < OUT_BUFFER_SIZE; buffercount += 4) {
		int16_t outl;	int16_t outr;

		//phase += .05125f;
		//while (phase > tau) phase -= tau;

		//outl = outr = sin(phase) * .0125f * amp;
		
		if (playing) {
			outl = outr = wav0[wavposition];
			wavposition++;
			if (!(wavposition < samples)) {
				playing = 0;	wavposition = samples - 1;
			}
		}
		else outl = outr = 0;

		//int aui = outl;	if (aui < -32767) aui = -32767;	else if (aui > 32767) aui = 32767;
		//int auj = outr;	if (auj < -32767) auj = -32767;	else if (auj > 32767) auj = 32767;
		//unsigned char so0 = outl, so1 = outl >> 8, so2 = outr, so3 = outr >> 8;

		BYTE so0 = outl, so2 = outr;	outl >>= 8;	outr >>= 8;
		BYTE so1 = outl, so3 = outr;
		pBuffer[buffercount] = so0;	pBuffer[buffercount + 1] = so1;	pBuffer[buffercount + 2] = so2;	pBuffer[buffercount + 3] = so3;
	}
	denormal = -denormal;

}