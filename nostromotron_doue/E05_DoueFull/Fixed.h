/*

    This file is part of the Aduino Piano Squealer.

    Arduino Piano Squealer is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Arduino Piano Squealer is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Arduino Piano Squealer.  If not, see <http://www.gnu.org/licenses/>.

    Copyright 2008, 2009 Marc Nostromo	

*/

// fixed point arithmatic

#ifndef _FIXED_H
#define _FIXED_H

#include <inttypes.h>

// We use 16 signed ints

typedef int16_t fixed;

#define FIXED_SHIFT		14
#define FIXED_SCALE		(1 << FIXED_SHIFT)

#define i2fp(a)			((a) << FIXED_SHIFT)
#define fp2i(a)			((a) >> FIXED_SHIFT)

#define fp_add(a, b)	((a) + (b))
#define fp_sub(a, b)	((a) - (b))

#define FP_FRACMASK FIXED_SCALE-1 ; 

#define fp_mul(x,y)		((fixed)(((int32_t)(x) * (int32_t)(y)) >> FIXED_SHIFT))


#define FP_ONE		(1 << FIXED_SHIFT)
#define FP_ZERO		(0)

extern fixed fl2fp(float val);
extern float fp2fl(fixed val);

#endif/*_FIXED_H*/





