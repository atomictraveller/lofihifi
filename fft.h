
void fft(int fftn, float* real, float* imag) {
	register int j, k;
	nm1 = powtwo[fftn] - 1;	nd2 = powtwo[fftn] >> 1;
	int jm1, ip;	register float ur, ui, tr, ti;
	j = nd2;
	for (int i = 1; i <= powtwo[fftn] - 2; i++) {
		if (i < j) {
			tr = real[j];		ti = imag[j];
			real[j] = real[i];	imag[j] = imag[i];
			real[i] = tr;		imag[i] = ti;
		}
		k = nd2;
		while (k <= j) {j -= k;	k /= 2;}
		j += k;
	}
	for (k = 1; k <= fftn; k++) {
		int le = powtwo[k];	int le2 = le >> 1;
		ur = 1.f;	ui = 0.f;
		for (j = 1; j <= le2; j++) {
			jm1	= j - 1;
			for (int i = jm1; i <= nm1; i += le) {
				ip = i + le2;
				tr = real[ip] * ur - imag[ip] * ui;
				ti = real[ip] * ui + imag[ip] * ur;
				real[ip] = real[i] - tr;
				imag[ip] = imag[i] - ti;
				real[i] = real[i] + tr;
				imag[i] = imag[i] + ti;
			}
			tr = ur;
			ur = tr * zr[k] - ui * zi[k];
			ui = tr * zi[k] + ui * zr[k];
	}	}
}


void drawwav() {
	mins = samples / (wavsamplerate * 60);
	secs = (samples - mins * wavsamplerate * 60) / wavsamplerate;
	wgrlen = samples / 800;
	wgrprog = posmins = possecs = 0;

	double r10 = std::log(10);
	//vertical indices
	for (int i = 20; i <= 100; i += 10) {
		double ix = 2.0 * 117.8797686062508286505044320866 * std::log(i) / r10;
		for (int j = 260; j < 453; j++) SetPixel(hdcbackground, (int)ix - 206, j, RGB(80, 80, 80));	//	64 + (int)ix
	}
	for (int i = 200; i <= 1000; i += 100) {
		double ix = 2.0 * 117.8797686062508286505044320866 * std::log(i) / r10;
		for (int j = 260; j < 453; j++) SetPixel(hdcbackground, (int)ix - 206, j, RGB(80, 80, 80));
	}
	for (int i = 2000; i <= 10000; i += 1000) {
		double ix = 2.0 * 117.8797686062508286505044320866 * std::log(i) / r10;
		for (int j = 260; j < 453; j++) SetPixel(hdcbackground, (int)ix - 206, j, RGB(80, 80, 80));
	}
	for (int i = 20000; i <= 20000; i += 10000) {
		double ix = 2.0 * 117.8797686062508286505044320866 * std::log(i) / r10;
		for (int j = 260; j < 453; j++) SetPixel(hdcbackground, (int)ix - 206, j, RGB(80, 80, 80));
	}

    TextOutA(hdcbackground, 840, 4, "samples", 7);
    TextOutA(hdcbackground, 942, 4, ":", 1);
    TextOutA(hdcbackground, 972, 4, "s", 1);
    TextOutA(hdcbackground, 925, 108, "loop min:", 9);
    TextOutA(hdcbackground, 925, 128, "loop max:", 9);
    SetTextColor(hdcbackground, RGB(175, 150, 125));
    TextOutA(hdcbackground, 910, 70, "-", 1);   TextOutA(hdcbackground, 960, 70, "s", 1);  TextOutA(hdcbackground, 980, 70, "+", 1);
    TextOutA(hdcbackground, 910, 168, "-", 1);  TextOutA(hdcbackground, 960, 168, "s", 1);  TextOutA(hdcbackground, 980, 168, "+", 1);

	SetTextColor(hdcbackground, RGB(128, 128, 128));
	TextOutA(hdcbackground, 818, 260 - 7, "0 dB", 4);	//	580 for 1x horiz scale
	TextOutA(hdcbackground, 818, 292 - 7, "-20 dB", 6);
	TextOutA(hdcbackground, 818, 324 - 7, "-40 dB", 6);
	TextOutA(hdcbackground, 818, 356 - 7, "-60 dB", 6);
	TextOutA(hdcbackground, 818, 388 - 7, "-80 dB", 6);
	TextOutA(hdcbackground, 818, 420 - 7, "-100 dB", 7);
	//TextOutA(hdcbackground, 580, 452 - 7, "-120 dB", 7);
	TextOutA(hdcbackground, 818, 453 - 5, "+180 deg.", 9);
	TextOutA(hdcbackground, 818, 493 - 5, "0 deg.", 6);
	TextOutA(hdcbackground, 818, 553 - 5, "-180 deg.", 9);


	//TextOutA(hdcframe, 125, 229, "drawing waveform please wait..", 30);
	for (int i = 100; i < 809; i++) {    //  "beginning" should be 74 not 100// 572 was end at x1 horizontal display
		SetPixel(hdcbackground, i, 260, RGB(50, 50, 50));
		SetPixel(hdcbackground, i, 292, RGB(50, 50, 50));
		SetPixel(hdcbackground, i, 324, RGB(50, 50, 50));
		SetPixel(hdcbackground, i, 356, RGB(50, 50, 50));
		SetPixel(hdcbackground, i, 388, RGB(50, 50, 50));
		SetPixel(hdcbackground, i, 420, RGB(50, 50, 50));
		SetPixel(hdcbackground, i, 452, RGB(50, 50, 50));

		SetPixel(hdcbackground, i, 493, RGB(60, 60, 60));
	}
}

