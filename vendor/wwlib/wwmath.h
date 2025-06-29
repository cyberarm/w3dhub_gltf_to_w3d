/*
**	Command & Conquer Renegade(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : WWMath                                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwmath/wwmath.h                              $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 3/01/02 9:06a                                               $*
 *                                                                                             *
 *                    $Revision:: 65                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#pragma once

#include <cmath>
#include <cfloat>
#include <cassert>

/*
** Some global constants.
*/
#define WWMATH_EPSILON      0.0001f
#define WWMATH_EPSILON2     WWMATH_EPSILON * WWMATH_EPSILON
#define WWMATH_PI           3.141592654f
#define WWMATH_FLOAT_MAX    (FLT_MAX)
#define WWMATH_FLOAT_MIN    (FLT_MIN)
#define WWMATH_SQRT2        1.414213562f
#define WWMATH_SQRT3        1.732050808f
#define WWMATH_OOSQRT2      0.707106781f
#define WWMATH_OOSQRT3      0.577350269f

/* 
**	Macros to convert between degrees and radians
*/
#ifndef RAD_TO_DEG
#define RAD_TO_DEG(x)    (((double)x)*180.0/WWMATH_PI)
#endif

#ifndef DEG_TO_RAD
#define DEG_TO_RAD(x)    (((double)x)*WWMATH_PI/180.0)
#endif

#ifndef RAD_TO_DEGF
#define RAD_TO_DEGF(x)    (((float)x)*180.0f/WWMATH_PI)
#endif

#ifndef DEG_TO_RADF
#define DEG_TO_RADF(x)    (((float)x)*WWMATH_PI/180.0f)
#endif


const int ARC_TABLE_SIZE = 1024;
const int SIN_TABLE_SIZE = 1024;
extern float _FastAcosTable[ARC_TABLE_SIZE];
extern float _FastAsinTable[ARC_TABLE_SIZE];
extern float _FastSinTable[SIN_TABLE_SIZE];
extern float _FastInvSinTable[SIN_TABLE_SIZE];

/*
** Some simple math functions which work on the built-in types.  
** Include the various other header files in the WWMATH library 
** in order to get matrices, quaternions, etc.
*/
class WWMath {
public:

// Initialization and Shutdown.  Other math sub-systems which require initialization and
// shutdown processing will be handled in these functions
    static void Init(void);

    static void Shutdown(void);

// These are meant to be a collection of small math utility functions to be optimized at some point.
    static inline float Fabs(float val) {
        int value = *(int *) &val;
        value &= 0x7fffffff;
        return *(float *) &value;
    }

    static inline int Float_To_Int_Chop(const float &f);

    static inline int Float_To_Int_Floor(const float &f);

#if defined(_MSC_VER) && defined(_M_IX86)
    static inline float Cos(float val);
    static inline float Sin(float val);
    static inline float Sqrt(float val);
    static float __fastcall Inv_Sqrt(float a);	// Some 30% faster inverse square root than regular C++ compiled, from Intel's math library
    static inline long	 Float_To_Long(float f);
#else

    static float Cos(float val);

    static float Sin(float val);

    static float Sqrt(float val);

    static float Inv_Sqrt(float a);

    static long Float_To_Long(float f);

#endif


    static inline float Fast_Sin(float val);

    static inline float Fast_Inv_Sin(float val);

    static inline float Fast_Cos(float val);

    static inline float Fast_Inv_Cos(float val);

    static inline float Fast_Acos(float val);

    static inline float Acos(float val);

    static inline float Fast_Asin(float val);

    static inline float Asin(float val);


    static float Atan(float x) { return static_cast<float>(atan(x)); }

    static float Atan2(float y, float x) { return static_cast<float>(atan2(y, x)); }

    static float Sign(float val);

    static float Ceil(float val) { return ceilf(val); }

    static float Floor(float val) { return floorf(val); }

    static bool Fast_Is_Float_Positive(const float &val);

    static float Random_Float(void);

