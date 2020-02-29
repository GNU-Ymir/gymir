#pragma once

#include <ymir/syntax/Declaration.hh>
#include <string>

/**
 * \brief Main function called by GCC internals
 * \brief Will procude a module for each file and perform syntax and semantic analyses
 * \brief Once semantic analyse is done, if no error occurs it will produce a gimple gimple target code for each file
 * \param nb_files the number of file in input
 * \param files the list of file names
 */
extern void ymir_parse_files (int nb_files, const char ** files);


namespace Ymir {

    /**
     * \struct Parser 
     * This structure is the main part of the compilation 
     * It will perform all the analyses and code production for a given file
     */
    struct Parser {

	/**
	 * \param filename the path of the file to parse
	 */
	Parser (const char * filename); 

	/**
	 * Read the version file
	 * This file is used to activate version (__version keyword)
	 */
	static void readVersionFile ();
	
	/**
	 * \brief Run all the phase of the parsing
	 */
	void run ();
	
	/**
	 * \brief Generate the abstract syntax tree of the current file
	 * \return the AST representing the source code
	 */
	void syntaxicTime ();

	/**
	 * \brief Semantic analyses of the AST 
	 * \brief Generate all the symbols and verify the consistency of the program
	 * \brief Produce the frames and the global variables that will be used during code generation 
	 * \param module the module that will be analysed
	 */
	void semanticTime ();

	/**
	 * \brief (lint is for Intermediate Language)
	 * \brief This method generate the code and call gcc internals to produce target	 
	 */
	void lintTime ();

	~Parser ();	
	
    private :

	/// The path of the file parsed 
	std::string _path;

	/// The module produce by syntaxic analyse
	syntax::Declaration _module;
			
    };
    
}
