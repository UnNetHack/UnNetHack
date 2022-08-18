/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* to get the malloc() prototype from system.h */
#define ALLOC_C     /* comment line for pre-compiled headers */
/* since this file is also used in auxiliary programs, don't include all the
 * function declarations for all of nethack
 */
#define EXTERN_H    /* comment line for pre-compiled headers */
#include "config.h"

#if defined(MONITOR_HEAP) || defined(WIZARD)
char *fmt_ptr(const genericptr);
#endif

#ifdef MONITOR_HEAP
#undef alloc
#undef free
extern void free(genericptr_t);
static void heapmon_init();

static FILE *heaplog = 0;
static boolean tried_heaplog = FALSE;
#endif

long *alloc(unsigned int);
extern void VDECL(panic, (const char *, ...)) PRINTF_F(1, 2);


long *
alloc(register unsigned int lth)
{
    void *ptr;

    ptr = calloc(lth, 1);
#ifndef MONITOR_HEAP
    if (!ptr) panic("Memory allocation failure; cannot get %u bytes", lth);
#endif
    return ptr;
}


#ifdef HAS_PTR_FMT
#define PTR_FMT "%p"
#define PTR_TYP genericptr_t
#else
#define PTR_FMT "%06lx"
#define PTR_TYP unsigned long
#endif

/* A small pool of static formatting buffers.
 * PTRBUFSIZ:  We assume that pointers will be formatted as integers in
 * hexadecimal, requiring at least 16+1 characters for each buffer to handle
 * 64-bit systems, but the standard doesn't mandate that encoding and an
 * implementation could do something different for %p, so we make some
 * extra room.
 * PTRBUFCNT:  Number of formatted values which can be in use at the same
 * time.  To have more, callers need to make copies of them as they go.
 */
#define PTRBUFCNT 4
#define PTRBUFSIZ 32
static char ptrbuf[PTRBUFCNT][PTRBUFSIZ];
static int ptrbufidx = 0;

/* format a pointer for display purposes; returns a static buffer */
char *
fmt_ptr(const void *ptr)
{
    char *buf;

    buf = ptrbuf[ptrbufidx];
    if (++ptrbufidx >= PTRBUFCNT) {
        ptrbufidx = 0;
    }

    Sprintf(buf, PTR_FMT, (PTR_TYP) ptr);
    return buf;
}

#ifdef MONITOR_HEAP

/* If ${NH_HEAPLOG} is defined and we can create a file by that name,
   then we'll log the allocation and release information to that file. */
static void
heapmon_init()
{
    char *logname = getenv("NH_HEAPLOG");

    if (logname && *logname)
        heaplog = fopen(logname, "w");
    tried_heaplog = TRUE;
}

long *
nhalloc(lth, file, line)
unsigned int lth;
const char *file;
int line;
{
    long *ptr = alloc(lth);
    char ptr_address[20];

    if (!tried_heaplog) heapmon_init();
    if (heaplog)
        (void) fprintf(heaplog, "+%5u %s %4d %s\n", lth,
                       fmt_ptr((genericptr_t)ptr, ptr_address),
                       line, file);
    /* potential panic in alloc() was deferred til here */
    if (!ptr) panic("Cannot get %u bytes, line %d of %s",
                    lth, line, file);

    return ptr;
}

void
nhfree(ptr, file, line)
genericptr_t ptr;
const char *file;
int line;
{
    char ptr_address[20];

    if (!tried_heaplog) heapmon_init();
    if (heaplog)
        (void) fprintf(heaplog, "-      %s %4d %s\n",
                       fmt_ptr((genericptr_t)ptr, ptr_address),
                       line, file);

    free(ptr);
}

#endif /* MONITOR_HEAP */

/* strdup() which uses our alloc() rather than libc's malloc();
   not used when MONITOR_HEAP is enabled, but included unconditionally
   in case utility programs get built using a different setting for that */
char *
dupstr(const char *string)
{
    return strcpy((char *) alloc(strlen(string) + 1), string);
}

/*alloc.c*/