    static float Random_Float(float min, float max);

    static float Clamp(float val, float min = 0.0f, float max = 1.0f);

    static double Clamp(double val, double min = 0.0f, double max = 1.0f);

    static int Clamp_Int(int val, int min_val, int max_val);

    static float Wrap(float val, float min = 0.0f, float max = 1.0f);

    static double Wrap(double val, double min = 0.0f, double max = 1.0f);

    static float Min(float a, float b);

    static float Max(float a, float b);

    static float Lerp(float a, float b, float lerp);

    static double Lerp(double a, double b, float lerp);

    static long Float_To_Long(double f);

    static unsigned char Unit_Float_To_Byte(float f) { return (unsigned char) (f * 255.0f); }

    static float Byte_To_Unit_Float(unsigned char byte) { return ((float) byte) / 255.0f; }

    static bool Is_Valid_Float(float x);

    static bool Is_Valid_Double(double x);

};

inline float WWMath::Sign(float val) {
    if (val > 0.0f) {
        return +1.0f;
    }
    if (val < 0.0f) {
        return -1.0f;
    }
    return 0.0f;
}

inline bool WWMath::Fast_Is_Float_Positive(const float &val) {
    return !((*(int *) (&val)) & 0x80000000);
}

inline float WWMath::Random_Float(float min, float max) {
    return Random_Float() * (max - min) + min;
}

inline float WWMath::Clamp(float val, float min /*= 0.0f*/, float max /*= 1.0f*/) {
    if (val < min) return min;
    if (val > max) return max;
    return val;
}

inline double WWMath::Clamp(double val, double min /*= 0.0f*/, double max /*= 1.0f*/) {
    if (val < min) return min;
    if (val > max) return max;
    return val;
}

inline int WWMath::Clamp_Int(int val, int min_val, int max_val) {
    if (val < min_val) return min_val;
    if (val > max_val) return max_val;
    return val;
}

inline float WWMath::Wrap(float val, float min /*= 0.0f*/, float max /*= 1.0f*/) {
    // Implemented as an if rather than a while, to long loops
    if (val >= max) val -= (max - min);
    if (val < min) val += (max - min);

    if (val < min) {
        val = min;
    }
    if (val > max) {
        val = max;
    }
    return val;
}

inline double WWMath::Wrap(double val, double min /*= 0.0f*/, double max /*= 1.0f*/) {
    // Implemented as an if rather than a while, to long loops
    if (val >= max) val -= (max - min);
    if (val < min) val += (max - min);
    if (val < min) {
        val = min;
    }
    if (val > max) {
        val = max;
    }
    return val;
}

inline float WWMath::Min(float a, float b) {
    if (a < b) return a;
    return b;
}

inline float WWMath::Max(float a, float b) {
    if (a > b) return a;
    return b;
}

inline float WWMath::Lerp(float a, float b, float lerp) {
    return (a + (b - a) * lerp);
}

inline double WWMath::Lerp(double a, double b, float lerp) {
    return (a + (b - a) * lerp);
}


inline bool WWMath::Is_Valid_Float(float x) {
    unsigned long *plong = (unsigned long *) (&x);
    unsigned long exponent = ((*plong) & 0x7F800000) >> (32 - 9);

    // if exponent is 0xFF, this is a NAN
    if (exponent == 0xFF) {
        return false;
    }
    return true;
}

inline bool WWMath::Is_Valid_Double(double x) {
    unsigned long *plong = (unsigned long *) (&x) + 1;
    unsigned long exponent = ((*plong) & 0x7FF00000) >> (32 - 12);

    // if exponent is 0x7FF, this is a NAN
    if (exponent == 0x7FF) {
        return false;
    }
    return true;
}

// ----------------------------------------------------------------------------
// Float to long
// ----------------------------------------------------------------------------

#if defined(_MSC_VER) && defined(_M_IX86)
inline long WWMath::Float_To_Long(float f)
{
    long i;

    __asm {
        fld [f]
        fistp [i]
    }

    return i;
}
#else

