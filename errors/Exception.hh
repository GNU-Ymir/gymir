#pragma once
#include <signal.h>
#include <setjmp.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


/* Stack is actually a linked list of catcher cells. */
struct exc_stack
{
    unsigned num;
    jmp_buf j;
    struct exc_stack *prev;
};


void exc_print (FILE *stream, const char *file, const char *function, unsigned line,
		int code);

int exc_push (jmp_buf *j, int returned);

void exc_pop (jmp_buf *j);

void exc_throw (const char *file, const char *function, unsigned line, int code);

void exc_rethrow ();

bool exc_check_error (int code);

#define TRY					\
    jmp_buf buf;				\
    int res = setjmp (buf);			\
    if (exc_push (&buf, res)) 			

#define CATCH(TYPE)				\
    else if (exc_check_error (TYPE))		\
	

#define CATCH_ALL				\
    else

#define FINALLY					\
    else exc_rethrow ()

#define THROW(code)				\
    exc_throw(__FILE__, __FUNCTION__, __LINE__, code)
