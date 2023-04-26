/**********************************************************************/
/** @addtogroup embedded_gcov
 * @{
 * @file
 * @version $Id: $
 *
 * @author 2021-08-31 kjpeters  Working and cleaned up version.
 * @author 2021-09-20 kjpeters  Use preprocessor macros to hide printf.
 * @author 2022-01-03 kjpeters  Add file output.
 * @author 2022-07-31 kjpeters  Add reinit of static variables.
 *
 * @note Based on GCOV-related code of the Linux kernel,
 * as described online by Thanassis Tsiodras (April 2016)
 * https://www.thanassis.space/coverage.html
 * and by Alexander Tarasikov
 * http://allsoftwaresucks.blogspot.com/2015/05/gcov-is-amazing-yet-undocumented.html
 * with additional investigation, updating, cleanup, and portability
 * by Ken Peters.
 *
 * @brief Public source file for embedded gcov.
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
 *  This code maintains a list of active profiling data structures.
 *
 *    Copyright IBM Corp. 2009
 *    Author(s): Peter Oberparleiter <oberpar@linux.vnet.ibm.com>
 *
 *    Uses gcc-internal data definitions.
 *    Based on the gcov-kernel patch by:
 *		 Hubertus Franke <frankeh@us.ibm.com>
 *		 Nigel Hinds <nhinds@us.ibm.com>
 *		 Rajan Ravindran <rajancr@us.ibm.com>
 *		 Peter Oberparleiter <oberpar@linux.vnet.ibm.com>
 *		 Paul Larson
 */

#include "gcov_gcc.h"

typedef unsigned int u32;

#if defined(GCOV_OPT_USE_MALLOC) || defined(GCOV_OPT_USE_STDLIB)
#include <stdlib.h>
#endif

#if defined(GCOV_OPT_PRINT_STATUS) || defined(GCOV_OPT_OUTPUT_SERIAL_HEXDUMP)
/* Include any header files needed for serial port I/O */
/* Not always stdio.h for highly embedded systems */
#include <stdio.h>
//#include "all.h"
#endif

#ifdef GCOV_OPT_OUTPUT_BINARY_MEMORY
/* You need to set the output buffer pointer to your memory block */
/* Size used will depend on size and complexity of source code
 * that you have compiled for coverage. */
static unsigned char *gcov_output_buffer = (unsigned char *)(0x42000000);
static gcov_unsigned_t gcov_output_index;
#endif // GCOV_OPT_OUTPUT_BINARY_MEMORY

typedef struct tagGcovInfo {
    struct gcov_info *info;
    struct tagGcovInfo *next;
} GcovInfo;
static GcovInfo *gcov_headGcov = NULL;

#ifndef GCOV_OPT_USE_MALLOC
/* Declare space. Need one entry per file compiled for coverage. */
static GcovInfo gcov_GcovInfo[100];
static gcov_unsigned_t gcov_GcovIndex = 0;

/* Declare space. Needs to be enough for the largest single file coverage data. */
/* Size used will depend on size and complexity of source code
 * that you have compiled for coverage. */
/* Need buffer to be 32-bit-aligned for type-safe internal usage */
gcov_unsigned_t gcov_buf[8192];
#endif // not GCOV_OPT_USE_MALLOC

/* ----------------------------------------------------------- */
/*
 * __gcov_init is called by gcc-generated constructor code for each
 * object file compiled with -fprofile-arcs.
 *
 * Depending on how your embedded system starts up,
 * you may need to enable (in gcov_public.h) and call
 * the helper function __gcov_call_constructors below
 * from close to the beginning of your code.
 */