inline long WWMath::Float_To_Long(float f) {
    return (long) f;
}

#endif

inline long WWMath::Float_To_Long(double f) {
#if defined(_MSC_VER) && defined(_M_IX86)
    long retval;
    __asm fld	qword ptr [f]
    __asm fistp dword ptr [retval]
    return retval;
#else
    return (long) f;
#endif
}

// ----------------------------------------------------------------------------
// Cos
// ----------------------------------------------------------------------------

#if defined(_MSC_VER) && defined(_M_IX86)
inline float WWMath::Cos(float val)
{
    float retval;
    __asm {
        fld [val]
        fcos
        fstp [retval]
    }
    return retval;
}
#else

inline float WWMath::Cos(float val) {
    return cosf(val);
}

#endif

// ----------------------------------------------------------------------------
// Sin
// ----------------------------------------------------------------------------

#if defined(_MSC_VER) && defined(_M_IX86)
inline float WWMath::Sin(float val)
{
    float retval;
    __asm {
        fld [val]
        fsin
        fstp [retval]
    }
    return retval;
}
#else

inline float WWMath::Sin(float val) {
    return sinf(val);
}

#endif

// ----------------------------------------------------------------------------
// Fast, table based sin
// ----------------------------------------------------------------------------

inline float WWMath::Fast_Sin(float val) {
    val *= float(SIN_TABLE_SIZE) / (2.0f * WWMATH_PI);

    int idx0 = Float_To_Int_Floor(val);
    int idx1 = idx0 + 1;
    float frac = val - (float) idx0;

    idx0 = ((unsigned) idx0) & (SIN_TABLE_SIZE - 1);
    idx1 = ((unsigned) idx1) & (SIN_TABLE_SIZE - 1);

    return (1.0f - frac) * _FastSinTable[idx0] + frac * _FastSinTable[idx1];
}

// ----------------------------------------------------------------------------
// Fast, table based 1.0f/sin
// ----------------------------------------------------------------------------

inline float WWMath::Fast_Inv_Sin(float val) {
#if 0 // TODO: more testing, not reliable! 
    float index = val * float(SIN_TABLE_SIZE) / (2.0f * WWMATH_PI);

    int idx0=Float_To_Int_Floor(index);
    int idx1=idx0+1;
    float frac=val-(float)idx0;

    idx0 = ((unsigned)idx0) & (SIN_TABLE_SIZE-1);
    idx1 = ((unsigned)idx1) & (SIN_TABLE_SIZE-1);

    // The table becomes inaccurate near 0 and 2pi so fall back to doing a divide.
    const int BUFFER = 16;
    if ((idx0 <= BUFFER) || (idx0 >= SIN_TABLE_SIZE-BUFFER-1)) {
        return 1.0f / WWMath::Fast_Sin(val);
    } else {
        return (1.0f - frac) * _FastInvSinTable[idx0] + frac * _FastInvSinTable[idx1];
    }
#else
    return 1.0f / WWMath::Fast_Sin(val);
#endif
}


// ----------------------------------------------------------------------------
// Fast, table based cos
// ----------------------------------------------------------------------------

inline float WWMath::Fast_Cos(float val) {
    val += (WWMATH_PI * 0.5f);
    val *= float(SIN_TABLE_SIZE) / (2.0f * WWMATH_PI);

    int idx0 = Float_To_Int_Floor(val);
    int idx1 = idx0 + 1;
    float frac = val - (float) idx0;

    idx0 = ((unsigned) idx0) & (SIN_TABLE_SIZE - 1);
    idx1 = ((unsigned) idx1) & (SIN_TABLE_SIZE - 1);

    return (1.0f - frac) * _FastSinTable[idx0] + frac * _FastSinTable[idx1];
}

// ----------------------------------------------------------------------------
// Fast, table based 1.0f/cos
// ----------------------------------------------------------------------------

