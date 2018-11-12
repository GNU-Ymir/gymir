#pragma once

#include "Declaration.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"
#include <ymir/utils/Array.hh>

namespace semantic {
    class IModule;
    typedef IModule* Module;
}

namespace syntax {
    
    /**
     * \struct IProgram
     * The root of an abstract syntax tree
     */
    class IProgram  {

	/** The list of declaration inside the program */	
	std::vector <Declaration> _decls;

	/** The location of the file (filename) */
	Word _locus;

    public:

	/** 
	 * \param token The location of the file (filename)
	 * \param decls the declaration inside the program
	 */
	IProgram (Word token, std::vector <Declaration> decls) :
	    _decls (decls),
	    _locus (token)
	{}

	void declare ();	
	void declareAsExtern (std::string, semantic::Module);
	
	
	void print (int nb = 0) {
	    printf ("\n%*c<Program> %s",
		    nb, ' ',
		    this-> _locus.toString ().c_str ()
	    );
	    
	    for (auto it : this-> _decls)
		it-> print (nb + 4);
	    printf ("\n");
	}	

	Ymir::json generateDocs ();
	
	virtual ~IProgram () {
	    for (auto it : this-> _decls)
		delete it;
	}

    private:

	bool verifyMatch (const Word & loc, std::string file, std::string mod);
	void detachFile (std::string & file, std::string & path);
	void detachSpace (std::string & file, std::string & path);
	void importAllCoreFiles ();
	void importAllCoreFilesAsExtern (semantic::Module mod);
	
    };

    typedef IProgram* Program;
}
