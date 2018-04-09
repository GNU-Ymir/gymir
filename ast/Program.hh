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
    


    class IProgram  {

	std::vector <Declaration> decls;
	Word locus;

    public:

	IProgram (Word token, std::vector <Declaration> decls) :
	    decls (decls),
	    locus (token)
	{}

	void declare ();	
	void declareAsExtern (std::string, semantic::Module);

	
	
	void print (int nb = 0) {
	    printf ("\n%*c<Program> %s",
		    nb, ' ',
		    this-> locus.toString ().c_str ()
	    );
	    
	    for (auto it : this-> decls)
		it-> print (nb + 4);
	    printf ("\n");
	}	
	
	virtual ~IProgram () {
	    for (auto it : decls)
		delete it;
	}

    private:

	bool verifyMatch (Word & loc, std::string file, std::string mod);
	void detachFile (std::string & file, std::string & path);
	void detachSpace (std::string & file, std::string & path);
	void importAllCoreFiles ();
	void importAllCoreFilesAsExtern (semantic::Module mod);
	
    };

    typedef IProgram* Program;
}
