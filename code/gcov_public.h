/**********************************************************************/
/** @addtogroup embedded_gcov
 * @{
 * @file
 * @version $Id: $
 *
 * @author 2021-08-31 kjpeters  Working and cleaned up version.
 * @author 2021-09-20 kjpeters  Provide optional printf imitation.
 * @author 2022-01-03 kjpeters  Add file output.
 *
 * @note Based on GCOV-related code of the Linux kernel,
 * as described online by Thanassis Tsiodras (April 2016)
 * https://www.thanassis.space/coverage.html
 * and by Alexander Tarasikov
 * http://allsoftwaresucks.blogspot.com/2015/05/gcov-is-amazing-yet-undocumented.html
 * with additional investigation, updating, cleanup, and portability
 * by Ken Peters.
 *
 * @brief Public header file for embedded gcov.
 *
 **********************************************************************/
#ifndef __GCOV_PUBLIC_H__
#define __GCOV_PUBLIC_H__

/* User-selectable compile-time options for the embedded gcov */
/* Not all have been fully tested */

/* select internal customizations ------------------------------------ */
/* needed to adapt to your system */

/* Allow functions to use malloc to get data buffers.
 * Not all embedded systems allow that, but instead
 * must declare all data in advance.
 * If not allowing malloc, might require your custom code
 * in gcov_public.c to set buffer sizes.
 */
//#define GCOV_OPT_USE_MALLOC

/* Allow functions to use othe stdlib functions.
 * Not all embedded systems allow that, but instead
 * haave their own functions or imitations.
 * If not allowing stdlib, might require your custom code
 * in gcov_public.c to call appropriate functions.
 */
//#define GCOV_OPT_USE_STDLIB

/* Allow functions to print status and error messages.
 * Not all embedded systems support that.
 * If not enabled, you might want custom code in gcov_public.c
 * to indicate some other way.
 * If defined, you must also provide defs below
 * for GCOV_PRINT_STR and GCOV_PRINT_NUM.
 */
#define GCOV_OPT_PRINT_STATUS

/* Reset watchdog timeout during gcov tree scanning.
 * Might be needed if you enable serial output on a slow port,
 * or if you have a very short watchdog timeout.
 * Requires your custom code in gcov_gcc.c
 */
//#define GCOV_OPT_RESET_WATCHDOG

/* Provide function to call constructor list (even in plain C)
 * (to call the gcc-generated code that calls __gcov_init).
 * Might be needed if you are not running a standard
 * C program startup that already does this (such as in boot code).
 * If defined, requires your custom linker file code as described below.
 */
//#define GCOV_OPT_PROVIDE_CALL_CONSTRUCTORS

#ifdef GCOV_OPT_PROVIDE_CALL_CONSTRUCTORS
/* start and end of constructor section defined in link file */
/* you have to provide appropriate linker file code to define these,
 * if your compiler/runtime environment does not automatically.
 * An example linker file segment:

.ctors : {
        __ctor_list = . ;
        *(SORT(.ctors.*))
        *(.ctors)
        __ctor_end = . ;
        . = ALIGN(16);
} > ram

 */
extern void *__ctor_list;
extern void *__ctor_end;
#endif // GCOV_OPT_PROVIDE_CALL_CONSTRUCTORS

/* Provide function to clear the counter data.
 * This is only needed if you want to be able to clear
 * the counter data after startup (the counters start up at zero).
 * Might be needed if you want to avoid counting your FSW startup,
 * or if you want to clear the counters between tests.
 * You might NOT want this if you are extremely memory constrained
 * (such as in PROM) and do not need this function to take up some bytes.
 */
#define GCOV_OPT_PROVIDE_CLEAR_COUNTERS

/* Provide small imitation printf function.
 * This is only needed if you want serial port outputs and
 * do not have already-existing functions to do the printing.
 * If you select this, then you need to provide a function
 * write_bytes() that does the actual serial output in your system.
 * See gcov_printf.c
 */
#define GCOV_OPT_PROVIDE_PRINTF_IMITATION

/* select data output method(s) ------------------------------------ */

/* Other output methods might be imagined,
 * if you have flash that can be written directly, etc.
 * Can be nice to write the concatenated results
 * to one file instead of a bunch of separate .gcda files.
 */
/* Output gcda data as binary format in file.
 * Might require your custom code in gcov_public.c
 * if your file headers and functions are non-standard.
 * Can be combined with other GCOV_OPT_OUTPUT_* options.
 */
//#define GCOV_OPT_OUTPUT_BINARY_FILE

/* Modify this output filename if desired */
/* Not used if you do not define GCOV_OPT_OUTPUT_BINARY_FILE */
#define GCOV_OUTPUT_BINARY_FILENAME "gcov_output.bin"

/* Modify file headers, data type and functions, if needed */
/* Not used if you do not define GCOV_OPT_OUTPUT_BINARY_FILE */
#ifdef GCOV_OPT_OUTPUT_BINARY_FILE
#if 1
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

