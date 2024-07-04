/*******************************************************************************
*
* (C) copyright 2003-2016, iBiquity Digital Corporation, U.S.A.
*
********************************************************************************

    This confidential and proprietary software may be used only as
    authorized by a licensing agreement from iBiquity Digital Corporation.
    In the event of publication, the following notice is applicable:

    The availability of this material does not provide any license
    by implication, or otherwise under any patent rights of iBiquity
    Digital Corporation or others covering any use of the
    contents herein.

    Any copies or derivative works must include this and all other
    proprietary notices.

        iBiquity Digital Corporation
        6711 Columbia Gateway Drive, Suite 500
        Columbia, MD USA 21046
*******************************************************************************/
/**
 * @file hdrBasicTypes.h
 * @brief Basic Type Definitions
 * @defgroup basicTypes Basic Type Definitions
 * @brief Basic Type Definitions
 * @ingroup HdrApi
 * @{
 */
#ifndef HDR_BASIC_TYPES
#define HDR_BASIC_TYPES

#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>

#include "tchdr_systypes.h"

/**
 * @brief Default platform type for float
 */
typedef float hdr_float32_t;

/**
 * @brief Define default platform type for int
 */
typedef int32_t int_t;

/**
 * @brief Define default platform type for unsigned int
 */
typedef uint32_t uint_t;

/**
 * @brief Complex 32-bit type definition comprising 16-bit Real + 16-bit Imaginary
 */
typedef struct{
    int16_t re;     /**< Real part */
    int16_t im;     /**< Imaginary part */
}int16c_t;

/**
 * @brief Complex, 64-bit integer type definition comprising 32-bit Real + 32-bit Imaginary
 */
typedef struct{
    int32_t re;     /**< Real part */
    int32_t im;     /**< Imaginary part */
}int32c_t;

/**
 * @brief Interlaced L/R 16-bit PCM audio samples
 */
typedef struct{
    int16_t left;   /**< Left channel PCM sample */
    int16_t right;  /**< Right channel PCM sample */
}HDR_pcm_stereo_t;

/**
 * @brief Macro for packed structure attribute
 */
#if defined(_M_X64) || defined(_M_IX86) //Compile for windows
#define PACKED_STRUCTURE __pragma(pack(1))
#else
#define PACKED_STRUCTURE __attribute__((packed))
#endif
/**
 * @brief Macro for Alignment attribute
 */
#if defined(_M_X64) || defined(_M_IX86) //Compile for windows
#define STATIC_ALIGN(t, v, w)   __declspec(align (w)) t v
#else
#define STATIC_ALIGN(t, v, w)   t v __attribute((aligned(w)))
#endif

#endif //HDR_BASIC_TYPES

/** @} */