inline float WWMath::Fast_Inv_Cos(float val) {
#if 0 // TODO: more testing, not reliable!
    float index = val + (WWMATH_PI * 0.5f);
    index *= float(SIN_TABLE_SIZE) / (2.0f * WWMATH_PI);

    int idx0=Float_To_Int_Chop(index);
    int idx1=idx0+1;
    float frac=val-(float)idx0;

    idx0 = ((unsigned)idx0) & (SIN_TABLE_SIZE-1);
    idx1 = ((unsigned)idx1) & (SIN_TABLE_SIZE-1);

    // The table becomes inaccurate near 0 and 2pi so fall back to doing a divide.
    if ((idx0 <= 2) || (idx0 >= SIN_TABLE_SIZE-3)) {
        return 1.0f / WWMath::Fast_Cos(val);
    } else {
        return (1.0f - frac) * _FastInvSinTable[idx0] + frac * _FastInvSinTable[idx1];
    }
#else
    return 1.0f / WWMath::Fast_Cos(val);
#endif
}

// ----------------------------------------------------------------------------
// Fast, table based arc cos
// ----------------------------------------------------------------------------

inline float WWMath::Fast_Acos(float val) {
    // Near -1 and +1, the table becomes too inaccurate
    if (WWMath::Fabs(val) > 0.975f) {
        return WWMath::Acos(val);
    }

    val *= float(ARC_TABLE_SIZE / 2);

    int idx0 = Float_To_Int_Floor(val);
    int idx1 = idx0 + 1;
    float frac = val - (float) idx0;

    idx0 += ARC_TABLE_SIZE / 2;
    idx1 += ARC_TABLE_SIZE / 2;

    // we dont even get close to the edge of the table...
    assert((idx0 >= 0) && (idx0 < ARC_TABLE_SIZE));
    assert((idx1 >= 0) && (idx1 < ARC_TABLE_SIZE));

    // compute and return the interpolated value
    return (1.0f - frac) * _FastAcosTable[idx0] + frac * _FastAcosTable[idx1];
}

// ----------------------------------------------------------------------------
// Arc cos
// ----------------------------------------------------------------------------

inline float WWMath::Acos(float val) {
    return (float) acos(val);
}

// ----------------------------------------------------------------------------
// Fast, table based arc sin
// ----------------------------------------------------------------------------

inline float WWMath::Fast_Asin(float val) {
    // Near -1 and +1, the table becomes too inaccurate
    if (WWMath::Fabs(val) > 0.975f) {
        return WWMath::Asin(val);
    }

    val *= float(ARC_TABLE_SIZE / 2);

    int idx0 = Float_To_Int_Floor(val);
    int idx1 = idx0 + 1;
    float frac = val - (float) idx0;

    idx0 += ARC_TABLE_SIZE / 2;
    idx1 += ARC_TABLE_SIZE / 2;

    // we dont even get close to the edge of the table...
    assert((idx0 >= 0) && (idx0 < ARC_TABLE_SIZE));
    assert((idx1 >= 0) && (idx1 < ARC_TABLE_SIZE));

    // compute and return the interpolated value
    return (1.0f - frac) * _FastAsinTable[idx0] + frac * _FastAsinTable[idx1];
}

// ----------------------------------------------------------------------------
// Arc sin
// ----------------------------------------------------------------------------

inline float WWMath::Asin(float val) {
    return (float) asin(val);
}

// ----------------------------------------------------------------------------
// Sqrt
// ----------------------------------------------------------------------------

#if defined(_MSC_VER) && defined(_M_IX86)
inline float WWMath::Sqrt(float val)
{
    float retval;
    __asm {
        fld [val]
        fsqrt
        fstp [retval]
    }
    return retval;
}
#else

inline float WWMath::Sqrt(float val) {
    return (float) sqrt(val);
}

#endif

