
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

	for (int i = 10; i < 66; i++) for (int j = 250; j < 270; j++) SetPixel(hdcbackground, i, j, RGB(32, 28, 24));
	BitBlt(hdcbackground, 10, 282, 66, 20, hdcbackground, 10, 250, SRCCOPY);
	BitBlt(hdcbackground, 10, 314, 66, 20, hdcbackground, 10, 250, SRCCOPY);
	BitBlt(hdcbackground, 10, 346, 66, 20, hdcbackground, 10, 250, SRCCOPY);
	BitBlt(hdcbackground, 10, 378, 66, 20, hdcbackground, 10, 250, SRCCOPY);
	BitBlt(hdcbackground, 10, 410, 66, 20, hdcbackground, 10, 250, SRCCOPY);
	BitBlt(hdcbackground, 10, 442, 66, 20, hdcbackground, 10, 250, SRCCOPY);
	BitBlt(hdcbackground, 10, 474, 66, 20, hdcbackground, 10, 250, SRCCOPY);
	BitBlt(hdcbackground, 10, 506, 66, 20, hdcbackground, 10, 250, SRCCOPY);

	TextOutA(hdcbackground, 18, 260 - 7, ".001", 4);
	TextOutA(hdcbackground, 18, 292 - 7, ".0003", 5);
	TextOutA(hdcbackground, 18, 324 - 7, ".0001", 5);
	TextOutA(hdcbackground, 18, 356 - 7, ".00003", 6);
	TextOutA(hdcbackground, 18, 388 - 7, ".00001", 6);
	TextOutA(hdcbackground, 18, 420 - 7, "1e-17", 5);
	TextOutA(hdcbackground, 18, 452 - 7, "1e-18", 5);
	TextOutA(hdcbackground, 18, 484 - 7, "1e-19", 5);
	TextOutA(hdcbackground, 18, 516 - 7, "0", 1);

    SetTextColor(hdcbackground, RGB(80, 80, 80));
    TextOutA(hdcbackground, 5, 243 - 7, "learn", 5);
    TextOutA(hdcbackground, 5, 403 - 7, "reg", 3);
}





