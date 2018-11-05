/**!
 * \author Emile Cadorel
 * \date 30 october 2018
*/

#pragma once

#include "syntax/_.hh"
#include <stdio.h>
#include <ymir/utils/json.hpp>

/**
 * Main function of the plugin, called by GCC internals
 * 
*/
extern void ymir_parse_files (int num_files, const char ** files);

namespace Ymir {

    /**
     * \struct Parser Parser.hh 
     * This structure is the main part of the compilation
     * 
     */
    struct Parser {
	
	/**
	 * \param filename the name of the current file getting parsed
	 * \param file the file we want to parse
	 */
	Parser (const char * filename, FILE * file);


	/**
	 * \brief Generate the abstract syntax tree of the current file
	 * \return the AST representing the source code
	 */
	syntax::Program syntax_analyse ();

	/**
	 * \brief Semantic analyses of the abstract syntax tree\n
	 *  generate all the symboles, verify the consistency of the program, and type all values \n	 
	 *  Fill the FrameTable informations (create all the required frames for code generation)
	 * \param program the program that will be analyzed	 
	 */
	void semantic_time (syntax::Program program, Ymir::json&);

	/**
	 * \brief Code generation\n
	 * Will get all the validated frames from semantic_time\n
	 * Will call GCC internals code generation
	 */
	void lint_time ();
		
    private:
	
	lexical::Lexer lexer;
	std::string _workingFile;
    };
    
};