void __gcov_init(struct gcov_info *info)
{
    GcovInfo *newHead = NULL;

#ifdef GCOV_OPT_PRINT_STATUS
    GCOV_PRINT_STR("__gcov_init called for ");
    GCOV_PRINT_STR(gcov_info_filename(info));
    GCOV_PRINT_STR("\n");
#ifdef GCOV_OPT_USE_STDLIB
    fflush(stdout);
#endif // GCOV_OPT_USE_STDLIB
#endif // GCOV_OPT_PRINT_STATUS

#ifdef GCOV_OPT_USE_MALLOC
    newHead = malloc(sizeof(GcovInfo));
#else
    if (gcov_GcovIndex >= sizeof(gcov_GcovInfo)/sizeof(gcov_GcovInfo[0])) {
        newHead = NULL;
    } else {
        newHead = gcov_GcovInfo + gcov_GcovIndex;
    }
#endif // GCOV_OPT_USE_MALLOC else

    if (!newHead) {
#ifdef GCOV_OPT_PRINT_STATUS
        GCOV_PRINT_STR("Out of memory!"); GCOV_PRINT_STR("\n");
#endif // GCOV_OPT_PRINT_STATUS
#ifdef GCOV_OPT_USE_STDLIB
        exit(1);
#else
        return;
#endif // GCOV_OPT_USE_STDLIB
    }

    newHead->info = info;
    newHead->next = gcov_headGcov;
    gcov_headGcov = newHead;

#ifndef GCOV_OPT_USE_MALLOC
    gcov_GcovIndex++;
#endif // not GCOV_OPT_USE_MALLOC
}


/* ----------------------------------------------------------- */
#ifdef GCOV_OPT_PROVIDE_CALL_CONSTRUCTORS
/*
 * Your code may need to call this function to execute the
 * contructors that call __gcov_init (even in plain C).
 * Might be needed if you are not running a standard
 * C program startup (such as in boot code).
 *
 * If used, you need to provide a linker file that creates a constructor section,
 * and defines symbols __ctor_list at the start and __ctor_end at the end.
 * gcov_public.h has external declarations for those symbols.
 *
 * An example linker file segment:

.ctors : {
        __ctor_list = . ;
        *(SORT(.ctors.*))
        *(.ctors)
        __ctor_end = . ;
        . = ALIGN(16);
} > ram

 *
 */

void __gcov_call_constructors(void) {
    void **ctor;

    /* Reinitialize static variables.
     * In case of unusual situations, where your code re-executes
     * this function without your code actually restarting,
     * so that static variables would otherwise
     * be left as is, not reinitialized.
     *
     * This does not clear line counters that might also
     * remain in such a situation, call __gcov_clear() if
     * you need to clear the counters.
     *
     * If not actually restarting, and if using malloc,
     * and if you do not call __gcov_exit between calls
     * to this function to free the memory,
     * you will have memory leaks.
     */
    gcov_headGcov = NULL;
#ifndef GCOV_OPT_USE_MALLOC
    gcov_GcovIndex = 0;
#endif

    ctor = &__ctor_list;
    while (ctor != &__ctor_end) {
        void (*func)(void);

        func = (void ( *)(void))(*(UINT32 *)ctor);

        func();
        ctor++;
    }
}
#endif // GCOV_OPT_PROVIDE_CALL_CONSTRUCTORS


/* ----------------------------------------------------------- */
/*
 * __gcov_exit needs to be called in your code at the point
 * where you want to generate coverage data for extraction.
 */
