
/*
 * avrdude - A Downloader/Uploader for AVR device programmers
 * Copyright (C) 2003, 2004  Eric B. Weddington <ericw@evcohs.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* $Id: pptest.c,v 1.1 2007/02/18 06:11:30 lady_ada Exp $ */

/*
This is the parallel port interface for Windows built using Cygwin.

In the ppi_* functions that access the parallel port registers,
fd = parallel port address
reg = register as defined in an enum in ppi.h. This must be converted
   to a proper offset of the base address.
*/


#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <windows.h>
#include <sys/time.h>
#include <windows.h>
#include "ppi.h"


#define DEVICE_LPT1 "lpt1"
#define DEVICE_LPT2 "lpt2"
#define DEVICE_LPT3 "lpt3"

#define DEVICE_MAX	3

typedef struct
{
    const char *name;
    int base_address;
} winpp;

static const winpp winports[DEVICE_MAX] = 
{
    {DEVICE_LPT1, 0x378},
    {DEVICE_LPT2, 0x278},
    {DEVICE_LPT3, 0x3BC},
};





/* FUNCTION PROTOTYPES */
static int winnt_pp_open(void);
static unsigned short port_get(int fd, int reg);
static unsigned char reg2offset(int reg);
static unsigned char inb(unsigned short port);
static void outb(unsigned char value, unsigned short port);




int main(int argc, char **argv) {
  int fd;

  printf("hi!");

  fd = ppi_open("lpt1");
  if (fd < 0)
    return;
  while (1) {
    printf("0x%2x\n", ppi_getall(fd, PPISTATUS));
    fflush(stdout);
    sleep(100);
 }
    
}


/* FUNCTION DEFINITIONS */

int ppi_open(char *port)
{
    unsigned char i;
    int fd;
	
    fd = winnt_pp_open();

    if(fd < 0)
    {
        fprintf(stderr, "Can't open device \"giveio\"\n\n");
        return(-1);
    }

    /* Search the windows port names for a match */
    fd = -1;
    for(i = 0; i < DEVICE_MAX; i++)
    {
        if(strcmp(winports[i].name, port) == 0)
        {
            /* Set the file descriptor with the Windows parallel port base address. */
            fd = winports[i].base_address;
            break;
        }
    }
    if(fd < 0)
    {
      fprintf(stderr, "Can't open device \"%s\"\n\n", port);
        return(-1);
    }

    return(fd);
}


#define DRIVERNAME      "\\\\.\\giveio"
static int winnt_pp_open(void)
{
    // Only try to use giveio under Windows NT/2000/XP.
    OSVERSIONINFO ver_info;

    memset(&ver_info, 0, sizeof(ver_info));

    ver_info.dwOSVersionInfoSize = sizeof(ver_info);

    if(!GetVersionEx(&ver_info))
    {
        return(-1);
    }
    else if(ver_info.dwPlatformId == VER_PLATFORM_WIN32_NT) 
    {
        HANDLE h = CreateFile(DRIVERNAME,
            GENERIC_READ,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL);

        if(h == INVALID_HANDLE_VALUE)
        {
            return(-1);
        }

        /* Close immediately. The process now has the rights it needs. */
        if(h != NULL)
        {
            CloseHandle(h);
        }
    }
    return(0);
}




void ppi_close(int fd)
{
    return;
}



/*
 * set the indicated bit of the specified register.
 */
int ppi_set(int fd, int reg, int bit)
{
    unsigned char v;
    unsigned short port;

    port = port_get(fd, reg);
    v = inb(port);
    v |= bit;
    outb(v, port);
    return 0;
}


/*
 * clear the indicated bit of the specified register.
 */
int ppi_clr(int fd, int reg, int bit)
{
    unsigned char v;
    unsigned short port;

    port = port_get(fd, reg);
    v = inb(port);
    v &= ~bit;
    outb(v, port);

    return 0;
}


/*
 * get the indicated bit of the specified register.
 */
int ppi_get(int fd, int reg, int bit)
{
    unsigned char v;

    v = inb(port_get(fd, reg));
    v &= bit;

    return(v);
}




