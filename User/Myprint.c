#include "Myprint.h"
#include "device.h"
#include "driverlib.h"
int32_t send = 0;

void print(const char* ch, float value)
{
    int i = 0;
	int len = strlen(ch);
	for (i = 0; i < len; i++)
	{
		if (ch[i] == '{' && ch[i + 3] == '}' && ch[i + 1] == '.')
		{
			PrintFloat(value, (int)ch[i + 2] - 48);
			i += 3;
		}
		else
		{
			put(ch[i]);
		}
	}
}

void put(char ch)
{
    SCI_writeCharBlockingNonFIFO(SCIA_BASE, ch);
}

void PrintFloat(float value, int num)
{
	if (value < 0)
	{
		value = -value;
		put('-');
	}
	
	send = (int32_t)(value * powf(10, num) + 0.5f);
	if (send == 0)
	{
		put('0');
		put('.');
		int i = 0;
		for (i = 0; i < num; i++)
		{
			put('0');
		}
	}
	int start_pos = floor(log10f(send));
	int pos;
	for (pos = start_pos; pos >= 0; pos--)
	{
		if (value < 1 && pos == start_pos)
		{
			put('0');
			put('.');
			int i = 0;
			for (i = 0; i < num - pos - 1; i++)
			{
				put('0');
			}
		}
		if (pos == num - 1 && value >= 1)
		{
			put('.');
		}
		int32_t temp = (int32_t)(powf(10, pos) + 0.5f);
		put(send / temp + 48);
		send %= temp;
	}
}
