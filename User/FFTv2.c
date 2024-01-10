#include "FFTv2.h"
#include "math.h"
#include "stdio.h"

float32_t sin_table[NPT_max / 4];
int use_table = 0;


void FFT_Init(void)
{
	use_table = 1;
	int32_t i;
	for (i = 0; i < NPT_max / 4; i++)
	{
		sin_table[i] = sin(2 * pi * i / NPT_max);
	}
}

/*
 *FFT鍑芥暟涓讳綋
 *length: FFT鐐规暟锛屽繀椤讳负2鐨勬暣鏁版鏂�
 *ch: FFTresult瀛樻斁缁撴灉鐨勭粨鏋勪綋
 */
static void FFT(int length, FFTresult* ch)
{
    int32_t i;
	DFT(&ch->result[0], length);
//	for (i = 0; i < length; i++)
//	{
//		ch->Amp[i] = Complex_Modular(ch->result[i]);
//		ch->Amp[i] = ch->Amp[i] / length * 2;
//		if (i == 0)
//		{
//			ch->Amp[i] /= 2;
//		}
//		//ch->phase[i] = Complex_arg(ch->result[i]);
//	}
}

/*
 *杈撳叆鏁版嵁鐨勭被鍨嬩负uint32_t鐨凢FT
 *input: 杈撳叆鏁版嵁
 *length: FFT鐐规暟锛屽繀椤讳负2鐨勬暣鏁版鏂�
 *ch: FFTresult瀛樻斁缁撴灉鐨勭粨鏋勪綋
 */
void FFT_u32(uint32_t* input, int length, FFTresult* ch)
{
    int32_t i;
    //閫氳繃姣旂壒鍙嶈浆娉曟潵瀵规暟缁勯噸鏂版帓搴�
	for (i = 0; i < length; i++)
	{
		int temp = BitReverse(i, length);
		ch->result[i].real = (float32_t)input[temp];
		ch->result[i].imag = 0;
	}
	FFT(length, ch);
	//ch->dc = ch->Amp[0];
}

/*
 *杈撳叆鏁版嵁鐨勭被鍨嬩负float鐨凢FT
 *input: 杈撳叆鏁版嵁
 *length: FFT鐐规暟锛屽繀椤讳负2鐨勬暣鏁版鏂�
 *ch: FFTresult瀛樻斁缁撴灉鐨勭粨鏋勪綋
 */
void FFT_f32(float32_t* input, int length, FFTresult* ch)
{
    int32_t i;
	for (i = 0; i < length; i++)
	{
		int temp = BitReverse(i, length);
		ch->result[i].real = input[temp];
		ch->result[i].imag = 0;
	}
	FFT(length, ch);
	//ch->dc = ch->Amp[0];
}

/*
 *杈撳叆鏁版嵁鐨勭被鍨嬩负int鐨凢FT
 *input: 杈撳叆鏁版嵁
 *length: FFT鐐规暟锛屽繀椤讳负2鐨勬暣鏁版鏂�
 *ch: FFTresult瀛樻斁缁撴灉鐨勭粨鏋勪綋
 */
void FFT_int32(int* input, int length, FFTresult* ch)
{
    int32_t i;
	for (i = 0; i < length; i++)
	{
		int temp = BitReverse(i, length);
		ch->result[i].real = (float32_t)input[temp];
		ch->result[i].imag = 0;
	}
	FFT(length, ch);
	//ch->dc = ch->Amp[0];
}

/*
 *杈撳叆鏁版嵁鐨勭被鍨嬩负int16_t鐨凢FT
 *input: 杈撳叆鏁版嵁
 *length: FFT鐐规暟锛屽繀椤讳负2鐨勬暣鏁版鏂�
 *ch: FFTresult瀛樻斁缁撴灉鐨勭粨鏋勪綋
 */
void FFT_int16(int16_t* input, int length, FFTresult* ch)
{
    int32_t i;
	for (i = 0; i < length; i++)
	{
		int temp = BitReverse(i, length);
		ch->result[i].real = (float32_t)input[temp] * 5.0f / 32767;
		ch->result[i].imag = 0;
	}
	FFT(length, ch);
	//ch->dc = ch->Amp[0];
}

/*
 *杈撳叆鏁版嵁鐨勭被鍨嬩负uint32_t鐨勫姞绐桭FT
 *input: 杈撳叆鏁版嵁
 *length: FFT鐐规暟锛屽繀椤讳负2鐨勬暣鏁版鏂�
 *win: 绐楀嚱鏁�
 *ch: FFTresult瀛樻斁缁撴灉鐨勭粨鏋勪綋
 */