void __gcov_exit(void)
{
    GcovInfo *listptr = gcov_headGcov;

#if defined(GCOV_OPT_OUTPUT_BINARY_FILE) || defined(GCOV_OPT_OUTPUT_BINARY_MEMORY)
    char const *p;
#endif

#ifdef GCOV_OPT_OUTPUT_BINARY_FILE
    unsigned char bf;
    GCOV_FILE_TYPE file;
#endif // GCOV_OPT_OUTPUT_BINARY_FILE

#ifdef GCOV_OPT_OUTPUT_BINARY_MEMORY
    gcov_output_index = 0;
#endif // GCOV_OPT_OUTPUT_BINARY_MEMORY

#ifdef GCOV_OPT_PRINT_STATUS
    GCOV_PRINT_STR("gcov_exit"); GCOV_PRINT_STR("\n");
#endif // GCOV_OPT_PRINT_STATUS

#ifdef GCOV_OPT_OUTPUT_BINARY_FILE
    file = GCOV_OPEN_FILE(GCOV_OUTPUT_BINARY_FILENAME);
    if (GCOV_OPEN_ERROR(file)) {
#ifdef GCOV_OPT_PRINT_STATUS
        GCOV_PRINT_STR("Unable to open gcov output file!"); GCOV_PRINT_STR("\n");
#endif // GCOV_OPT_PRINT_STATUS
#ifdef GCOV_OPT_USE_STDLIB
        exit(1);
#else
        return;
#endif // GCOV_OPT_USE_STDLIB
    }
#endif // GCOV_OPT_OUTPUT_BINARY_FILE

    while (listptr) {
        gcov_unsigned_t *buffer = NULL; // Need buffer to be 32-bit-aligned for type-safe internal usage
        u32 bytesNeeded;

        /* Do pretend conversion to see how many bytes are needed */
        bytesNeeded = gcov_convert_to_gcda(NULL, listptr->info);

#ifdef GCOV_OPT_USE_MALLOC
        buffer = malloc(bytesNeeded);
#else
        if (bytesNeeded > sizeof(gcov_buf)/sizeof(char)) {
            buffer = (gcov_unsigned_t *)NULL;
        } else {
            buffer = gcov_buf;
        }
#endif // GCOV_OPT_USE_MALLOC else

        if (!buffer) {
#ifdef GCOV_OPT_PRINT_STATUS
            GCOV_PRINT_STR("Out of memory!"); GCOV_PRINT_STR("\n");
#endif // GCOV_OPT_PRINT_STATUS
#ifdef GCOV_OPT_USE_STDLIB
            exit(1);
#else
            return;
#endif // GCOV_OPT_USE_STDLIB
        }

        /* Do the real conversion into buffer */
        gcov_convert_to_gcda(buffer, listptr->info);

#if defined(GCOV_OPT_PRINT_STATUS) || defined(GCOV_OPT_OUTPUT_SERIAL_HEXDUMP)
        GCOV_PRINT_STR("Emitting ");
        GCOV_PRINT_NUM(bytesNeeded);
        GCOV_PRINT_STR(" bytes for ");
        GCOV_PRINT_STR(gcov_info_filename(listptr->info));
        GCOV_PRINT_STR("\n");
#endif

#ifdef GCOV_OPT_OUTPUT_BINARY_FILE
        /* write the filename */
        p = gcov_info_filename(listptr->info);
        while (p && (*p)) {
            bf = (*p++);
            (void)GCOV_WRITE_BYTE(file, bf);
        }
        /* add trailing null char */
        bf = '\0';
        (void)GCOV_WRITE_BYTE(file, bf);
        
        /* write the data byte count */
        /* we don't know endianness, so use division for consistent MSB first */
        bf = (unsigned char)(bytesNeeded / 16777216);
        (void)GCOV_WRITE_BYTE(file, bf);
        bf = (unsigned char)(bytesNeeded / 65536);
        (void)GCOV_WRITE_BYTE(file, bf);
        bf = (unsigned char)(bytesNeeded / 256);
        (void)GCOV_WRITE_BYTE(file, bf);
        bf = (unsigned char)(bytesNeeded);
        (void)GCOV_WRITE_BYTE(file, bf);

        /* write the data */
        for (u32 i=0; i<bytesNeeded; i++) {
            bf = (unsigned char)(((unsigned char *)buffer)[i]);
            (void)GCOV_WRITE_BYTE(file, bf);
        }
#endif // GCOV_OPT_OUTPUT_BINARY_FILE

#ifdef GCOV_OPT_OUTPUT_BINARY_MEMORY
        /* copy the filename */
        p = gcov_info_filename(listptr->info);
        while (p && (*p)) {
            gcov_output_buffer[gcov_output_index++] = (*p++);
        }
        /* add trailing null char */
        gcov_output_buffer[gcov_output_index++] = '\0';

        /* store the data byte count */
        /* we don't know endianness, so use division for consistent MSB first */
        gcov_output_buffer[gcov_output_index++] = (unsigned char)(bytesNeeded / 16777216);
        gcov_output_buffer[gcov_output_index++] = (unsigned char)(bytesNeeded / 65536);
        gcov_output_buffer[gcov_output_index++] = (unsigned char)(bytesNeeded / 256);
        gcov_output_buffer[gcov_output_index++] = (unsigned char)(bytesNeeded);

        /* copy the data */
        for (u32 i=0; i<bytesNeeded; i++) {
            gcov_output_buffer[gcov_output_index++] = (unsigned char)(((unsigned char *)buffer)[i]);
        }
#endif // GCOV_OPT_OUTPUT_BINARY_MEMORY

#ifdef GCOV_OPT_OUTPUT_SERIAL_HEXDUMP
        /* If your embedded system does not support printf or an imitation,
         * you'll need to change this code.
         */
        for (u32 i=0; i<bytesNeeded; i++) {
            if (i%16 == 0) GCOV_PRINT_HEXDUMP_ADDR(i);
            GCOV_PRINT_HEXDUMP_DATA((unsigned char)(((unsigned char *)buffer)[i]));
            if (i%16 == 15) GCOV_PRINT_STR("\n");
        }
        GCOV_PRINT_STR("\n");
        GCOV_PRINT_STR(gcov_info_filename(listptr->info));
        GCOV_PRINT_STR("\n");
#endif // GCOV_OPT_OUTPUT_SERIAL_HEXDUMP

/* Other output methods might be imagined,
 * if you have flash that can be written directly,
 * or the luxury of a filesystem, etc.
 */

#ifdef GCOV_OPT_USE_MALLOC
        free(buffer);
#endif // GCOV_OPT_USE_MALLOC

        listptr = listptr->next;
    } /* end while listptr */

    /* Add end marker to output */
#ifdef GCOV_OPT_OUTPUT_BINARY_FILE
    bf = 'G';
    (void)GCOV_WRITE_BYTE(file, bf);
    bf = 'c';
    (void)GCOV_WRITE_BYTE(file, bf);
    bf = 'o';
    (void)GCOV_WRITE_BYTE(file, bf);
    bf = 'v';
    (void)GCOV_WRITE_BYTE(file, bf);
    bf = ' ';
    (void)GCOV_WRITE_BYTE(file, bf);
    bf = 'E';
    (void)GCOV_WRITE_BYTE(file, bf);
    bf = 'n';
    (void)GCOV_WRITE_BYTE(file, bf);
    bf = 'd';
    (void)GCOV_WRITE_BYTE(file, bf);
    bf = '\0';
    (void)GCOV_WRITE_BYTE(file, bf);

    GCOV_CLOSE_FILE(file);
#endif // GCOV_OPT_OUTPUT_BINARY_FILE

#ifdef GCOV_OPT_OUTPUT_BINARY_MEMORY
    gcov_output_buffer[gcov_output_index++] = 'G';
    gcov_output_buffer[gcov_output_index++] = 'c';
    gcov_output_buffer[gcov_output_index++] = 'o';
    gcov_output_buffer[gcov_output_index++] = 'v';
    gcov_output_buffer[gcov_output_index++] = ' ';
    gcov_output_buffer[gcov_output_index++] = 'E';
    gcov_output_buffer[gcov_output_index++] = 'n';
    gcov_output_buffer[gcov_output_index++] = 'd';
    gcov_output_buffer[gcov_output_index++] = '\0';
#endif // GCOV_OPT_OUTPUT_BINARY_MEMORY

#if defined(GCOV_OPT_PRINT_STATUS) || defined(GCOV_OPT_OUTPUT_SERIAL_HEXDUMP)
    GCOV_PRINT_STR("Gcov End");
    GCOV_PRINT_STR("\n");
#endif
}