/*
 * toggle the indicated bit of the specified register.
 */
int ppi_toggle(int fd, int reg, int bit)
{
    unsigned char v;
    unsigned short port;

    port = port_get(fd, reg);

    v = inb(port);
    v ^= bit;
    outb(v, port);

    return 0;
}


/*
 * get all bits of the specified register.
 */
int ppi_getall(int fd, int reg)
{
    unsigned char v;

    v = inb(port_get(fd, reg));

    return((int)v);
}




/*
 * set all bits of the specified register to val.
 */
int ppi_setall(int fd, int reg, int val)
{
    outb((unsigned char)val, port_get(fd, reg));
    return 0;
}




/* Calculate port address to access. */
static unsigned short port_get(int fd, int reg)
{
    return((unsigned short)(fd + reg2offset(reg)));
}


/* Convert register enum to offset of base address. */
static unsigned char reg2offset(int reg)
{
    unsigned char offset = 0;

    switch(reg)
    {
        case PPIDATA:
        {
            offset = 0;
            break;
        }
        case PPISTATUS:
        {
            offset = 1;
            break;
        }
        case PPICTRL:
        {
            offset = 2;
            break;
        }
    }

    return(offset);
}


/* Read in value from port. */
static unsigned char inb(unsigned short port)
{
    unsigned char t;
    
	asm volatile ("in %1, %0"
        : "=a" (t)
        : "d" (port));
    
	return t;
}


/* Write value to port. */
static void outb(unsigned char value, unsigned short port)
{
    asm volatile ("out %1, %0"
        :
        : "d" (port), "a" (value) );

    return;
}

void gettimeofday(struct timeval*tv, void*z){
// i've found only ms resolution, avrdude expects us

	SYSTEMTIME st;
	GetSystemTime(&st);
  
	tv->tv_sec=(long)(st.wSecond+st.wMinute*60+st.wHour*3600);
	tv->tv_usec=(long)(st.wMilliseconds*1000);
}

// #define W32USLEEPDBG

#ifdef W32USLEEPDBG

#  define DEBUG_QueryPerformanceCounter(arg) QueryPerformanceCounter(arg)
#  define DEBUG_DisplayTimingInfo(start, stop, freq, us, has_highperf)     \
     do {                                                                  \
       unsigned long dt;                                                   \
       dt = (unsigned long)((stop.QuadPart - start.QuadPart) * 1000 * 1000 \
                            / freq.QuadPart);                              \
       fprintf(stderr,                                                     \
               "hpt:%i usleep usec:%lu sleep msec:%lu timed usec:%lu\n",   \
               has_highperf, us, ((us + 999) / 1000), dt);                 \
     } while (0)

#else

#  define DEBUG_QueryPerformanceCounter(arg)
#  define DEBUG_DisplayTimingInfo(start, stop, freq, us, has_highperf)

#endif

void usleep(unsigned long us)
{
	int has_highperf;
	LARGE_INTEGER freq,start,stop,loopend;

	// workaround: although usleep is very precise if using
	// high-performance-timers there are sometimes problems with
	// verify - increasing the delay helps sometimes but not
	// realiably. There must be some other problem. Maybe just
	// with my test-hardware maybe in the code-base.
	//// us=(unsigned long) (us*1.5);	

	has_highperf=QueryPerformanceFrequency(&freq);

	//has_highperf=0; // debug

	if (has_highperf) {
		QueryPerformanceCounter(&start);
		loopend.QuadPart=start.QuadPart+freq.QuadPart*us/(1000*1000);
		do { 
			QueryPerformanceCounter(&stop);
		} while (stop.QuadPart<=loopend.QuadPart);
	}
	else {
		DEBUG_QueryPerformanceCounter(&start);

		Sleep(1);
		Sleep( (DWORD)((us+999)/1000) );

		DEBUG_QueryPerformanceCounter(&stop);
	}
	
    DEBUG_DisplayTimingInfo(start, stop, freq, us, has_highperf);
}
