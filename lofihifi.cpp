// lofihifi.cpp : Defines the entry point for the application.

static const float pi = 3.1415926535897932384626433832795f;
static const float tau = 6.283185307179586476925286766559f;
static const float pih = 1.5707963267948966192313216916398f;
static const float piq = 0.78539816339744830961566084581988f;

#include "framework.h"
#include "windows.h"    //  rest of includes are mine
#include "lofihifi.h"

//using namespace std; // for reading .dat but std:: it locally if required
#include <iostream>
#include <fstream>
#include <tchar.h>
#include <cmath>    //"math.h"
#include <cstdio>   //"<stdio.h>"
#include <mmsystem.h>
#pragma comment(lib, "Winmm.lib")

#include <shobjidl.h>       //  for file dialog, requires win vista+

#include "fft.h"
#include "audio.h"
//#include "vectoralg.h"
//#include "graphics.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_LOFIHIFI, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LOFIHIFI));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return (int) msg.wParam;
}


ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LOFIHIFI));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_LOFIHIFI);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance;

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
        0, 0, 1024 + 2, 576 + 18, nullptr, nullptr, hInstance, nullptr);
     // CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    static BOOL bShutOff, bClosing;
    static HWAVEOUT hWaveOut;
    static PBYTE pBuffer1, pBuffer2;
    static PWAVEHDR pWaveHdr1, pWaveHdr2;
    static WAVEFORMATEX waveformat;

    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            switch (wmId) {
            case IDM_OPEN: {
                loadamt = mins = secs = wavsamplerate = loaded = 0;
                HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
                if (SUCCEEDED(hr)) {
                    IFileOpenDialog* pFileOpen;
                    hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
                        IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));
                    if (SUCCEEDED(hr)) {
                        COMDLG_FILTERSPEC ComDlgFS[1] = { {L"wav files", L"*.wav"} };
                        hr = pFileOpen->SetFileTypes(1, ComDlgFS);
                        //pFileOpen->SetTitle(L"A Single Selection Dialog");
                        hr = pFileOpen->Show(NULL);
                        if (SUCCEEDED(hr)) {
                            IShellItem* pItem;
                            hr = pFileOpen->GetResult(&pItem);
                            if (SUCCEEDED(hr)) {
                                PWSTR pszFilePath;
                                hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                                if (SUCCEEDED(hr)) {
                                    //LPWSTR wideStr = L"Some message";
                                    char nicebuffer[512];
                                    wcstombs(nicebuffer, pszFilePath, 512);
                                    FILE* fp;
                                    fp = fopen((const char*)nicebuffer, "rb");
                                    //fp = fopen((const char*)pszFilePath, "rb");
                                    if (fp) {
                                        char id[4];
                                        DWORD size;
                                        short wFormatTag, nBlockAlign;
                                        DWORD FormatChunkSize, nSamplesPerSec, nAvgBytesPerSec, DataChunkSize;
                                        BYTE bitl, bith, bit3;
                                        short nChannels, wBitsPerSample;

                                        fread(id, sizeof(char), 4, fp);	//	fread(id, sizeof(id[0]), sizeof(id)/sizeof(id[0]), fp);	//	to read entire file

                                        if (!strncmp(id, "RIFF", 4)) {
                                            fread(&size, sizeof(DWORD), 1, fp);
                                            fread(id, sizeof(char), 4, fp);
                                            if (!strncmp(id, "WAVE", 4)) {
                                                fread(id, sizeof(char), 4, fp);	//	"fmt "
                                                fread(&FormatChunkSize, sizeof(DWORD), 1, fp);
                                                fread(&wFormatTag, sizeof(short), 1, fp);	//	1 = PCM
                                                fread(&nChannels, sizeof(short), 1, fp);
                                                fread(&nSamplesPerSec, sizeof(DWORD), 1, fp);
                                                fread(&nAvgBytesPerSec, sizeof(DWORD), 1, fp);	//	SampleRate * nChannels * BitsPerSample/8
                                                fread(&nBlockAlign, sizeof(short), 1, fp);	//	nChannels * BitsPerSample/8
                                                fread(&wBitsPerSample, sizeof(short), 1, fp);	//	eg. 16, not 16 * nChannels
                                                fread(id, sizeof(char), 4, fp); //	"data"
                                                while (strncmp(id, "data", 4)) fread(id, sizeof(char), 4, fp);
                                                if (!strncmp(id, "data", 4)) {
                                                    fread(&DataChunkSize, sizeof(DWORD), 1, fp); //	how many bytes of sound data
                                                    if (DataChunkSize % 2) fread(id, sizeof(char), 1, fp);	//	padding byte
                                                    if (wBitsPerSample == 8) {
                                                        loaded = 1;
                                                        if (nChannels == 1) {
                                                            samples = DataChunkSize;
                                                            //wav0 = (float*)malloc(sizeof(float) * samples); wav1 = (float*)malloc(sizeof(float) * samples);
                                                            wav0 = (short int*)malloc(sizeof(short int) * samples); wav1 = (short int*)malloc(sizeof(short int) * samples);
                                                            for (unsigned long int i = 0; i < samples; i++) {
                                                                fread(&bitl, sizeof(BYTE), 1, fp);
                                                                //wav0[i] = (float)bitl / 128.f - 1.f;   wav1[i] = 0.f;
                                                                int16_t tbit = bitl;    tbit -= 128;    tbit *= 256;   wav0[i] = tbit;  wav1[i] = 0;
                                                                if (!(i % 4096)) {
                                                                    loadamt = (unsigned long int)((float)(i * 100) / (float)samples);
                                                                    InvalidateRgn(hWnd, 0, 0);	UpdateWindow(hWnd);
                                                                }
                                                            }
                                                        }
                                                        else if (nChannels == 2) {
                                                            samples = DataChunkSize / 2;
                                                            wav0 = (short int*)malloc(sizeof(short int) * samples); wav1 = (short int*)malloc(sizeof(short int) * samples);
                                                            for (unsigned long int i = 0; i < samples; i++) {
                                                                fread(&bitl, sizeof(BYTE), 1, fp);	fread(&bith, sizeof(BYTE), 1, fp);
                                                                int16_t tbit = bitl;    tbit -= 128;    tbit *= 256;   wav0[i] = tbit;  wav1[i] = 0;
                                                                if (!(i % 4096)) {
                                                                    loadamt = (unsigned long int)((float)(i * 100) / (float)samples);
                                                                    InvalidateRgn(hWnd, 0, 0);	UpdateWindow(hWnd);
                                                                }
                                                            }
                                                        }
                                                    }
                                                    else if (wBitsPerSample == 16) {
                                                        loaded = 1;
                                                        if (nChannels == 1) {
                                                            samples = DataChunkSize / 2;
                                                            wav0 = (short int*)malloc(sizeof(short int) * samples); wav1 = (short int*)malloc(sizeof(short int) * samples);
                                                            for (unsigned long int i = 0; i < samples; i++) {
                                                                fread(&bitl, sizeof(BYTE), 1, fp);	fread(&bith, sizeof(BYTE), 1, fp);
                                                                //bith > 127 ? wav0[i] = (float)(bith * 256 + bitl - 65536) : wav0[i] = (float)(bith * 256 + bitl);
                                                                //wav1[i] = wav0[i] /= 32768.f;   //0.f;
                                                                int tbit = bith * 256 + bitl;   if (bith > 127) tbit -= 65536;  wav0[i] = tbit; wav1[i] = 0;
                                                                if (!(i % 4096)) {
                                                                    loadamt = (unsigned long int)((float)(i * 100) / (float)samples);
                                                                    InvalidateRgn(hWnd, 0, 0);	UpdateWindow(hWnd);
                                                                }
                                                            }
                                                        }
                                                        else if (nChannels == 2) {
                                                            samples = DataChunkSize / 4;
                                                            wav0 = (short int*)malloc(sizeof(short int) * samples); wav1 = (short int*)malloc(sizeof(short int) * samples);
                                                            for (unsigned long int i = 0; i < samples; i++) {
                                                                fread(&bitl, sizeof(BYTE), 1, fp);	fread(&bith, sizeof(BYTE), 1, fp);
                                                                //bith > 127 ? wav0[i] = (float)(bith * 256 + bitl - 65536) : wav0[i] = (float)(bith * 256 + bitl);
                                                                int tbit = bith * 256 + bitl;   if (bith > 127) tbit -= 65536;  wav0[i] = tbit; wav1[i] = 0;
                                                                fread(&bitl, sizeof(BYTE), 1, fp);	fread(&bith, sizeof(BYTE), 1, fp);
                                                                //bith > 127 ? wav1[i] = (float)(bith * 256 + bitl - 65536) : wav1[i] = (float)(bith * 256 + bitl);
                                                                //wav0[i] /= 32768.f;   wav1[i] = 0.f;
                                                                if (!(i % 4096)) {
                                                                    loadamt = (unsigned long int)((float)(i * 100) / (float)samples);
                                                                    InvalidateRgn(hWnd, 0, 0);	UpdateWindow(hWnd);
                                                                }
                                                            }
                                                        }
                                                    }
                                                    else if (wBitsPerSample == 24) {
                                                        loaded = 1;
                                                        if (nChannels == 1) {
                                                            samples = DataChunkSize / 3;
                                                            wav0 = (short int*)malloc(sizeof(short int) * samples); wav1 = (short int*)malloc(sizeof(short int) * samples);
                                                            for (unsigned long int i = 0; i < samples; i++) {
                                                                fread(&bitl, sizeof(BYTE), 1, fp);	fread(&bith, sizeof(BYTE), 1, fp);	fread(&bit3, sizeof(BYTE), 1, fp);
                                                                //bit3 > 127 ? wav0[i] = (float)(bit3 * 65536 + bith * 256 + bitl - 16777216) : wav0[i] = (float)(bit3 * 65536 + bith * 256 + bitl);
                                                                int tbit = bit3 * 256 + bith;   if (bit3 > 127) tbit -= 65536;  wav0[i] = tbit; wav1[i] = 0;
                                                                //wav0[i] /= 8388608.f;   wav1[i] = 0.f;
                                                                if (!(i % 4096)) {
                                                                    loadamt = (unsigned long int)((float)(i * 100) / (float)samples);
                                                                    InvalidateRgn(hWnd, 0, 0);	UpdateWindow(hWnd);
                                                                }
                                                            }
                                                        }
                                                        else if (nChannels == 2) {
                                                            samples = DataChunkSize / 6;
                                                            wav0 = (short int*)malloc(sizeof(short int) * samples); wav1 = (short int*)malloc(sizeof(short int) * samples);
                                                            for (unsigned long int i = 0; i < samples; i++) {
                                                                fread(&bitl, sizeof(BYTE), 1, fp);	fread(&bith, sizeof(BYTE), 1, fp);	fread(&bit3, sizeof(BYTE), 1, fp);
                                                                //bit3 > 127 ? wav0[i] = (float)(bit3 * 65536 + bith * 256 + bitl - 16777216) : wav0[i] = (float)(bit3 * 65536 + bith * 256 + bitl);
                                                                int tbit = bit3 * 256 + bith;   if (bit3 > 127) tbit -= 65536;  wav0[i] = tbit; wav1[i] = 0;
                                                                fread(&bitl, sizeof(BYTE), 1, fp);	fread(&bith, sizeof(BYTE), 1, fp);	fread(&bit3, sizeof(BYTE), 1, fp);
                                                                //bit3 > 127 ? wav1[i] = (float)(bit3 * 65536 + bith * 256 + bitl - 16777216) : wav1[i] = (float)(bit3 * 65536 + bith * 256 + bitl);
                                                                //wav0[i] /= 8388608.f;   wav1[i] = 0.f;
                                                                if (!(i % 4096)) {
                                                                    loadamt = (unsigned long int)((float)(i * 100) / (float)samples);
                                                                    InvalidateRgn(hWnd, 0, 0);	UpdateWindow(hWnd);
                                                                }
                                                            }
                                                        }
                                                    }

                                                    if (samples > 0) { SetTimer(hWnd, 1, 2, NULL);  loaded = 2; }//2nd digit in timer is ms
                                                    wavsamplerate = nSamplesPerSec;

                                                    BitBlt(hdcbackground, 0, 0, dispx, dispy, NULL, 0, 0, BLACKNESS);
                                                    SelectObject(hdcbackground, hfont0); SetBkMode(hdcbackground, TRANSPARENT);
                                                    SetTextColor(hdcbackground, RGB(150, 150, 150));
                                                    //sprintf_s(nicebuffer, "%s", pszFilePath);
                                                    TextOutA(hdcbackground, 125, 4, nicebuffer, strlen(nicebuffer));
                                                    drawwav();
                                                    char str[20];
                                                    sprintf(str, "%d", wavsamplerate);
                                                    TextOutA(hdcbackground, 700, 4, str, strlen(str));
                                                    sprintf(str, "%d", samples);
                                                    TextOutA(hdcbackground, 780, 4, str, strlen(str));
                                                    sprintf(str, "%d", mins);
                                                    TextOutA(hdcbackground, 910, 4, str, strlen(str));
                                                    sprintf(str, "%d", secs);
                                                    TextOutA(hdcbackground, 950, 4, str, strlen(str));
                                                }
                                            }	//	else not WAVE
                                        }	//	else not RIFF
                                        fclose(fp);
                                    }
                                    CoTaskMemFree(pszFilePath);
                                }
                            }
                            pItem->Release();
                        }   pFileOpen->Release();
                    }
                }   CoUninitialize();
            }   break;

            case IDM_SAVE: {
                HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
                if (SUCCEEDED(hr)) {
                    IFileSaveDialog* pFileSave;
                    hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL,
                        IID_IFileSaveDialog, reinterpret_cast<void**>(&pFileSave));
                    if (SUCCEEDED(hr)) {
                        COMDLG_FILTERSPEC ComDlgFS[1] = { {L"wav files", L"*.wav"} };
                        hr = pFileSave->SetFileTypes(1, ComDlgFS);
                        hr = pFileSave->Show(NULL);
                        if (SUCCEEDED(hr)) {
                            IShellItem* pItem;
                            hr = pFileSave->GetResult(&pItem);
                            if (SUCCEEDED(hr)) {
                                PWSTR pszFilePath;
                                hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                                if (SUCCEEDED(hr)) {
                                    //MessageBoxW(NULL, pszFilePath, L"File Path", MB_OK);
                                    // try using fwrite (C) sometime 
                                    if (!wcsrchr(pszFilePath, L'.')) lstrcat(pszFilePath, L".wav");
                                    //ofstream out;	out.open(pszFilePath);
                                    std::ofstream out(pszFilePath, std::ios::binary);

                                    out.write("RIFF", 4);
                                    unsigned long int sampbytes = samples * 2;   //  2 = mono 16 bit
                                    unsigned long int fsize = sampbytes + 36;
                                    out.put(fsize & 255); out.put((fsize >> 8) & 255);
                                    out.put((fsize >> 16) & 255); out.put((fsize >> 24) & 255);
                                    out.write("WAVEfmt ", 8);
                                    out.put(16);		out.put(0);	out.put(0);	out.put(0);	//	16 = size of format chunk
                                    out.put(1);			out.put(0);							//	format 1 = pcm (what we do) 3 = float
                                    out.put(1);			out.put(0);							//	# channels
                                    //out.write("44AC", 4);									//out.put(samplerate);
                                    out.put(68);	out.put(172);out.put(0);out.put(0);		//	samplerate 44100 = 44/ac = 68/172..		
                                    out.put(136);	out.put(88);	out.put(1);	out.put(0);	//	"byte rate" obsolete? chan * bitd * sr
                                    //out.put(0);	out.put(0);	out.put(0);	out.put(0);
                                    out.put(2);	out.put(0);							//	"block align" channels * bytes per sample.. 2 for 16 bit mono
                                    out.put(16);	out.put(0);						//	bits per sample CHANGE FOR PCM
                                    out.write("data", 4);
                                    //out.put(0);	out.put(128);out.put(5);out.put(0);		//	45056 * 8 = 360448 endianated, 45056 = # of samples
                                    unsigned char wbit;
                                    wbit = (sampbytes & 255);           out.put(wbit);
                                    wbit = ((sampbytes >> 8) & 255);    out.put(wbit);
                                    wbit = ((sampbytes >> 16) & 255);   out.put(wbit);
                                    wbit = ((sampbytes >> 24) & 255);   out.put(wbit);
                                    //out.write(reinterpret_cast<const char*>(&wav0), samples * sizeof(int16_t));
                                    //out.write(reinterpret_cast<const char*>(&wav0), samples * sizeof(int16_t));
                                    for (unsigned long int i = 0; i < samples; i++) {
                                        //BYTE tbyte0 = static_cast<BYTE>(wav0[i] & 0xff);
                                        //uint16_t tbt1 = static_cast<uint16_t>(wav0[i] >> 8);
                                        //BYTE tbyte1 = static_cast<BYTE>(tbt1 & 0xff);
                                        BYTE tbyte0 = (wav0[i] & 255);
                                        short int tbt1 = wav0[i] >> 8;
                                        BYTE tbyte1 = (tbt1 & 255);
                                        out.write(reinterpret_cast<const char*>(&tbyte0), 1);
                                        out.write(reinterpret_cast<const char*>(&tbyte1), 1);
                                    }
                                    out.close();

                                    CoTaskMemFree(pszFilePath);
                                }   pItem->Release();
                            }
                        }   pFileSave->Release();
                    }   CoUninitialize();
                }
            }   break;
            case IDM_REMOVELOOPS: {
                playing = 0;    loaded = 5;
                unsigned long int tmin = (unsigned long int)(loopmins * (float)wavsamplerate);
                unsigned long int tmax = (unsigned long int)(loopmaxs * (float)wavsamplerate);
                unsigned long int endi = samples - tmax - 2;
                unsigned long int iwrite = 0;
                for (register unsigned long int i = 0; i < endi; i++) {
                    wav1[iwrite] = wav0[i];
                    unsigned long int imin = i + tmin;
                    unsigned long int imax = i + tmax;
                    bool loop = 0;
                    for (register unsigned long int j = imin; j < imax; j++) if (!((wav0[i] - wav0[j]) >> 8)) { //327=40dB, 32.7=-60dB, where the noise is, so 64, 128, 256 good
                        unsigned long int m = j - i;
                        loop = 1;
                        for (unsigned long int k = 0; k < m; k += 32) {
                            if ((wav0[i + k] - wav0[j + k]) >> 8) {
                                k = m;   loop = 0;
                            }
                        }
                        if (loop) {
                            i = j - 1;  j = imax;
                            unsigned long int lpos = 100 + (unsigned long int)(800.f * (float)i / (float)samples);
                            for (int g = 210; g < 225; g++) SetPixel(hdcbackground, lpos, g, RGB(125, 160, 250));
                        }
                    }
                    if (!loop) iwrite++;
                    if (!(i % 1024)) {
                        loadamt = (unsigned long int)(100000.f * ((float)i / (float)samples));
                        InvalidateRgn(hWnd, 0, 0);	UpdateWindow(hWnd);
                    }
                }
                //BitBlt(hdcbackground, 125, 229, 600, 21, NULL, 0, 0, BLACKNESS);
                if (iwrite < samples) {
                    for (unsigned long int i = endi; i < samples; i++) {
                        wav1[iwrite] = wav0[i]; iwrite++;
                    }
                    for (unsigned long int i = 0; i < iwrite; i++) wav0[i] = wav1[i];
                    samples = iwrite;
                }
                if (samples > 0) { SetTimer(hWnd, 1, 2, NULL);  loaded = 2; }
                BitBlt(hdcbackground, 0, 0, dispx, dispy, NULL, 0, 0, BLACKNESS);
                SelectObject(hdcbackground, hfont0); SetBkMode(hdcbackground, TRANSPARENT);
                SetTextColor(hdcbackground, RGB(150, 150, 150));
                drawwav();
                char str[20];
                sprintf(str, "%d", wavsamplerate);
                TextOutA(hdcbackground, 700, 4, str, strlen(str));
                sprintf(str, "%d", samples);
                TextOutA(hdcbackground, 780, 4, str, strlen(str));
                sprintf(str, "%d", mins);
                TextOutA(hdcbackground, 910, 4, str, strlen(str));
                sprintf(str, "%d", secs);
                TextOutA(hdcbackground, 950, 4, str, strlen(str));
                if (samples > 0) SetTimer(hWnd, 1, framerate, NULL);
            }   break;
            case IDM_NEWDATA:
                for (unsigned int i = 0; i < 232; i++) for (unsigned int j = 0; j < hidd; j++) {
                    rnd = 196314165 * rnd + 907633515;  nni[i][j] = (float)(rnd >> 16) / 65536.f; //2048.f - 16.f;
                }
                for (unsigned int i = 0; i < hidd; i++) for (unsigned int j = 0; j < 180; j++) {
                    rnd = 196314165 * rnd + 907633515;  nno[i][j] = (float)(rnd >> 16) / 65536.f;
                }
                epochs = 0;
                break;
            case IDM_OPENDATA: {
                HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
                if (SUCCEEDED(hr)) {
                    IFileOpenDialog* pFileOpen;
                    hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
                        IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));
                    if (SUCCEEDED(hr)) {
                        COMDLG_FILTERSPEC ComDlgFS[1] = { {L"data files", L"*.dat"} };
                        hr = pFileOpen->SetFileTypes(1, ComDlgFS);
                        //pFileOpen->SetTitle(L"A Single Selection Dialog");
                        hr = pFileOpen->Show(NULL);
                        if (SUCCEEDED(hr)) {
                            IShellItem* pItem;
                            hr = pFileOpen->GetResult(&pItem);
                            if (SUCCEEDED(hr)) {
                                PWSTR pszFilePath;
                                hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                                if (SUCCEEDED(hr)) {
                                    std::ifstream infile(pszFilePath, std::ios::binary);
                                    infile.read(reinterpret_cast<char*>(&nni), sizeof(nni));
                                    infile.read(reinterpret_cast<char*>(&nno), sizeof(nno));
                                    infile.read(reinterpret_cast<char*>(&epochs), sizeof(epochs));
                                    infile.close();
                                    CoTaskMemFree(pszFilePath);
                                }
                                pItem->Release();
                            }   pFileOpen->Release();
                        }
                    }   CoUninitialize();
                }
            }   break;
            case IDM_SAVEDATA: {
                HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
                if (SUCCEEDED(hr)) {
                    IFileSaveDialog* pFileSave;
                    hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL,
                        IID_IFileSaveDialog, reinterpret_cast<void**>(&pFileSave));
                    if (SUCCEEDED(hr)) {
                        COMDLG_FILTERSPEC ComDlgFS[1] = { {L"data files", L"*.dat"} };
                        hr = pFileSave->SetFileTypes(1, ComDlgFS);
                        hr = pFileSave->Show(NULL);
                        if (SUCCEEDED(hr)) {
                            IShellItem* pItem;
                            hr = pFileSave->GetResult(&pItem);
                            if (SUCCEEDED(hr)) {
                                PWSTR pszFilePath;
                                hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                                if (SUCCEEDED(hr)) {
                                    // try using fwrite (C) sometime 
                                    if (!wcsrchr(pszFilePath, L'.')) lstrcat(pszFilePath, L".dat");
                                    std::ofstream out(pszFilePath, std::ios::binary);
                                    out.write(reinterpret_cast<char*>(&nni), sizeof(nni)); // Write the array to the file
                                    out.write(reinterpret_cast<char*>(&nno), sizeof(nno)); // Write the array to the file
                                    out.write(reinterpret_cast<char*>(&epochs), sizeof(epochs));
                                    out.close();
                                    CoTaskMemFree(pszFilePath);
                                }   pItem->Release();
                            }
                        }   pFileSave->Release();
                    }   CoUninitialize();
                }
            }   break;
            case IDM_TRAIN: {
                loaded = 6;
                float nnh[420];	//	hidden layer
                float nnhe[420];//	hidden error
                float nnoe[180];//	output error
                unsigned long int teststep = (unsigned long int)((float)(samples - 520) / (float)numtests);
                unsigned long int testpos = 0;
                if (samples < 2048) break;
                rnd = 196314165 * rnd + 907633515;  testpos += ((rnd >> 20) & 255);
                while (tests < 100000) {
                    unsigned long int dBcheck = 0;
                    for (unsigned long int i = 0; i < 512; i++) dBcheck += abs(wav0[testpos + i]);
                    dBcheck >>= 8;
                    if (dBcheck >> 7) {
                        for (unsigned long int i = 0; i < 512; i++) wff0[i] = wff1[i] = (float)wav0[testpos + i] / 32768.f;
                        fft(9, wff0, wff1);
                        for (unsigned int i = 0; i < 207; i++) {	//	cartesian to polar .. 206 is the highest bin used, 256 = nyquist
                            wmag[i] = sqrt(wff0[i] * wff0[i] + wff1[i] * wff1[i]);
                            if (wff0[i] == 0) wpha[i] = 0.f;
                            else {
                                wpha[i] = atan(wff1[i] / wff0[i]);
                                if (wff0[i] < 0) wff1[i] < 0 ? wpha[i] -= pi : wpha[i] += pi;
                            }
                            wmag[i] = log10(wmag[i]); magi[i] = (int)(wmag[i] * 32.f);
                            wmag[i] *= 0.3125f;
                            wpha[i] *= 0.2f;
                        }
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
                        for (unsigned int i = 0; i < 180; i++) { //  size of output      hidden to output
                            float osum = 0.f;
                            for (unsigned int j = 0; j < hidd; j++) osum += nnh[j] * nno[j][i];
                            nnr[i] = std::tanh(osum);
                        }
                        Lreg = 0.f;  //  regularization .. method of avoiding local minima L2 = x*x, L1 = abs
                        for (unsigned int i = 0; i < 90; i++) {  //  error = prediction - target
                            int j = i + 90;
                            //nnoe[i] = wmag[i + 117] - nnr[i];
                            //nnoe[j] = wpha[i + 117] - nnr[j];
                            float tsu;
                            tsu = wmag[i + 117] - nnr[i];   Lreg += fabs(tsu); nnoe[i] = tsu;
                            tsu = wpha[i + 117] - nnr[j];   Lreg += fabs(tsu); nnoe[j] = tsu;
                        }
                        Lreg *= 1e-13f;    //  /512, * regularisation strength, typically .0001 to .00001f .. had 1e-13 for L2
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
                        for (unsigned int i = 0; i < hidd; i++) { //  size of hidden      hidden to input
                            for (int j = 0; j < 116; j++) {
                                nni[j][i] += learn * nnhe[i] * wmag[j + 1];
                                nni[j + 116][i] += learn * nnhe[i] * wpha[j + 1];
                            }
                        }
                        for (unsigned int i = 0; i < 180; i++) { //  size of output      output to hidden.. step 4!!
                            for (unsigned int j = 0; j < hidd; j++) {
                                nno[j][i] += learn * nnoe[i] * nnh[j] + Lreg;
                            }
                        }
                        if (!(tests & 31)) {
                            epos = 100 + (unsigned long int)(800.f * (float)testpos / (float)samples);
                            InvalidateRgn(hWnd, 0, 0);	UpdateWindow(hWnd);
                        }
                        tests++;
                    }
                    testpos += teststep;    if (testpos > samples - 520) testpos -= (samples - 520);
                }
                if (tests == 100000) { epochs++;    tests = 0; }
            }   break;

            case IDM_APPLYHIFI: {               
                playing = 0;    loaded = 7;
                float nnh[420];	//	hidden layer
                unsigned long int wavpos = 0;
                while (wavpos < samples) {
                    if (wavpos < samples - 512) for (unsigned long int i = 0; i < 512; i++) wff0[i] = wff1[i] = (float)wav0[wavpos + i] / 32768.f;
                    else {
                        unsigned long int j = samples - wavpos;
                        for (unsigned long int i = 0; i < j; i++) wff0[i] = wff1[i] = (float)wav0[wavpos + i] / 32768.f;
                        for (unsigned long int i = j; i < 512; i++) wff0[i] = wff1[i] = 0.f;
                    }
                    fft(9, wff0, wff1);
                    for (int i = 1; i < 117; i++) {	//	cartesian to polar .. 206 is the highest bin used, 256 = nyquist
                        wmag[i] = sqrt(wff0[i] * wff0[i] + wff1[i] * wff1[i]);
                        if (wff0[i] == 0) wpha[i] = 0.f;
                        else {
                            wpha[i] = atan(wff1[i] / wff0[i]);
                            if (wff0[i] < 0) wff1[i] < 0 ? wpha[i] -= pi : wpha[i] += pi;
                        }
                        wmag[i] = log10(wmag[i]); magi[i] = (int)(wmag[i] * 32.f);
                    }
                    //feedforward neural network or multilayer perceptron.
                    for (unsigned int i = 0; i < hidd; i++) { //  size of hidden..    input to hidden
                        float hsum = 0.f;
                        for (int j = 1; j < 117; j++) { //  input is 232, 116 mag bins 116 phase bins
                            hsum += wmag[j] * .3125f * nni[j - 1][i];
                            hsum += wpha[j] * .5f * nni[j + 115][i];
                        }
                        nnh[i] = std::tanh(hsum);
                    }
                   for (unsigned int i = 0; i < 90; i++) { //  size of output/2 (split) hidden to output
                        unsigned int k = i + 90;
                        unsigned int m = i + 117;
                        float osum = 0.f;
                        for (unsigned int j = 0; j < hidd; j++) osum += nnh[j] * nno[j][i];
                        nnr[i] = std::tanh(osum);
                        wmag[m] = nnr[i] * 3.2f;
                        osum = 0.f;
                        for (unsigned int j = 0; j < hidd; j++) osum += nnh[j] * nno[j][k];
                        nnr[k] = std::tanh(osum);
                        wpha[m] = nnr[k] * 5.f;
                    }
                    for (unsigned int i = 117; i < 207; i++) {
                        float tm = pow(10.f, wmag[i]);  //  second term is inverted as a condition of IFFT.. FFT>(rx,ix), (rx,-ix)>IFFT
                        wff0[i] = tm * cos(wpha[i]);    wff1[i] = -tm * sin(wpha[i]);   //	polar to cartesian
                    }
                    //  note zeroing one side of fourier frame halves amplitude (which is ok for this app since we're adding)
                    for (unsigned int i = 207; i < 512; i++) wff0[i] = wff1[i] = 0.f;
                    for (unsigned long int i = 0; i < 117; i++) wff1[i] = -wff1[i];
                    fft(9, wff0, wff1);

                    if (wavpos) {
                        if (wavpos < samples - 512) {
                            for (unsigned long int i = 0; i < 256; i++) {
                                long int t = wff0[i] * hann[i]; t += wav1[wavpos + i];
                                t = max(min(t, 32767), -32767);
                                wav1[wavpos + i] = (short int)t;
                            }
                            for (unsigned long int i = 256; i < 512; i++) {
                                long int t = wff0[i] * hann[i]; t = max(min(t, 32767), -32767);
                                wav1[wavpos + i] = (short int)t;
                            }
                        }
                        else {
                            for (unsigned long int i = wavpos + 256; i < samples; i++) wav1[i] = 0;
                            for (unsigned long int i = 0; i < 512; i++) {
                                unsigned long int j = i + wavpos;
                                if (j < samples) {
                                    long int t = wff0[i] * hann[i]; t += wav1[j];
                                    t = max(min(t, 32767), -32767);
                                    wav1[j] = (short int)t;
                                }
                                else i = 512;
                            }
                        }
                    }
                    else {  //  first frame
                        for (unsigned int i = 0; i < 256; i++) {
                            long int t = wff0[i] * 64.f; t = max(min(t, 32767), -32767);
                            wav1[i] = (short int)t;
                        } //  /512 ifft and *32768 float to 16 bit int = 64
                        for (unsigned int i = 256; i < 512; i++) {
                            long int t = wff0[i] * hann[i]; t = max(min(t, 32767), -32767);
                            wav1[i] = (short int)t;
                        }
                    }

                    if (!((wavpos >> 9) & 31)) {
                        epos = 100 + (unsigned long int)(800.f * (float)wavpos / (float)samples);
                        InvalidateRgn(hWnd, 0, 0);	UpdateWindow(hWnd);
                    }
                    wavpos += 256;
                }
                for (unsigned long int i = 0; i < samples; i++) wav0[i] = wav1[i];
                SetTimer(hWnd, 1, 1, NULL);  loaded = 2;
                BitBlt(hdcbackground, 0, 0, dispx, dispy, NULL, 0, 0, BLACKNESS);
                SelectObject(hdcbackground, hfont0); SetBkMode(hdcbackground, TRANSPARENT);
                SetTextColor(hdcbackground, RGB(150, 150, 150));
                drawwav();
                char str[20];
                sprintf(str, "%d", wavsamplerate);
                TextOutA(hdcbackground, 700, 4, str, strlen(str));
                sprintf(str, "%d", samples);
                TextOutA(hdcbackground, 780, 4, str, strlen(str));
                sprintf(str, "%d", mins);
                TextOutA(hdcbackground, 910, 4, str, strlen(str));
                sprintf(str, "%d", secs);
                TextOutA(hdcbackground, 950, 4, str, strlen(str));
                SetTimer(hWnd, 1, framerate, NULL);
            }   break;

            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_CREATE:
    {

        HDC hdc;
        BITMAPINFO bitmapinfo;
        hdc = CreateCompatibleDC(NULL);
        bitmapinfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bitmapinfo.bmiHeader.biWidth = dispx;
        bitmapinfo.bmiHeader.biHeight = -dispy;
        bitmapinfo.bmiHeader.biPlanes = 1;
        bitmapinfo.bmiHeader.biBitCount = 32;
        bitmapinfo.bmiHeader.biCompression = BI_RGB;
        bitmapinfo.bmiHeader.biSizeImage = 0;
        bitmapinfo.bmiHeader.biClrUsed = 256;
        bitmapinfo.bmiHeader.biClrImportant = 256;
        framebmp = CreateDIBSection(hdc, &bitmapinfo, DIB_RGB_COLORS, (void**)&framebuf, 0, 0);
        hdcframe = CreateCompatibleDC(NULL);
        oldframe = (HBITMAP)SelectObject(hdcframe, framebmp);
        backgroundbmp = CreateDIBSection(hdc, &bitmapinfo, DIB_RGB_COLORS, (void**)&backgroundbuf, 0, 0);
        hdcbackground = CreateCompatibleDC(NULL);
        oldbackground = (HBITMAP)SelectObject(hdcbackground, backgroundbmp);


        //DeleteDC(hdc);

        if (hWaveOut == NULL) {
            pWaveHdr1 = (PWAVEHDR)malloc(sizeof(WAVEHDR));
            pWaveHdr2 = (PWAVEHDR)malloc(sizeof(WAVEHDR));
            pBuffer1 = (PBYTE)malloc(OUT_BUFFER_SIZE);
            pBuffer2 = (PBYTE)malloc(OUT_BUFFER_SIZE);
            if (!pWaveHdr1 || !pWaveHdr2 || !pBuffer1 || !pBuffer2) {
                if (!pWaveHdr1) free(pWaveHdr1);
                if (!pWaveHdr2) free(pWaveHdr2);
                if (!pBuffer1)  free(pBuffer1);
                if (!pBuffer2)  free(pBuffer2);
                fprintf(stderr, "Error allocating memory!\n");
                return TRUE;
            }
            bShutOff = FALSE;

            waveformat.nSamplesPerSec = 44100;
            waveformat.wBitsPerSample = 16;
            waveformat.nChannels = 2;
            waveformat.cbSize = 0;
            waveformat.wFormatTag = WAVE_FORMAT_PCM;
            waveformat.nBlockAlign = (waveformat.wBitsPerSample >> 3) * waveformat.nChannels;
            waveformat.nAvgBytesPerSec = waveformat.nBlockAlign * waveformat.nSamplesPerSec;
            if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &waveformat, (DWORD)hWnd, 0, CALLBACK_WINDOW) != MMSYSERR_NOERROR) {
                free(pWaveHdr1);	free(pWaveHdr2);	free(pBuffer1);	free(pBuffer2);
                hWaveOut = NULL;
                fprintf(stderr, "unable to open WAVE_MAPPER device\n");
                ExitProcess(1);
            }
            pWaveHdr1->lpData = (LPSTR)pBuffer1;
            pWaveHdr1->dwBufferLength = OUT_BUFFER_SIZE;
            pWaveHdr1->dwBytesRecorded = 0;
            pWaveHdr1->dwUser = 0;
            pWaveHdr1->dwFlags = 0;
            pWaveHdr1->dwLoops = 1;
            pWaveHdr1->lpNext = NULL;
            pWaveHdr1->reserved = 0;
            waveOutPrepareHeader(hWaveOut, pWaveHdr1, sizeof(WAVEHDR));

            pWaveHdr2->lpData = (LPSTR)pBuffer2;
            pWaveHdr2->dwBufferLength = OUT_BUFFER_SIZE;
            pWaveHdr2->dwBytesRecorded = 0;
            pWaveHdr2->dwUser = 0;
            pWaveHdr2->dwFlags = 0;
            pWaveHdr2->dwLoops = 1;
            pWaveHdr2->lpNext = NULL;
            pWaveHdr2->reserved = 0;
            waveOutPrepareHeader(hWaveOut, pWaveHdr2, sizeof(WAVEHDR));
        }

        hfont0 = CreateFont(15, 0, 0, 0, 0, 0, 0, 0, 0, OUT_DEVICE_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 0, 0);
        //hfont0 = CreateFont(18, 0, 0, 0, 0, 0, 0, 0, 0, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, 0, 0);



        //ifstream infile("data.dat", ios::binary); // Open the binary file for reading
        //infile.read(reinterpret_cast<char*>(&buf), sizeof(buf)); // Read the data from the file
        //infile.close(); // Close the file

        //ofstream outfile("data.dat", ios::binary); // Create a binary file called "data.dat"
        //outfile.write(reinterpret_cast<char*>(&buf), sizeof(buf)); // Write the array to the file
        //outfile.close(); // Close the file

        for (int i = 1; i <= 19; i++) {
            int le = powtwo[i];	int le2 = le >> 1;
            float angle = pi / (float)le2;	zr[i] = cos(angle);	zi[i] = -sin(angle);
        }
        //	reference bin to pixel values
        //	we are using 16384 bins, 512 pixels
        /*int logbin[8192];	logbin[0] = 0;
        for (int i = 1; i < 8192; i++) {
            //	double hz = (double)i * 43.06640625;	//	44100/1024 ------ change this value if window length changed
            double hz = (double)i * 2.691650390625;	//	44100/16384
            hz = log(hz) / r10;
            hz *= 117.8797686062508286505044320866;	//	512/4.3434etc .. pixels div ?? dunno where i got 4.34
            logbin[i] = (int)hz;
        }*/
        logbin[0] = 0;  double r10 = log(10);
        for (int i = 1; i < 256; i++) {
            double hz = (double)i * 86.1328125;	//	44100/512 bins
            hz = log(hz) / r10;
            hz *= 2.0 * 117.8797686062508286505044320866;	//	512/4.3434etc .. pixels div ?? dunno where i got 4.34 (512 is pixels here not fft)
            logbin[i] = (int)hz;
        }
        float tix = tau / 512.f;    //  window function for fourier audio resynthesis
        //for (unsigned int i = 0; i < 512; i++) { hann[i] = .5f - cos(tix * (float)i) * .5f;	hann[i] /= 512.f; }
        //coeffs 7938/18608=.42659, 9240/18608=.49656, a2 = 1430/18608=.076849 initial blackman .42,.5,.08
        for (unsigned int i = 0; i < 512; i++) hann[i] = (.426591f - .49656f * cos(tix * (float)i) + .076849f * cos(tix * (float)(i + i))) * 64.f;
        //  scaling by 64 handles division by 512 after the IFFT and the conversion from float [-1,1] to [-32768,32767]

        SetTimer(hWnd, 1, framerate, NULL);
    }
    break;
    case WM_KEYDOWN:
        if ((HIWORD(lParam) & KF_REPEAT) < 1) {
            switch (LOWORD(wParam)) {
            case 'o': {
            }   break;
            case VK_ESCAPE:	PostMessage(hWnd, WM_CLOSE, 0, 0);	break;
            case VK_SPACE:  playing = !playing; break;
            }
        }
        break;
    case WM_LBUTTONDOWN: {
        POINT mouloc;	GetCursorPos(&mouloc);	ScreenToClient(hWnd, &mouloc);
        if (loaded > 2 && mouloc.y > 20 && mouloc.y < 250) {
            if (mouloc.x > 99) {
                if (mouloc.x < 901) { wavposition = (mouloc.x - 100);  wavposition *= wgrlen;  
                    posmins = wavposition / (wavsamplerate * 60);
                    possecs = (wavposition - posmins * wavsamplerate * 60) / wavsamplerate;
                    loaded = 3; }
                else if (mouloc.y < 125) {
                    mouloc.x < 950 ? loopmins = max(loopmins - 0.1f, 0.8f) : loopmins = min(loopmins + 0.1f, min(4.8f, loopmaxs));
                }
                else {
                    mouloc.x < 950 ? loopmaxs = max(loopmaxs - 0.1f, max(loopmins, 1.f)) : loopmaxs = min(loopmaxs + 0.1f, 5.f);
                }
            }
            else if (mouloc.x > 70) { wavposition = 0;  loaded = 3; }
        }
    }   break;
    case WM_PAINT:
        {
        PAINTSTRUCT ps;

        BitBlt(hdcframe, 0, 0, dispx, dispy, NULL, 0, 0, BLACKNESS);
        BitBlt(hdcframe, 0, 0, dispx, dispy, hdcbackground, 0, 0, SRCCOPY);
        SetBkMode(hdcframe, TRANSPARENT);

        //POINT mouloc;	GetCursorPos(&mouloc);	ScreenToClient(hWnd, &mouloc);
        SelectObject(hdcframe, hfont0);	SetTextColor(hdcframe, RGB(175, 150, 125));
        char str[20];
        sprintf(str, "%d", posmins);
        TextOutA(hdcframe, 910, 24, str, strlen(str));
        sprintf(str, "%d", possecs);
        TextOutA(hdcframe, 950, 24, str, strlen(str));
        TextOutA(hdcframe, 942, 24, ":", 1);
        TextOutA(hdcframe, 972, 24, "s", 1);

        sprintf(str, "%.1f", loopmins);  TextOutA(hdcframe, 940, 70, str, strlen(str));
        sprintf(str, "%.1f", loopmaxs);  TextOutA(hdcframe, 940, 168, str, strlen(str));

        SetTextColor(hdcframe, RGB(150,150,150));
       // TextOutA(hdcframe, 20, 16, "Open wav", 8);

        switch (loaded) {
            case 0: 
                TextOutA(hdcframe, 125, 129, "for 44100 samplerate files with spectra to 10kHz (riffusion)", 60);
                TextOutA(hdcframe, 125, 149, "left channel of stereo files used for processing", 48);
                TextOutA(hdcframe, 125, 169, "note: loop removal function is time intensive", 45);
                TextOutA(hdcframe, 125, 189, "enhanced file is half amplitude to accomodate peaks", 51);
                TextOutA(hdcframe, 125, 229, "open wav to begin", 17);  break;
            case 1: {
                TextOutA(hdcframe, 125, 229, "loading wav please wait..", 26);
                TextOutA(hdcframe, 300, 229, "%", 1);
                sprintf(str, "%d", loadamt);
                TextOutA(hdcframe, 280, 229, str, strlen(str));
            }   break;
            case 2: {
                int wpeak = 0;
                unsigned long int tread = wgrprog * wgrlen;
                int tstep = wgrlen / 256; if (tstep < 1) tstep = 1;
                for (unsigned long int i = tread; i < tread + wgrlen; i+=tstep) if (wav0[i] > wpeak) wpeak = wav0[i];
                //tread = wpeak / 27; //(int)(wpeak * 100.f);
            //    wpeak /= 328;
                wpeak >>= 9;
                for (int i = 125 + wpeak; i > 124 - wpeak; i--) SetPixel(hdcbackground, 100 + wgrprog, i, RGB(125, 200, i));
                TextOutA(hdcframe, 125, 229, "drawing waveform please wait..", 30);
                if (wgrprog < 798) wgrprog++;
                else { SetTimer(hWnd, 1, framerate, NULL); loaded = 3; }
            }   break;
            case 3: {   //  perform fourier analysis at position
                wpos = 100 + (unsigned long int)(800.f * (float)wavposition / (float)samples);
                for (int i = 55; i < 210; i++) SetPixel(hdcframe, wpos, i, RGB(200, 160, 125));
                unsigned long int fouposition = min(wavposition, samples - 520);
                for (int i = 0; i < 512; i++) wff0[i] = wff1[i] = (float)wav0[fouposition + i] / 32768.f;
                fft(9, wff0, wff1);
                for (int i = 0; i < 256; i++) {	//	cartesian to polar
                    wmag[i] = sqrt(wff0[i] * wff0[i] + wff1[i] * wff1[i]);
                    if (wff0[i] == 0) wpha[i] = 0;
                    else {wpha[i] = atan(wff1[i] / wff0[i]);
                        if (wff0[i] < 0) wff1[i] < 0 ? wpha[i] -= pi : wpha[i] += pi;
                    } 
                    wmag[i] = log10(wmag[i]); magi[i] = (int)(wmag[i] * 32.f);
                }
                loaded = 4;
            }   break;
            case 4: {   //  sit
                for (int i = 55; i < 210; i++) SetPixel(hdcframe, wpos, i, RGB(200, 160, 125));
                //sprintf(str, "%d", wavposition);  TextOutA(hdcframe, 950, 400, str, strlen(str));
                for (int i = 1; i < 233; i++) {
                    SetPixel(hdcframe, logbin[i] - 206, 360 - magi[i], RGB(125, 125, 250));
                    SetPixel(hdcframe, logbin[i] - 206, 493 - (int)(wpha[i] * 10.f), RGB(125, 125, 250));
                }
                if (playing) {
                    posmins = wavposition / (wavsamplerate * 60);
                    possecs = (wavposition - posmins * wavsamplerate * 60) / wavsamplerate;
                    loaded = 3;
                }
                //else {wavposition++; loaded = 3;}
            }   break;
            case 5: {   //  finding loops
                for (int ii = 55; ii < 210; ii++) SetPixel(hdcframe, wpos, ii, RGB(200, 160, 125));
                for (int ii = 1; ii < 233; ii++) {
                    SetPixel(hdcframe, logbin[ii] - 206, 360 - magi[ii], RGB(125, 125, 250));  //360
                    SetPixel(hdcframe, logbin[ii] - 206, 493 - (int)(wpha[ii] * 10.f), RGB(125, 125, 250));
                }
                TextOutA(hdcframe, 125, 229, "finding loops..", 15);
                TextOutA(hdcframe, 330, 229, "%", 1);   TextOutA(hdcframe, 295, 229, ".", 1);
                sprintf(str, "%d", (loadamt / 1000));    TextOutA(hdcframe, 275, 229, str, strlen(str));
                sprintf(str, "%d", (loadamt % 1000));    TextOutA(hdcframe, 305, 229, str, strlen(str));
            }   break;

            case 6: {   //  training
                //TextOutA(hdcframe, 920, 269, "epoch: ", 1);
                sprintf(str, "epochs:   %i", epochs);    TextOutA(hdcframe, 880, 260, str, strlen(str));
                sprintf(str, "tests:   %i", tests);    TextOutA(hdcframe, 880, 280, str, strlen(str));
                float readout = Lreg * 1e10f;
                sprintf(str, "Lreg:   %.5f", readout);    TextOutA(hdcframe, 880, 300, str, strlen(str));

                for (int i = 55; i < 210; i++) SetPixel(hdcframe, epos, i, RGB(250, 100, 0));
                for (int i = 1; i < 206; i++) {//233 = 20kHz
                    SetPixel(hdcframe, logbin[i] - 206, 360 - magi[i], RGB(0, 100, 250));
                    SetPixel(hdcframe, logbin[i] - 206, 493 - (int)(wpha[i] * 50.f), RGB(0, 100, 250));
                }
                for (int i = 0; i < 90; i++) {
                    int j = i + 117;
                    SetPixel(hdcframe, logbin[j] - 206, 360 - (int)(nnr[i] * 100.f), RGB(250, 100, 0));  //  *32
                    SetPixel(hdcframe, logbin[j] - 206, 493 - (int)(nnr[i + 90] * 50.f), RGB(250, 100, 0)); //  *10
                }
            }   break;


            case 7: {   //  applying hifi
                for (int i = 55; i < 210; i++) SetPixel(hdcframe, epos, i, RGB(250, 100, 0));
                TextOutA(hdcframe, 125, 229, "applying data model to waveform please wait..", 45);
                for (int i = 1; i < 116; i++) {//233 = 20kHz
                    SetPixel(hdcframe, logbin[i] - 206, 360 - magi[i], RGB(0, 100, 250));
                    SetPixel(hdcframe, logbin[i] - 206, 493 - (int)(wpha[i] * 10.f), RGB(0, 100, 250));
                }
                for (int i = 0; i < 90; i++) {
                    int j = i + 117;
                    SetPixel(hdcframe, logbin[j] - 206, 360 - (int)(nnr[i] * 100.f), RGB(250, 60, 60));  //  *32
                    SetPixel(hdcframe, logbin[j] - 206, 493 - (int)(nnr[i + 90] * 50.f), RGB(250, 60, 60)); //  *10
                }
            }   break;



            default:    break;
        }

        hdc = BeginPaint(hWnd, &ps);
        BitBlt(hdc, 0, 0, dispx, dispy, hdcframe, 0, 0, SRCCOPY);
        EndPaint(hWnd, &ps);
        DeleteDC(hdc);
        }
        break;
    case WM_TIMER: {
        InvalidateRgn(hWnd, 0, 0);	UpdateWindow(hWnd);
    }
                 break;
    case WM_DESTROY:
        bShutOff = TRUE;
        PostQuitMessage(0);
        break;
    case MM_WOM_OPEN:
        FillAudioBuffer(pBuffer1);	waveOutWrite(hWaveOut, pWaveHdr1, sizeof(WAVEHDR));
        FillAudioBuffer(pBuffer2);	waveOutWrite(hWaveOut, pWaveHdr2, sizeof(WAVEHDR));
        return TRUE;
    case MM_WOM_DONE:
        if (bShutOff) { waveOutClose(hWaveOut);	return TRUE; }
        FillAudioBuffer((PBYTE)((PWAVEHDR)lParam)->lpData);
        waveOutWrite(hWaveOut, (PWAVEHDR)lParam, sizeof(WAVEHDR));
        return TRUE;
    case MM_WOM_CLOSE:
        waveOutUnprepareHeader(hWaveOut, pWaveHdr1, sizeof(WAVEHDR));
        waveOutUnprepareHeader(hWaveOut, pWaveHdr2, sizeof(WAVEHDR));
        free(pWaveHdr1);	free(pWaveHdr2);	free(pBuffer1);	free(pBuffer2);
        hWaveOut = NULL;
        return TRUE;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
