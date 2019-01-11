#include <ymir/errors/Exception.hh>
#include <ymir/errors/Error.hh>

const char *exc_file;
const char *exc_function;
unsigned exc_line;
volatile int exc_code;
volatile unsigned exc_tries;

/* This is the global stack of catchers. */
struct exc_stack *exc_global;

std::vector <std::string> errors;

void excPrint (FILE *stream, const char *file, const char *function, unsigned line,
	       int)
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

int excPush (jmp_buf *j, int returned, int line, const char * file) {
    static exc_stack *head;
    println ("Pushing : ", file, ":", line);
    if (returned != 0) { // The jmp buffer has already been declared, we are comming back there due to a throw
	return 0;	
    }

    ++ exc_tries;
    /* Using memcpy here is the best alternative. */
    head = new exc_stack ();
    memcpy (&head->j, j, sizeof (jmp_buf));
    head->num = exc_tries;
    head-> file = file;
    head-> line = line;
    head->prev = exc_global;
    exc_global = head;

    return 1;     
}

void excPop (jmp_buf *j) {
    struct exc_stack *stored = exc_global;
    if (stored == NULL)
	{	    
	    fprintf (stderr, "Unhandled exception\n");
	    excPrint (stderr, exc_file, exc_function,
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
    
    println ("Poping : ", stored-> file, ":", stored-> line);
    /* While with MALLOC, free.  When using obstacks it is better not to
       free and hold up. */
    free (stored);
}

void excThrow (const char *file, const char *function, unsigned line, int code, const std::string& msg) {
    jmp_buf j;

    exc_file = file;
    exc_function = function;
    exc_line = line;

    /* Pop for jumping. */
    excPop (&j);
    exc_code = code;
    errors.push_back (msg);
    
    /* LONGJUMP to J with nonzero value. */
    longjmp (j, 1);
}

void excRethrow () {
    jmp_buf j;
    excPop (&j);

    /* LONGJUMP to J with nonzero value. */
    longjmp (j, 1);

}

namespace Ymir {
    void bt_print ();
}

bool excCheckError (int code) {
    if (code == exc_code) {
	exc_code = 1;
	return true;
    }
    return false;
}

void printErrors () {
    for (auto it : errors) {
	fprintf (stderr, "%s\n", it.c_str ());
    }
    errors.clear ();
}

void clearErrors () {
    exc_code = -1;
    errors.clear ();
}
