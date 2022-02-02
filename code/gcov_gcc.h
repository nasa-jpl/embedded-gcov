/**********************************************************************/
/** @addtogroup embedded_gcov
 * @{
 * @file
 * @version $Id: $
 *
 * @author 2021-08-31 kjpeters  Working and cleaned up version.
 * @author 2022-01-07 kjpeters  Adjust gcc releases for GCOV_COUNTERS.
 *
 * @note Based on GCOV-related code of the Linux kernel,
 * as described online by Thanassis Tsiodras (April 2016)
 * https://www.thanassis.space/coverage.html
 * and by Alexander Tarasikov
 * http://allsoftwaresucks.blogspot.com/2015/05/gcov-is-amazing-yet-undocumented.html
 * with additional investigation, updating, cleanup, and portability
 * by Ken Peters.
 *
 * @brief Private header file for embedded gcov.
 *
 **********************************************************************/
/*
 * This code is based on the GCOV-related code of the Linux kernel (kept under
 * "kernel/gcov"). It basically uses the convert_to_gcda function to generate
 * the .gcda files information upon application completion, and dump it on the
 * host filesystem via GDB scripting.
 *
 * Original Linux banner follows below - but note that the Linux guys have
 * nothing to do with these modifications, so blame me (and contact me)
 * if something goes wrong.
 *
 * Thanassis Tsiodras
 * Real-time Embedded Software Engineer 
 * System, Software and Technology Department
 * European Space Agency
 *
 * e-mail: ttsiodras@gmail.com / Thanassis.Tsiodras@esa.int (work)
 *
 */


/*
 *  Profiling infrastructure declarations.
 *
 *  This file is based on gcc-internal definitions. Data structures are
 *  defined to be compatible with gcc counterparts. For a better
 *  understanding, refer to gcc source: gcc/gcov-io.h.
 *
 *    Copyright IBM Corp. 2009
 *    Author(s): Peter Oberparleiter <oberpar@linux.vnet.ibm.com>
 *
 *    Uses gcc-internal data definitions.
 */

#ifndef GCOV_GCC_H
#define GCOV_GCC_H GCOV_GCC_H

#include <stddef.h> // for size_t

#include "gcov_public.h"

/* Compare to gcc/gcov-counter.def and gcc/gcov-io.h */
/* GCC has changed this back and forth over time, do not know exact GCC versions */
/* This has been used with GCC 7.5.0 and GCC 11.1.0 */
#if (__GNUC__ >= 10)
#define GCOV_COUNTERS			8
#elif (__GNUC__ >= 5) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 9)
#define GCOV_COUNTERS			9
#else
#define GCOV_COUNTERS			8
#endif

/* Compare to gcc/gcov-io.h */

/*
 * Profiling data types used for gcc 3.4 and above - these are defined by
 * gcc and need to be kept as close to the original definition as possible to
 * remain compatible.
 * Compare to gcc/gcov-io.h
 * (gcc 11.1.0 (but not 11.2.0) multiplied counter length by 2*4,
 * but we do not need to duplicate that glitch(?))
 */
#define GCOV_DATA_MAGIC		((gcov_unsigned_t) 0x67636461)
#define GCOV_TAG_FUNCTION	((gcov_unsigned_t) 0x01000000)
#define GCOV_TAG_FUNCTION_LENGTH	(3)
#define GCOV_TAG_COUNTER_BASE	((gcov_unsigned_t) 0x01a10000)
#define GCOV_TAG_COUNTER_LENGTH(NUM) ((NUM) * 2)
#define GCOV_TAG_FOR_COUNTER(count) (GCOV_TAG_COUNTER_BASE + ((gcov_unsigned_t) (count) << 17))

/* Interface to access gcov_info data  */
/* Our own creation */
const char *gcov_info_filename(struct gcov_info *info);

/* Convert internal gcov data tree into .gcds output format */
/* Our own creation (though based on gcc internals, see source code) */
/* Need buffer to be 32-bit-aligned for type-safe internal usage */
size_t gcov_convert_to_gcda(gcov_unsigned_t *buffer, struct gcov_info *info);

/* Convert internal gcov data tree into .gcds output format */
/* Our own creation (though based on gcc internals, see source code) */
void gcov_clear_counters(struct gcov_info *gi_ptr);

#endif /* GCOV_GCC_H */

/** @}
 */
/*
 * embedded-gcov gcov_gcc.h gcov internals interface code
 *
 * Copyright (c) 2021 California Institute of Technology (“Caltech”).
 * U.S. Government sponsorship acknowledged.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *    Redistributions of source code must retain the above copyright notice,
 *        this list of conditions and the following disclaimer.
 *    Redistributions in binary form must reproduce the above copyright notice,
 *        this list of conditions and the following disclaimer in the documentation
 *        and/or other materials provided with the distribution.
 *    Neither the name of Caltech nor its operating division, the Jet Propulsion Laboratory,
 *        nor the names of its contributors may be used to endorse or promote products
 *        derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