inline int WWMath::Float_To_Int_Chop(const float &f) {
    int a = *reinterpret_cast<const int *>(&f);                // take bit pattern of float into a register
    int sign = (a
            >> 31);                                                // sign = 0xFFFFFFFF if original value is negative, 0 if positive
    int mantissa = (a & ((1 << 23) - 1)) | (1 << 23);                        // extract mantissa and add the hidden bit
    int exponent = ((a & 0x7fffffff) >> 23) - 127;                    // extract the exponent
    int r = ((unsigned int) (mantissa) << 8)
            >> (31 - exponent);    // ((1<<exponent)*mantissa)>>24 -- (we know that mantissa > (1<<24))
    return ((r ^ (sign)) - sign) &
           ~(exponent >> 31);            // add original sign. If exponent was negative, make return value 0.
}

inline int WWMath::Float_To_Int_Floor(const float &f) {
    int a = *reinterpret_cast<const int *>(&f);            // take bit pattern of float into a register
    int sign = (a
            >> 31);                                                // sign = 0xFFFFFFFF if original value is negative, 0 if positive
    a &= 0x7fffffff;                                                            // we don't need the sign any more

    int exponent = (a >> 23) - 127;                                        // extract the exponent
    int expsign = ~(exponent
            >> 31);                                    // 0xFFFFFFFF if exponent is positive, 0 otherwise
    int imask = ((1 << (31 - (exponent)))) - 1;                    // mask for true integer values
    int mantissa = (a & ((1 << 23) - 1));                                // extract mantissa (without the hidden bit)
    int r = ((unsigned int) (mantissa | (1 << 23)) << 8)
            >> (31 - exponent);    // ((1<<exponent)*(mantissa|hidden bit))>>24 -- (we know that mantissa > (1<<24))

    r = ((r & expsign) ^ (sign)) + ((!((mantissa << 8) & imask) & (expsign ^ ((a - 1) >> 31))) &
                                    sign);    // if (fabs(value)<1.0) value = 0; copy sign; if (value < 0 && value==(int)(value)) value++;
    return r;
}

// ----------------------------------------------------------------------------
// Inverse square root
// ----------------------------------------------------------------------------

#if defined(_MSC_VER) && defined(_M_IX86)
inline __declspec(naked) float __fastcall WWMath::Inv_Sqrt(float a)
{
    __asm {
        mov		eax, 0be6eb508h
        mov		DWORD PTR [esp-12],03fc00000h ;  1.5 on the stack
        sub		eax, DWORD PTR [esp+4]; a
        sub		DWORD PTR [esp+4], 800000h ; a/2 a=Y0
        shr		eax, 1     ; firs approx in eax=R0
        mov		DWORD PTR [esp-8], eax

        fld		DWORD PTR [esp-8] ;r
        fmul	st, st            ;r*r
        fld		DWORD PTR [esp-8] ;r
        fxch	st(1)
        fmul	DWORD PTR [esp+4];a ;r*r*y0
        fld		DWORD PTR [esp-12];load 1.5
        fld		st(0)
        fsub	st,st(2)			   ;r1 = 1.5 - y1
        ;x1 = st(3)
        ;y1 = st(2)
        ;1.5 = st(1)
        ;r1 = st(0)

        fld		st(1)
        fxch	st(1)
        fmul	st(3),st			; y2=y1*r1*...
        fmul	st(3),st			; y2=y1*r1*r1
        fmulp	st(4),st            ; x2=x1*r1
        fsub	st,st(2)               ; r2=1.5-y2
        ;x2=st(3)
        ;y2=st(2)
        ;1.5=st(1)
        ;r2 = st(0)

        fmul	st(2),st			;y3=y2*r2*...
        fmul	st(3),st			;x3=x2*r2
        fmulp	st(2),st			;y3=y2*r2*r2
        fxch	st(1)
        fsubp	st(1),st			;r3= 1.5 - y3
        ;x3 = st(1)
        ;r3 = st(0)
        fmulp	st(1), st
        ret 4
    }
}
#else

inline float WWMath::Inv_Sqrt(float val) {
    return 1.0f / (float) sqrt(val);
}

#endif
