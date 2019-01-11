#pragma once
#include <signal.h>
#include <setjmp.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>

/**
 * \file Exception.hh
 * GCC does not allow to use tti, but exception are usefull
 * This file implements a simple version of exception handling and throwing
 * implemented in errors/Exception.cc
 */

/* Stack is actually a linked list of catcher cells. */
struct exc_stack
{
    unsigned num;
    int line;
    const char * file;
    jmp_buf j;
    struct exc_stack *prev;
};

/**
 * \brief Print the error in case of non catch excepetion
 * \param stream the stream where to put the error (stderr for example)
 * \param file the filename of the source file
 * \param function the name of the function that throws the exception
 * \param line the line where the exception was thrown
 * \param code the kind of error thrown
 */
void excPrint (FILE *stream, const char *file, const char *function, unsigned line,
	       int code);

/**
 * \brief enter en new try catch block
 * \brief Use TRY {} FINALLY; block for simplication
 */
int excPush (jmp_buf *j, int returned, int line, const char * file);

/**
 * \brief Depop the list of try catch block and jump to the first one
 * \brief If no try block was declared call excPrint and raise an abort
 */
void excPop (jmp_buf *j);

/**
 * \brief Throw a new exception
 * \param file the file where the exception is located (source file)
 * \param function the function that throws the exception 
 * \param line the line of the exception 
 * \param code the type of exception
 * \param msg the message of the exception
 * \brief use THROW macro for simplification
*/
void excThrow (const char *file, const char *function, unsigned line, int code, const std::string& msg);

/**
 * \brief Rethrow a previous exception
 * \brief It used in FINALLY, to rethrow if none of the catching have successfully catch this exception
 */
void excRethrow ();

/**
 * \return true if the current exception is typed of code
 */
bool excCheckError (int code);

/**
 * \brief Print all the errors since the last print
 */
void printErrors ();

/**
 * \brief Clear all the errors since the last clear
 */
void clearErrors ();

/**
 * \return the last error in the list of errors
 */
std::string& getLastError ();

/**
 * Define a try block 
 * \verbatim
 TRY {
 foo ();
 } FINALLY;
 \endverbatim
*/
#define TRY								\
    jmp_buf buf;							\
    int res = setjmp (buf);						\
    if (excPush (&buf, res, __LINE__, __FILE__))			\
	for (int END = 1 ; END == 1 ; END = 0, excPop (&buf))		\
						
					       
/**
 * Define a catch of a kind of exception
 * \verbatim
 TRY {
     foo ();
 } CATCH (ERROR) {
    caught ();
 } FINALLY;
 \endverbatim
*/
#define CATCH(TYPE)					\
    else if (excCheckError ((int) TYPE))		\

	

/**
 * Define a catch of any kind of exception
 * \verbatim
 TRY {
     foo ();
 } CATCH_ALL {
    caught_any ();
 } FINALLY;
 \endverbatim
*/
#define CATCH_ALL				\
    else

/**
 * Place at the end of a try block (mandatory)
 * Will rethrow uncaught exception
 * In the case where nothing happend, we remove the jmp_buf from the list
 */
#define FINALLY								\
    else { excRethrow (); }						\

/**
 * Throw a new expression 
 * \brief This will quit the current frame and jump to the first valid catcher
 * \param code the kind of exception
 * \param msg the message of the exception
 */
#define THROW(code, msg)					\
    excThrow(__FILE__, __FUNCTION__, __LINE__, code, msg)

/**
 * \brief Print the messages of all the thrown exceptions
 */
#define PRINT_ERRORS()				\
    printErrors ();

/**
 * \brief Clear all the error message that append since the last clear
 */
#define CLEAR_ERRORS()				\
    clearErrors ();