/* ----------------------------------------------------------- */
#ifdef GCOV_OPT_PROVIDE_CLEAR_COUNTERS
/*
 * __gcov_clear is optional to call if you want to clear the counters,
 * such as after program startup (if you don't want to count the startup)
 * or between test runs.
 * The counters are automatically zero at startup.
 */
void __gcov_clear(void)
{
    GcovInfo *listptr = gcov_headGcov;

#ifdef GCOV_OPT_PRINT_STATUS
    GCOV_PRINT_STR("gcov_clear"); GCOV_PRINT_STR("\n");
#endif // GCOV_OPT_PRINT_STATUS

    while (listptr) {

        gcov_clear_counters(listptr->info);

        listptr = listptr->next;
    }
}
#endif // GCOV_OPT_PROVIDE_CLEAR_COUNTERS

/* ----------------------------------------------------------- */
/*
 * This function should never be called. Merging is not supported.
 * Just providing for the interface, and to warn if someone
 * (including gcc internals) tries to use it.
 */
void __gcov_merge_add(gcov_type *counters, gcov_unsigned_t n_counters)
{
    (void)counters; // ignore unused param
    (void)n_counters; // ignore unused param

#ifdef GCOV_OPT_PRINT_STATUS
    GCOV_PRINT_STR("__gcov_merge_add isn't called, right? Right? RIGHT?");
#endif // GCOV_OPT_PRINT_STATUS

#ifdef GCOV_OPT_USE_STDLIB
    fflush(stdout);
    exit(1);
#else
    return;
#endif // GCOV_OPT_USE_STDLIB
}

/** @}
 */
/*
 * embedded-gcov gcov_public.c gcov application interface code
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
