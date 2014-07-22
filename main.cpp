#include <stdio.h>
#include <math.h>
#include <sys/timeb.h>

#ifdef _MSC_VER
typedef unsigned short stbr__uint16;
typedef   signed short stbr__int16;
typedef unsigned int   stbr__uint32;
typedef   signed int   stbr__int32;
#else
#include <stdint.h>
typedef uint16_t stbr__uint16;
typedef int16_t  stbr__int16;
typedef uint32_t stbr__uint32;
typedef int32_t  stbr__int32;
#endif

typedef unsigned char stbr_uc;

// for [0, 27] -> (n*10)^2.4
stbr__int32 stbr__pow_24_lookup[] =
{
	0,      // 0
	251,    // 10
	1326,   // 20
	3508,   // 30
	6998,   // 40
	11954,  // 50
	18517,  // 60
	26806,  // 70
	36933,  // 80
	48998,  // 90
	63096,  // 100
	79313,  // 110
	97732,  // 120
	118431, // 130
	141484, // 140
	166963, // 150
	194934, // 160
	225464, // 170
	258615, // 180
	294448, // 190
	333021, // 200
	374392, // 210
	418615, // 220
	465744, // 230
	515830, // 240
	568926, // 250
	625081, // 260
	684342, // 270
};

// Raises an integer to the power of 2.4
// Input is a fixed-point number in [0 270]
// Interpolates between entries in the above lookup table to return an answer.
stbr__int32 stbr__pow_24(stbr__int16 n)
{
#define STBR__LOOKUP_INTERVAL 10
	stbr__int32 i0 = stbr__pow_24_lookup[n / STBR__LOOKUP_INTERVAL];
	stbr__int32 i1 = stbr__pow_24_lookup[n / STBR__LOOKUP_INTERVAL + 1];

	stbr__int32 n0 = n - n % STBR__LOOKUP_INTERVAL;

	return (n - n0) * (i1 - i0) / STBR__LOOKUP_INTERVAL + i0;
#undef STBR__LOOKUP_INTERVAL
}

// For [0, 32] -> (n*1024)^(1/2.4)*128
stbr__int32 stbr__pow_417_lookup[] =
{
	0,    // 1024 * 0
	2299, // 1024 * 1
	3069, // 1024 * 2
	3633, // 1024 * 3
	4096, // 1024 * 4
	4495, // 1024 * 5
	4850, // 1024 * 6
	5172, // 1024 * 7
	5468, // 1024 * 8
	5743, // 1024 * 9
	6000, // 1024 * 10
	6243, // 1024 * 11
	6474, // 1024 * 12
	6693, // 1024 * 13
	6903, // 1024 * 14
	7105, // 1024 * 15
	7298, // 1024 * 16
	7485, // 1024 * 17
	7665, // 1024 * 18
	7840, // 1024 * 19
	8009, // 1024 * 20
	8174, // 1024 * 21
	8334, // 1024 * 22
	8490, // 1024 * 23
	8642, // 1024 * 24
	8790, // 1024 * 25
	8935, // 1024 * 26
	9076, // 1024 * 27
	9215, // 1024 * 28
	9350, // 1024 * 29
	9484, // 1024 * 30
	9614, // 1024 * 31
	9742, // 1024 * 32
};

// Raises an integer to the power of (1/2.4)
// Input is a fixed-point number in [0 2^15]
// Interpolates between entries in the above lookup table to return an answer.
// The answer is multiplied by 128 in order to retain precision. You must
// divide again by 128 to get the correct result when you want it.
stbr__int32 stbr__pow_417_times_128(stbr__int32 n)
{
#define STBR__LOOKUP_INTERVAL 1024
	if (n < 1024)
	{
		// The curve isn't flat enough here. Use a Taylor series approximation instead.
		// Taylor series for y = (x ^ (1/2.4) * 128) calculated using Wolfram Alpha.
		stbr__int32 x = n - 450;
		stbr__int32 x2 = x*x;
		return 1632 + (x * 1511090 - x2 * 979) / 1000000 + (x2*x * 11) / 10000000;
	}
	else
	{
		stbr__int32 i0 = stbr__pow_417_lookup[n / STBR__LOOKUP_INTERVAL];
		stbr__int32 i1 = stbr__pow_417_lookup[n / STBR__LOOKUP_INTERVAL + 1];

		stbr__int32 n0 = n - n % STBR__LOOKUP_INTERVAL;

		return (n - n0) * (i1 - i0) / STBR__LOOKUP_INTERVAL + i0;
	}
#undef STBR__LOOKUP_INTERVAL
}

// https://en.wikipedia.org/wiki/SRGB#The_reverse_transformation
static float stbr__srgb_to_linear_float(stbr_uc srgb)
{
	if (srgb <= 10)
		return ((float)srgb) / 255.0f / 12.92f;
	else
		return (float)pow((((float)srgb) / 255.0f + 0.055f) / 1.055f, 2.4f);
}

// https://en.wikipedia.org/wiki/SRGB#The_forward_transformation_.28CIE_xyY_or_CIE_XYZ_to_sRGB.29
static stbr_uc stbr__linear_to_srgb_float(float linear)
{
	if (linear <= 0.0031308f)
		return (stbr_uc)(linear * 12.92f * 255);
	else
		// The + 0.00001f is so that numbers just under a whole number round up.
		return (stbr_uc)((1.055f * pow(linear + 0.00001f, 0.41666666666f) - 0.055f) * 255);
}

stbr__int32 stbr__pow_24(stbr__int16 srgb);
stbr__int32 stbr__pow_417_times_128(stbr__int32 srgb);

// https://en.wikipedia.org/wiki/SRGB#The_reverse_transformation
static stbr__int32 stbr__srgb_to_linear_uc(stbr_uc srgb)
{
	if (srgb <= 10)
		return (srgb * 9946) / 1000;
	else
		return stbr__pow_24(srgb + 14) * 483 / 10000;
}

// https://en.wikipedia.org/wiki/SRGB#The_forward_transformation_.28CIE_xyY_or_CIE_XYZ_to_sRGB.29
static stbr_uc stbr__linear_to_srgb_uc(stbr__int32 linear)
{
	// The + 3 is so that numbers just under a whole number round up.
	if (linear <= 102)
		return ((linear + 3) * 1000) / 9946;
	else
		return stbr__pow_417_times_128(linear + 3) * 3535 / (1000 * 128) - 14;
}


void main()
{
	struct timeb initial_time_millis, final_time_millis;

	ftime(&initial_time_millis);

	for (long long i = 0; i < 100000; i++)
	{
		for (int j = 0; j < 255; j++)
		{
			float l = stbr__srgb_to_linear_float(j);
			int s = stbr__linear_to_srgb_float(l);
		}
	}

	ftime(&final_time_millis);

	long lapsed_ms = (long)(final_time_millis.time - initial_time_millis.time) * 1000 + (final_time_millis.millitm - initial_time_millis.millitm);
	printf("Floating point: %dms\n", lapsed_ms);

	ftime(&initial_time_millis);

	for (long long i = 0; i < 100000; i++)
	{
		for (int j = 0; j < 255; j++)
		{
			stbr__int32 l = stbr__srgb_to_linear_uc(i);
			int s = stbr__linear_to_srgb_uc(l);
		}
	}

	ftime(&final_time_millis);

	lapsed_ms = (long)(final_time_millis.time - initial_time_millis.time) * 1000 + (final_time_millis.millitm - initial_time_millis.millitm);
	printf("Integer: %dms\n", lapsed_ms);

	// My results:
	// Floating point: 9021ms
	// Integer: 4501ms
}
