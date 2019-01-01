#include <ymir/errors/Exception.hh>

const char *exc_file;
const char *exc_function;
unsigned exc_line;
volatile int exc_code;
volatile unsigned exc_tries;

/* This is the global stack of catchers. */
struct exc_stack *exc_global;

void exc_print (FILE *stream, const char *file, const char *function, unsigned line,
		   int code)
{
    fprintf (stream, "Exception in file \"%s\", at line %u",
	     file, line);
    
    if (function)
	{
	    fprintf (stream, ", in function \"%s\"", function);
	}
    fprintf (stream, ".");
    
#ifdef EXC_PRINT
    fprintf (stream, " Exception ");
    EXC_PRINT (code, stream);
#endif
    fprintf (stream, "\n");
}

int exc_push (jmp_buf *j, int returned) {
    static exc_stack *head;
    if (returned != 0) { // Le jmp a déjà été déclaré, on est revenu dessus suite à un throw
	return 0;	
    }

    ++ exc_tries;
    /* Using memcpy here is the best alternative. */
    head = new exc_stack ();
    memcpy (&head->j, j, sizeof (jmp_buf));
    head->num = exc_tries;
    head->prev = exc_global;
    exc_global = head;

    return 1;     
}

void exc_pop (jmp_buf *j) {
    struct exc_stack *stored = exc_global;

    if (stored == NULL)
	{	    
	    fprintf (stderr, "Unhandled exception\n");
	    exc_print (stderr, exc_file, exc_function,
		       exc_line, exc_code);
	    
	    raise (SIGABRT);
	}
    
    exc_global = stored->prev;
    
    if (j)
	{
	    /* This assumes that JMP_BUF is a structure etc. and can be
	       copied rawely.  This is true in GLIBC, as I know. */
	    memcpy (j, &stored->j, sizeof (jmp_buf));
	}
        
    /* While with MALLOC, free.  When using obstacks it is better not to
       free and hold up. */
    free (stored);
}

void exc_throw (const char *file, const char *function, unsigned line, int code) {
    jmp_buf j;

    exc_file = file;
    exc_function = function;
    exc_line = line;

    /* Pop for jumping. */
    exc_pop (&j);
    exc_code = code;
   
    /* LONGJUMP to J with nonzero value. */
    longjmp (j, 1);
}

void exc_rethrow () {
    jmp_buf j;
    exc_pop (&j);
   
    /* LONGJUMP to J with nonzero value. */
    longjmp (j, 1);
}

bool exc_check_error (int code) {
    if (code == exc_code) {
	return true;
    }
    return false;
}
