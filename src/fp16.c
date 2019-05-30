/****************************************************************************
**
** Copyright (C) 2019 King Brain Infotech Co., Ltd.
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in
** all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
** THE SOFTWARE.
**
****************************************************************************/

#include "fp16.h"
#include <math.h>

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

bool is_ftoh_loss(float f)
{
	if (f == 0.0 || !isfinite(f) || isnan(f))
	{
		return false;
	}

	uint32_t *p = (uint32_t *)(&f);
	int32_t mant = *p & 0x7fffff;
	uint16_t exp = (*p >> 23) & 0xff;

	if (exp >= 0x71 && exp < 0x8f)
	{
		if (!(mant & 0x1fff))
		{
			return false;
		}
	}

	if (exp >= 0x67 && exp < 0x71)
	{
		if (!(mant & ((1 << (0x7e - exp)) - 1)))
		{
			return false;
		}
	}

	return true;
}
