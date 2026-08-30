/***********************************************************
*
*	Sine Generation Module
*
*	File Name : sinef.c
*	Date Created :
*	Last modified :
*
************************************************************/

#include <math.h>


#define PI		3.141592654

int GenSine( float *buf, float freq, float sampling_freq, int len, int index )
{
	int i;
	float w=2.*PI*freq/sampling_freq;

	for(i=0; i < len; i++)
	{
		buf[i] = sinf(w*index++);
		if( index == sampling_freq )
			index = 0;
	}

	return index;
}