void train() {


        
        fft(9, wff0, wff1);
        for (unsigned int i = 0; i < 207; i++) {	//	cartesian to polar .. 206 is the highest bin used, 256 = nyquist
            wmag[i] = sqrt(wff0[i] * wff0[i] + wff1[i] * wff1[i]);
            if (wff0[i] == 0) wpha[i] = 0.f;
            else {
                wpha[i] = atan(wff1[i] / wff0[i]);
                if (wff0[i] < 0) wff1[i] < 0 ? wpha[i] -= pi : wpha[i] += pi;
            }
            wmag[i] = log10(wmag[i]); magi[i] = (int)(wmag[i] * 32.f);
            wmag[i] = wmag[i] * 0.2f;
            wpha[i] *= 0.2f;
        }
for (unsigned int i = 117; i < 207; i++) wmag[i] += 0.4f;
        //feedforward neural network or multilayer perceptron.
        // tanh(14.3932) = .999999, tanh(11.548) = .99999, tanh(9.9668) = .9999
        for (unsigned int i = 0; i < hidd; i++) { //  size of hidden..    input to hidden
            float hsum = 0.f;
            for (int j = 1; j < 117; j++) { //  input is 232, 116 mag bins 116 phase bins
                hsum += wmag[j] * nni[j - 1][i];
                hsum += wpha[j] * nni[j + 115][i];
            }
            //nnh[i] = (std::exp(hsum) - std::exp(-hsum)) / (std::exp(hsum) + std::exp(-hsum)); //   "stable tanh"
            nnh[i] = std::tanh(hsum);
            //hsum < 11.f ? nnh[i] = tanh(hsum) : nnh[i] = .99999f;
            //nnh[i] = 1.f / (1.f + exp(-hsum));    //  sigmoid
        }

        /*for (unsigned int i = 0; i < hidd; i++) { //  hidden to hidden
            float hsum = 0.f;
            for (unsigned int j = 0; j < hidd; j++) hsum += nnh4[j] * nn4[j][i];
            nnh3[i] = std::tanh(hsum);
        }
        for (unsigned int i = 0; i < hidd; i++) { //  hidden to hidden
            float hsum = 0.f;
            for (unsigned int j = 0; j < hidd; j++) hsum += nnh3[j] * nn3[j][i];
            nnh2[i] = std::tanh(hsum);
        }
        for (unsigned int i = 0; i < hidd; i++) { //  hidden to hidden
            float hsum = 0.f;
            for (unsigned int j = 0; j < hidd; j++) hsum += nnh2[j] * nn2[j][i];
            nnh1[i] = std::tanh(hsum);
        }
        for (unsigned int i = 0; i < hidd; i++) { //  hidden to hidden
            float hsum = 0.f;
            for (unsigned int j = 0; j < hidd; j++) hsum += nnh1[j] * nn1[j][i];
            nnh[i] = std::tanh(hsum);
        }*/

        for (unsigned int i = 0; i < 180; i++) { //  size of output      hidden to output
            float osum = 0.f;
            for (unsigned int j = 0; j < hidd; j++) osum += nnh[j] * nno[j][i];
            nnr[i] = std::tanh(osum);
        }
        Lreg = 0.f;  //  regularization .. method of avoiding local minima L2 = x*x, L1 = abs
        readm = readph = 0.f;
        for (unsigned int i = 0; i < 40; i++) {  //  error = prediction - target
            int j = i + 90;
            float tsu;
            tsu = wmag[i + 117] - nnr[i];   readm += tsu * tsu; nnoe[i] = tsu;
            tsu = wpha[i + 117] - nnr[j];   readph += tsu * tsu; nnoe[j] = tsu;
        }
        for (unsigned int i = 40; i < 90; i++) {  //  error = prediction - target
            int j = i + 90;
            float tsu;
            tsu = wmag[i + 117] - nnr[i];   readm += tsu * tsu; nnoe[i] = tsu;
            nnoe[j] = wpha[i + 117] - nnr[j];
        }

        merr += readm;  perr += readph;
        Lreg = readm + readph * .1f;//.2f;
        Lreg *= regs;
        //for (int i = 0; i < 180; i++) nnoe[i] = nnoe[i] * nnr[i] * (1.f - nnr[i]);    //  derivative of sigmoid
        for (unsigned int i = 0; i < 180; i++) {
            float dta = std::tanh(nnr[i]); dta = 1.f - dta * dta; //  derivative of tanh
            nnoe[i] = nnoe[i] * dta;
        }
        for (unsigned int i = 0; i < hidd; i++) { //  size of hidden      output to hidden
            float hsum = 0.f;
            for (int j = 0; j < 180; j++) hsum += nnoe[j] * nno[i][j];
            float dta = std::tanh(nnh[i]); dta = 1.f - dta * dta;
            nnhe[i] = hsum * dta;
        }

        /*for (unsigned int i = 0; i < hidd; i++) { //  hidden to hidden backprop
            float hsum = 0.f;
            for (int j = 0; j < hidd; j++) hsum += nnhe[j] * nn1[i][j];
            float dta = std::tanh(nnh[i]); dta = 1.f - dta * dta;
            nnhe1[i] = hsum * dta;
        }
        for (unsigned int i = 0; i < hidd; i++) { //  hidden to hidden backprop
            float hsum = 0.f;
            for (int j = 0; j < hidd; j++) hsum += nnhe1[j] * nn2[i][j];
            float dta = std::tanh(nnh[i]); dta = 1.f - dta * dta;
            nnhe2[i] = hsum * dta;
        }
        for (unsigned int i = 0; i < hidd; i++) { //  hidden to hidden backprop
            float hsum = 0.f;
            for (int j = 0; j < hidd; j++) hsum += nnhe2[j] * nn3[i][j];
            float dta = std::tanh(nnh[i]); dta = 1.f - dta * dta;
            nnhe3[i] = hsum * dta;
        }
        for (unsigned int i = 0; i < hidd; i++) { //  hidden to hidden backprop
            float hsum = 0.f;
            for (int j = 0; j < hidd; j++) hsum += nnhe3[j] * nn4[i][j];
            float dta = std::tanh(nnh[i]); dta = 1.f - dta * dta;
            nnhe4[i] = hsum * dta;
        }*/

        if (tests < 99000) for (unsigned int i = 0; i < 180; i++) {    //  begin updating weights
            for (unsigned int j = 0; j < hidd; j++) {
                nno[j][i] += learn * nnoe[i] * nnh[j] + Lreg;
            }
        }
        else for (unsigned int i = 0; i < 180; i++) {    //  stops regulation near end of training
            for (unsigned int j = 0; j < hidd; j++) {
                nno[j][i] += learn * nnoe[i] * nnh[j];
            }
        }


        /*for (unsigned int i = 0; i < hidd; i++) {
            for (unsigned int j = 0; j < hidd; j++) {
                nn1[j][i] += learn * nnhe[i] * nnh1[j];
            }
        }
        for (unsigned int i = 0; i < hidd; i++) {
            for (unsigned int j = 0; j < hidd; j++) {
                nn2[j][i] += learn * nnhe1[i] * nnh2[j];
            }
        }
        for (unsigned int i = 0; i < hidd; i++) {
            for (unsigned int j = 0; j < hidd; j++) {
                nn3[j][i] += learn * nnhe2[i] * nnh3[j];
            }
        }
        for (unsigned int i = 0; i < hidd; i++) {
            for (unsigned int j = 0; j < hidd; j++) {
                nn4[j][i] += learn * nnhe3[i] * nnh4[j];
            }
        }*/

        for (unsigned int i = 0; i < hidd; i++) {
            for (int j = 0; j < 116; j++) {
                nni[j][i] += learn * nnhe[i] * wmag[j + 1];
                nni[j + 116][i] += learn * nnhe[i] * wpha[j + 1];
            }
        }

}