void FFT_win_u32(uint32_t* input, int length, float32_t* win, FFTresult* ch)
{
    int32_t i;
	float32_t sum = 0;
	for (i = 0; i < length; i++)
	{
		sum += input[i];
	}
	ch->dc = sum / length;
	for (i = 0; i < length; i++)
	{
		int temp = BitReverse(i, length);
		ch->result[i].real = ((float32_t)input[temp] - ch->dc) * win[temp];
		ch->result[i].imag = 0;
	}
	FFT(length, ch);
}

/*
 *杈撳叆鏁版嵁鐨勭被鍨嬩负float鐨勫姞绐桭FT
 *input: 杈撳叆鏁版嵁
 *length: FFT鐐规暟锛屽繀椤讳负2鐨勬暣鏁版鏂�
 *win: 绐楀嚱鏁�
 *ch: FFTresult瀛樻斁缁撴灉鐨勭粨鏋勪綋
 */
void FFT_win_f32(float32_t* input, int length, float32_t* win, FFTresult* ch)
{
    int32_t i;
	float32_t sum = 0;
	for (i = 0; i < length; i++)
	{
		sum += input[i];
	}
	ch->dc = sum / length;
	for (i = 0; i < length; i++)
	{
		int temp = BitReverse(i, length);
		ch->result[i].real = (input[temp] - ch->dc) * win[temp];
		ch->result[i].imag = 0;
	}
	FFT(length, ch);
}

/*
 *杈撳叆鏁版嵁鐨勭被鍨嬩负int鐨勫姞绐桭FT
 *input: 杈撳叆鏁版嵁
 *length: FFT鐐规暟锛屽繀椤讳负2鐨勬暣鏁版鏂�
 *win: 绐楀嚱鏁�
 *ch: FFTresult瀛樻斁缁撴灉鐨勭粨鏋勪綋
 */
void FFT_win_int32(int* input, int length, float32_t* win, FFTresult* ch)
{
    int32_t i;
	float32_t sum = 0;
	for (i = 0; i < length; i++)
	{
		sum += input[i];
	}
	ch->dc = sum / length;
	for (i = 0; i < length; i++)
	{
		int temp = BitReverse(i, length);
		ch->result[i].real = ((float32_t)input[temp] - ch->dc) * win[temp];
		ch->result[i].imag = 0;
	}
	FFT(length, ch);
}

void FFT_win_int16(int16_t* input, int length, float32_t* win, FFTresult* ch)
{
	float32_t sum = 0;
	int32_t i;
	for (i = 0; i < length; i++)
	{
		sum += input[i];
	}
	ch->dc = sum / length;
	for (i = 0; i < length; i++)
	{
		int temp = BitReverse(i, length);
		ch->result[i].real = ((float32_t)input[temp] - ch->dc) * win[temp];
		ch->result[i].imag = 0;
	}
	FFT(length, ch);
}

/*
 *鎭㈠鍔犵獥鍚庡甫鏉ョ殑骞呭�艰“鍑�
 *ch: 寰呮仮澶嶅箙鍊肩殑FFT缁撴灉
 *length: FFT鐐规暟
 *k: 鎭㈠绯绘暟锛岀敓鎴愮獥鍑芥暟鏃剁殑杩斿洖鍊煎氨鏄仮澶嶇郴鏁�
 */
void AmpRecovery(FFTresult* ch, int length, float32_t k)
{
//    int32_t i;
//	for (i = 0; i < length; i++)
//	{
//		ch->Amp[i] *= k;
//	}
}

/*
 *姣旂壒鍙嶈浆锛屼负浜嗗仛铦跺舰杩愮畻闇�瑕佸皢鏁扮粍閲嶆柊鎺掑簭
 *閲囩敤姣旂壒鍙嶈浆娉曞彲浠ョ敓鎴愭柊鐨勬暟缁勪笅鏍�
 *input: 鍘熷鐨勪笅鏍�
 *N: FFT鐐规暟锛屽繀椤绘槸2鐨勬暣鏁版鏂�
 *return: 鐢熸垚鐨勬柊鏁扮粍涓嬫爣
 */
static int BitReverse(int input, int N)
{
	int output = 0;
	int32_t i;

	int times = (int)ceil(log2f((float32_t)N));
	for (i = 0; i < times; i++)
	{
		output <<= 1;
		output += (input % 2);
		input /= 2;
	}

	return output;
}

/*
 *閫掑綊鐢―FT
 */
static void DFT(Complex* p, int len)
{
	if (len == 1)
	{
		return;
	}
	else
	{
		DFT(p, len / 2);
		DFT(p + len / 2, len / 2);
		int32_t i;
		for (i = 0; i < len / 2; i++)
		{
			Complex c1 = *p;
			Complex c2 = *(p + len / 2);
			Complex w;//鏃嬭浆鍥犲瓙
			if (use_table == 0)
			{
				w.real = (float32_t)cos(2 * pi / len * i);
				w.imag = -(float32_t)sin(2 * pi / len * i);
			}
			else
			{
				w.real = Mycos(len, i);
				w.imag = -Mysin(len, i);
			}
			*p = Complex_Add(c1, Complex_Multi(c2, w));
			*(p + len / 2) = Complex_Minus(c1, Complex_Multi(c2, w));
			p++;
		}
		return;
	}
}

