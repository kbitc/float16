#include "fp16.h"

float htof(half h)
{
	uint16_t *p = (uint16_t *)(&h);

	uint32_t fval = 0;
	fval |= (*p & 0x8000) << 16;				// sign

	uint32_t mant = *p & 0x03ff;
	uint32_t exp = (*p & 0x7c00) >> 10;			// exponential
	if (exp == 0x1f)							// NaN or Infinity
	{
		fval |= mant ? 0x7fc00000 : 0x7f800000;
	}
	else if (exp > 0)							// normalized
	{
		fval |= (exp + 0x70) << 23;
		if (mant != 0)
		{
			fval |= mant << 13;
		}
	}
	else if (mant != 0)							// denormarlized
	{
		for (int i = 9; i >= 0; i--)
		{
			if (mant & (1 << i))
			{
				fval |= ((0x67 + i) << 23) | ((mant << (23 - i)) & 0x7fffff);
				break;
			}
		}
	}
	// else;									// 0.0, -0.0
	
	return *((float *)&fval);
}

half ftoh(float f)
{
	uint32_t *p = (uint32_t *)(&f);

	uint16_t hval = 0;
	hval |= (*p >> 16) & 0x8000;				// sign

	int32_t mant = *p & 0x7fffff;
	uint16_t exp = (*p >> 23) & 0xff;
	if (exp == 0xff)							// NaN or Infinity
	{
		hval |= mant ? 0x7e00 : 0x7c00;
	}
	else if (exp >= 0x8f)						// overflow, Infinity instead
	{
		hval |= 0x7c00;
	}
	else if (exp >= 0x71)						// normalized
	{
		hval |= ((exp - 0x70) << 10) | (mant >> 13);
	}
	else if (exp >= 0x67)						// denormalized
	{
		hval |= (mant | 0x800000) >> (0x7e - exp);
	}
	// else;									// 0.0, -0.0 or loss of precision

	return *((half *)&hval);
}
