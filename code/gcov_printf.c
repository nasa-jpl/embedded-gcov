/**********************************************************************/
/** @addtogroup gcov
 * @{
 * @file
 * @version $Id: gcov_printf.c 302 2015-09-14 23:41:57Z scclancy $
 * @author 2008-10-30 cyamamot
 * @author 2021-08-24 kjpeters
 * @author 2022-01-03 kjpeters Adjust character output for portability.
 *
 * Provide small imitation printf function.
 * This is only needed if you want serial port outputs and
 * do not have already-existing functions to do the printing.
 * If you select this, then you need to provide a function
 * write_bytes() that does the actual serial output in your system.
 *
 * @brief Formatted printing utility functions
 **********************************************************************/

#include "gcov_public.h"

#ifdef GCOV_OPT_PROVIDE_PRINTF_IMITATION

#include <stdarg.h>

/* Include any files you need to get your write_bytes() function */
/* that does the actual serial output in your system. */
/*
 * INT32
 * write_bytes(INT32 fd,
 *       CHAR_T const *buf,
 *       UINT32 nbyte);
 * where
 * @param [in]     fd (which serial port)
 * @param [in,out] *buf (bytes to send out)
 * @param [in]     nbyte (number of bytes to send)
 *
 * @return -1 if error, otherwise returns byte count written to UART
 */
#include <stdio.h>
#define write_bytes(fd, buf, n) putchar((int)(*(buf)))

/***********************************************************************
 * The following functions support gcov_printf and are not meant to be
 * called otherwise.
 **********************************************************************/
static void gcov_uli2a(unsigned long int num, unsigned int base, int uc,char * bf);
static void gcov_uli2a(unsigned long int num, unsigned int base, int uc,char * bf)
{
	int n=0;
	unsigned int d=1;
	while (num/d >= base)
		d*=base;
	while (d!=0) {
		int dgt = num / d;
		num%=d;
		d/=base;
		if (n || dgt>0|| d==0) {
			*bf++ = dgt+(dgt<10 ? '0' : (uc ? 'A' : 'a')-10);
			++n;
		}
	}
	*bf=0;
}

static void gcov_li2a (long num, char * bf);
static void gcov_li2a (long num, char * bf)
{
	if (num<0) {
		num=-num;
		*bf++ = '-';
	}
	gcov_uli2a(num,10,0,bf);
}

static void gcov_ui2a(unsigned int num, unsigned int base, int uc,char * bf);
static void gcov_ui2a(unsigned int num, unsigned int base, int uc,char * bf)
{
	int n=0;
	unsigned int d=1;
	while (num/d >= base)
		d*=base;
	while (d!=0) {
		int dgt = num / d;
		num%= d;
		d/=base;
		if (n || dgt>0 || d==0) {
			*bf++ = dgt+(dgt<10 ? '0' : (uc ? 'A' : 'a')-10);
			++n;
		}
	}
	*bf=0;
}

static void gcov_i2a (int num, char * bf);
static void gcov_i2a (int num, char * bf)
{
	if (num<0) {
		num=-num;
		*bf++ = '-';
	}
	gcov_ui2a(num,10,0,bf);
}

static int gcov_a2d(char ch);
static int gcov_a2d(char ch)
{
	if (ch>='0' && ch<='9')
		return ch-'0';
	else if (ch>='a' && ch<='f')
		return ch-'a'+10;
	else if (ch>='A' && ch<='F')
		return ch-'A'+10;
	else return -1;
}

static char gcov_a2i(char ch, const char* src,int base,int* nump,int *numc);
static char gcov_a2i(char ch, const char* src,int base,int* nump,int *numc)
{
	const char* p= src;
	int n=0;
	int num=0;
	int digit;
	while ((digit=gcov_a2d(ch))>=0) {
		if (digit>base) break;
		num=num*base+digit;
		ch=*p++;
		n++;
	}
	*nump=num;
	*numc=n;
	return ch;
}

static void gcov_putchw(int n, char z, char* bf);
static void gcov_putchw(int n, char z, char* bf)
{
	char fc=z? '0' : ' ';
	char ch;
	char* p=bf;
	while (*p++ && n > 0)
		n--;
	while (n-- > 0)
		write_bytes(1,&fc,1);
	while ((ch= *bf++))
		write_bytes(1,&ch,1);
}



/**********************************************************************/
/** @brief
 *
 * @author 2008-10-30 cyamamot
 * @author 2021-08-24 kjpeters
 * @author 2022-01-03 kjpeters Adjust character output for portability.
 *
 * @param [in]     *fmt
 * @param [in]     va
 *
 * @note This is a supporting function for \ref gcov_printf gcov_printf
 * and is not meant to be called otherwise.
 **********************************************************************/
static void gcov_format(const char *fmt, va_list va);
static void gcov_format(const char *fmt, va_list va)
{
	// 10/3/2017 m. chase - Added abort flag to remove the need of goto

	char bf[12];
	char ch;
	char c2;
	char abort_flg = 0; // make nonzero to abort

	while ((ch=*(fmt++)) && (!abort_flg)) {
		if (ch!='%')
			write_bytes(1,&ch,1);
		else {
			char lz=0;
			char lng=0;
			int w=0;
			int n=0;
			ch=*(fmt++);
			if (ch=='0') {
				ch=*(fmt++);
				lz=1;
			}
			if (ch>='0' && ch<='9') {
				ch=gcov_a2i(ch,fmt,10,&w,&n);
				fmt+=n;
			}
			if (ch=='l') {
				ch=*(fmt++);
				lng=1;
			}
			if(ch==0) {
				abort_flg = 1;
			}
			else {
				switch (ch) {
				case 'u' : {
					if (lng)
						gcov_uli2a(va_arg(va, unsigned long int),10,0,bf);
					else
						gcov_ui2a(va_arg(va, unsigned int),10,0,bf);
					gcov_putchw(w,lz,bf);
					break;
				}
				case 'd' :  {
					if (lng)
						gcov_li2a(va_arg(va, unsigned long int),bf);
					else
						gcov_i2a(va_arg(va, int),bf);
					gcov_putchw(w,lz,bf);
					break;
				}
				case 'x': case 'X' :
					if (lng)
						gcov_uli2a(va_arg(va, unsigned long int),16,(ch=='X'),bf);
					else
						gcov_ui2a(va_arg(va, unsigned int),16,(ch=='X'),bf);
					gcov_putchw(w,lz,bf);
					break;
				case 'c' :
					c2 = (char)(va_arg(va, int));
					write_bytes(1,&c2,1);
					break;
				case 's' :
					gcov_putchw(w,0,va_arg(va, char*));
					break;
				case '%' :
					write_bytes(1,&ch,1);
				default:
					break;
				}
			}
		}
	}
}



/**********************************************************************/
/** @brief Simplistic printf() function, floating point not supported
 *
 * @author 2008-10-30 cyamamot
 * @author 2021-08-24 kjpeters
 *
 * @param [in]     *fmt
 * @param [in]     ... variable length argument list
 *
 **********************************************************************/
void gcov_printf(const char *fmt, ...)
{
	va_list va;
	va_start(va,fmt);
	gcov_format(fmt,va);
	va_end(va);
}

#endif // GCOV_OPT_PROVIDE_PRINTF_IMITATION


/** @}
 */
/*
 * embedded-gcov gcov_printf.c gcov small printf imitation if needed
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