// 澶嶆暟涔樻硶锛岃繑鍥炲�肩瓑浜巆1 * c2
Complex Complex_Multi(Complex c1, Complex c2)
{
	Complex result;
	result.real = c1.real * c2.real - c1.imag * c2.imag;
	result.imag = c1.real * c2.imag + c1.imag * c2.real;
	return result;
}

// 澶嶆暟鍔犳硶锛岃繑鍥炲�肩瓑浜巆1 + c2
Complex Complex_Add(Complex c1, Complex c2)
{
	Complex result;
	result.real = c1.real + c2.real;
	result.imag = c1.imag + c2.imag;
	return result;
}

// 澶嶆暟鍑忔硶锛岃繑鍥炲�肩瓑浜巆1 - c2
Complex Complex_Minus(Complex c1, Complex c2)
{
	Complex result;
	result.real = c1.real - c2.real;
	result.imag = c1.imag - c2.imag;
	return result;
}

// 澶嶆暟姹傛ā
float32_t Complex_Modular(Complex c)
{
	return (float32_t)sqrt(c.real * c.real + c.imag * c.imag);
}

// 澶嶆暟姹傝緪瑙�
float32_t Complex_arg(Complex c)
{
	return atan2f(c.imag, c.real);
}

/*-------闃朵箻-------*/
static int Factorial(int num)
{
	int a = 1;
	int32_t i;
	for (i = num; i >= 1; i--)
	{
		a *= i;
	}
	return a;
}
/*--------0闃惰礉濉炲皵--------*/
static float32_t Besseli(float32_t x)
{
	float32_t a = 1.0;
	int32_t i;
	for (i = 1; i < 20; i++)
	{
		a += pow((pow((x / 2), i) * 1.0 / Factorial(i)), 2);
	}
	return a;
}
/*--------Kaiser绐�--------*/
float32_t Kaiser(int n, float32_t beta, float32_t* win)
{
    int32_t i;
	for (i = 0; i < n; i++)
	{
		win[i] = Besseli(beta * sqrt(1 - pow(2.0 * i / (n - 1) - 1, 2))) / Besseli(beta);
	}
	return 1;
}
/*--------Flattop绐�--------*/
float32_t FlattopWin(int n, float32_t* win)
{
	float32_t a0 = 0.21557895, a1 = 0.41663158, a2 = 0.277263158, a3 = 0.083578947, a4 = 0.006947368;
	int32_t i;
	for (i = 0; i < n; i++)
	{
		win[i] = a0 - a1 * cos(2 * pi * i / (n - 1)) + a2 * cos(4 * pi * i / (n - 1))
			- a3 * cos(6 * pi * i / (n - 1)) + a4 * cos(8 * pi * i / (n - 1));
	}
	return 4.639;
}
/*--------Hanning--------*/
float32_t Hanning(int n, float32_t* win)
{
    int32_t i;
	for (i = 0; i < n; i++)
	{
		win[i] = 0.5 * (1 - cos(2 * pi * i / (n - 1)));
	}
	return 2;
}
/*--------Hamming--------*/
float32_t Hamming(int n, float32_t* win)
{
    int32_t i;
	for (i = 0; i < n; i++)
	{
		win[i] = 0.54 - 0.46 * cos(2 * pi * i / (n - 1));
	}
	return 1.852;
}
/*--------rectangle--------*/
float32_t rectangle(int n, float32_t* win)
{
    int32_t i;
	for (i = 0; i < n; i++)
	{
		win[i] = 1;
	}
	return 1;
}

// sin鎵撹〃
static float32_t Mysin(int len, int i)
{
	if (len == 2)
	{
		return 0.0f;
	}
	else
	{
		if (i < len / 4)
		{
			return sin_table[NPT_max / len * i];
		}
		else if (i > len / 4)
		{
			i = len / 2 - i;
			return sin_table[NPT_max / len * i];
		}
		else
		{
			return 1.0f;
		}
	}
}
// cos鎵撹〃
static float32_t Mycos(int len, int i)
{
	if (len == 2)
	{
		return 1.0f;
	}
	else
	{
		i = (i + len / 4) % (len / 2);
		if (i < len / 4)
		{
			return -sin_table[NPT_max / len * i];
		}
		else if (i > len / 4)
		{
			i = len / 2 - i;
			return sin_table[NPT_max / len * i];
		}
		else
		{
			return 1.0f;
		}
	}
}
