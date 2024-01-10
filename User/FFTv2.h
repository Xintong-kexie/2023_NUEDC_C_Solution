#define __FPU_PRESENT 1U    // 寮�鍚疐PU

#ifndef __FFTV2_H
#define __FFTV2_H

#include "stdint.h"
#include "device.h"
#include "driverlib.h"
#define NPT_max  4096           //鏈�澶FT鐐规暟锛岀悊璁轰笂鑳藉紑寰堝ぇ锛屼絾鏄鑰冭檻鍗曠墖鏈篟AM鐨勫ぇ灏�
#define pi 3.1415926

typedef struct Complex
{
	float32_t real; // 瀹為儴
	float32_t imag; // 铏氶儴
}Complex; // 澶嶆暟

typedef struct FFTresult
{
	Complex result[NPT_max];    //FFT鐨勭粨鏋滐紝鏄鏁板舰寮�
	//float32_t	Amp[NPT_max];       //骞呭��
	//float32_t	phase[NPT_max];     //鐩镐綅
    float32_t   dc;                 //鐩存祦鍒嗛噺
}FFTresult; //瀛樻斁FFT杩愮畻缁撴灉鐨勭粨鏋勪綋


static void FFT(int length, FFTresult* ch);


// 浠ヤ笅鍑芥暟鏄敤鎴蜂娇鐢ㄧ殑FFT鍑芥暟

//FFT鍒濆鍖栵紝涓昏鏄疐FT鍓嶆墦琛╯in锛屽疄鐜板姞閫�
void FFT_Init(void);

void FFT_u32(uint32_t* input, int length, FFTresult* ch);
void FFT_f32(float32_t* input, int length, FFTresult* ch);
void FFT_int32(int* input, int length, FFTresult* ch);
void FFT_int16(int16_t* input, int length, FFTresult* ch);
void FFT_win_u32(uint32_t* input, int length, float32_t* win, FFTresult* ch);
void FFT_win_f32(float32_t* input, int length, float32_t* win, FFTresult* ch);
void FFT_win_int32(int* input, int length, float32_t* win, FFTresult* ch);
void FFT_win_int16(int16_t* input, int length, float32_t* win, FFTresult* ch);

void AmpRecovery(FFTresult* ch, int length, float32_t k);


//浠ヤ笅涓哄姞绐楀嚱鏁�

static int Factorial(int num);//闃朵箻
static float32_t Besseli(float32_t x);//0闃惰礉濉炲皵
float32_t Kaiser(int n, float32_t beta, float32_t* win);//Kaiser绐�
float32_t FlattopWin(int n, float32_t* win);//Flattop绐�
float32_t Hanning(int n, float32_t* win);//Hanning绐�
float32_t Hamming(int n, float32_t* win);//Hamming绐�
float32_t rectangle(int n, float32_t* win);//鐭╁舰绐�

//浠ヤ笅涓哄疄鐜癋FT绠楁硶鐢ㄥ埌鐨勫嚱鏁�

static int BitReverse(int input, int N);        // 姣旂壒鍙嶈浆
static void DFT(Complex* p, int len);           // DFT锛岀敤浜庨�掑綊
Complex Complex_Multi(Complex c1, Complex c2);  // 澶嶆暟涔樻硶锛岃繑鍥炲�肩瓑浜巆1 * c2
Complex Complex_Add(Complex c1, Complex c2);    // 澶嶆暟鍔犳硶锛岃繑鍥炲�肩瓑浜巆1 + c2
Complex Complex_Minus(Complex c1, Complex c2);  // 澶嶆暟鍑忔硶锛岃繑鍥炲�肩瓑浜巆1 - c2
float32_t Complex_Modular(Complex c);               // 澶嶆暟姹傛ā
float32_t Complex_arg(Complex c);                   // 澶嶆暟姹傝緪瑙�

//sin鍜宑os鏌ヨ〃

static float32_t Mysin(int len, int i);
static float32_t Mycos(int len, int i);

#endif

