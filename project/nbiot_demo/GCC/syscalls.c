#include <sys/stat.h>
#include <system.h>
#include <exsystem.h>
#if defined(MTK_PORT_SERVICE_ENABLE)
#include "serial_port.h"
#endif
#include "FreeRTOS.h"
#include "task.h"
#include <sys/time.h>
#include "portmacro.h"

#if defined(MTK_PORT_SERVICE_ENABLE)
serial_port_write_data_t __io_putchar_send_data;
extern serial_port_handle_t emmiPortHandle;
#endif

extern int __io_putchar(int ch) __attribute__((weak));
extern int __io_getchar(void) __attribute__((weak));

/*Used for the system heap memory*/
#define HEAP_MEMORY_SIZE   8192
ATTR_NONINIT_DATA_IN_RAM static char xHeapMemory[HEAP_MEMORY_SIZE];
static char *pxHeapMemory = &xHeapMemory[HEAP_MEMORY_SIZE - 1];

int _close(int file)
{
    return 0;
}

int _fstat(int file, struct stat *st)
{
    return 0;
}

int _isatty(int file)
{
    return 1;
}

int _lseek(int file, int ptr, int dir)
{
    return 0;
}

int _open(const char *name, int flags, int mode)
{
    return -1;
}

int _read(int file, char *ptr, int len)
{
    return 0;
    int DataIdx;

    for (DataIdx = 0; DataIdx < len; DataIdx++) {
        __io_putchar('+');
        *ptr++ = __io_getchar();
        __io_putchar('-');
    }

    return len;
}

/*
 * * getpid -- only one process, so just return 1.
 * */
#define __MYPID 1
int _getpid()
{
    return __MYPID;
}

/*
 * * kill -- go out via exit...
 * */
int _kill(int pid, int sig)
{
    return -1;
}

#ifndef RT_USING_NEWLIB
int _exit(int val)
{
    return (-1);
}

caddr_t _sbrk_r(struct _reent *r, int incr)
{
    static char *heap_end;
    char *prev_heap_end;
    
    if(heap_end == NULL)
    {
        /*initialise the heap address*/
        heap_end = (char *)&xHeapMemory;
    }
    
    prev_heap_end = heap_end;

    /*check whether the remaining memory can satisfy this allocate action*/
    if(heap_end + incr > pxHeapMemory)
    {
        /*have no enough memory to allocate
                   generate an assert fail error */
        DevParam(heap_end, incr, pxHeapMemory);
        return (caddr_t) - 1;
    }
    /*update the current heap end address*/
    heap_end += incr;
    return (caddr_t)prev_heap_end;
}

#endif

int _write(int file, char *ptr, int len)
{
#if defined(MTK_PORT_SERVICE_ENABLE)
    if (emmiPortHandle != (serial_port_handle_t)NULL)
    {
        if(((portNVIC_INT_CTRL_REG & 0xFFL) < 16) && ((portNVIC_INT_CTRL_REG & 0xFFL) != 0))
        {
            __io_putchar_send_data.data = (uint8_t*)ptr;
            __io_putchar_send_data.size = len;
            serial_port_control(emmiPortHandle, SERIAL_PORT_CMD_WRITE_DATA_BLOCKING, (serial_port_ctrl_para_t*)&__io_putchar_send_data.data);
            return __io_putchar_send_data.ret_size;
        }
        else
        {
            return -1;
        }
    }
#endif

    int DataIdx;
    for (DataIdx = 0; DataIdx < len; DataIdx++) {
        __io_putchar(*ptr++);
    }
    return len;
}

int _gettimeofday(struct timeval *tv, void *ptz)
{
    int ticks = xTaskGetTickCount();
    if (tv != NULL) {
        tv->tv_sec = ((long)ticks) * portTICK_RATE_MS / 1000;
        tv->tv_usec = (((long)ticks * portTICK_RATE_MS) % 1000) * 1000;
        return 0;
    }

    return -1;
}