typedef int GCOV_FILE_TYPE;
#define GCOV_OPEN_FILE(filename) open((filename), (O_CREAT|O_WRONLY), (S_IRWXU|S_IRWXG|S_IRWXO))
#define GCOV_OPEN_ERROR(fileref) ((fileref) < 0)
#define GCOV_CLOSE_FILE(fileref) close((fileref))
#define GCOV_WRITE_BYTE(fileref, char_var) write((fileref), &(char_var), (1))
#else
#include <stdio.h>

typedef FILE * GCOV_FILE_TYPE;
#define GCOV_OPEN_FILE(filename) fopen((filename), ("wb"))
#define GCOV_OPEN_ERROR(fileref) ((fileref) == NULL)
#define GCOV_CLOSE_FILE(fileref) fclose((fileref))
#define GCOV_WRITE_BYTE(fileref, char_var) fprintf((fileref), "%c", (char_var))
#endif
#endif // GCOV_OPT_OUTPUT_BINARY_FILE

/* Output gcda data as binary format in memory block.
 * Requires your custom code in gcov_public.c
 * to set the starting address of the block.
 * Can be combined with other GCOV_OPT_OUTPUT_* options.
 */
//#define GCOV_OPT_OUTPUT_BINARY_MEMORY

/* Output gcda data as hexdump format ASCII on serial port.
 * Might require your custom code in gcov_public.c
 * if your serial headers and functions are not stdio.h,
 * puts, and printf.
 * If defined, you must also provide defs below
 * for GCOV_PRINT_STR and GCOV_PRINT_NUM.
 * Can be combined with other GCOV_OPT_OUTPUT_* options.
 */
#define GCOV_OPT_OUTPUT_SERIAL_HEXDUMP

/* Function to print a string without newline.
 * Not used if you don't define either GCOV_OPT_PRINT_STATUS
 * or GCOV_OPT_OUTPUT_SERIAL_HEXDUMP.
 * If you do, you need to set this as appropriate for your system.
 * You might need to add header files to gcc_public.c
 */
//#define GCOV_PRINT_STR(str) fputs((str), stdout)
//#define GCOV_PRINT_STR(str) printf("%s", str)
#define GCOV_PRINT_STR(str) gcov_printf("%s", str)
//#define GCOV_PRINT_STR(str) puts((str))

/* Function to print a number without newline.
 * Not used if you don't define either GCOV_OPT_PRINT_STATUS
 * or GCOV_OPT_OUTPUT_SERIAL_HEXDUMP.
 * If you do, you need to set this as appropriate for your system.
 * You might need to add header files to gcc_public.c
 */
//#define GCOV_PRINT_NUM(num) printf("%d", (num))
#define GCOV_PRINT_NUM(num) gcov_printf("%d", (num))
//#define GCOV_PRINT_NUM(num) print_num((num))

/* Function to print hexdump address.
 * Not used if you don't define GCOV_OPT_OUTPUT_SERIAL_HEXDUMP.
 * If you do, you need to set this as appropriate for your system.
 * You might need to add header files to gcc_public.c
 */
//#define GCOV_PRINT_HEXDUMP_ADDR(num) printf("%08x: ", (num))
#define GCOV_PRINT_HEXDUMP_ADDR(num) gcov_printf("%08x: ", (num))

/* Function to print hexdump data value.
 * Not used if you don't define GCOV_OPT_OUTPUT_SERIAL_HEXDUMP.
 * If you do, you need to set this as appropriate for your system.
 * You might need to add header files to gcc_public.c
 */
//#define GCOV_PRINT_HEXDUMP_DATA(num) printf("%02x ", (num))
#define GCOV_PRINT_HEXDUMP_DATA(num) gcov_printf("%02x ", (num))

/* End of user settings ---------------------------------- */

/* Opaque gcov_info. The gcov structures can change as for example in gcc 4.7 so
 * we cannot use full definition here and they need to be placed in gcc specific
 * implementation of gcov. This also means no direct access to the members in
 * generic code and usage of the interface below.*/
struct gcov_info;

/* Compare to gcc/gcov-io.h */
typedef unsigned gcov_unsigned_t;
typedef long long gcov_type;

/* Compare to libgcc/libgcov.h */
void __gcov_init(struct gcov_info *info);
void __gcov_exit(void);
void __gcov_merge_add(gcov_type *counters, gcov_unsigned_t n_counters);

/* Our own creations */
#ifdef GCOV_OPT_PROVIDE_CLEAR_COUNTERS
void __gcov_clear(void);
#endif
#ifdef GCOV_OPT_PROVIDE_CALL_CONSTRUCTORS
void __gcov_call_constructors(void);
#endif

#ifdef GCOV_OPT_PROVIDE_PRINTF_IMITATION
void gcov_printf(const char *fmt, ...);
#endif

#endif // __GCOV_PUBLIC_H__

/** @}
 */
/*
 * embedded-gcov gcov_public.h gcov application interface code
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
