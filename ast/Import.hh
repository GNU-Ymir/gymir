#pragma once

#include "Declaration.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"
#include <ymir/utils/Array.hh>

namespace syntax {

    /**
     * \struct IImport
     * The syntaxic node representation of an import
     * \verbatim
     import := 'import' module (',' module)* ';'
     module := Identifier ('.' Identifier)* ('.' '_')?
     \endverbatim
    */
    class IImport : public IDeclaration {

	/** the location */
	Word _ident;

	/** The module imported */
	std::vector <Word> _params;
	
    public:
	
	/**
	 * \param ident the location 
	 * \param params the module that has to be imported
	 */
	IImport (Word ident, std::vector <Word> params) :
	    IDeclaration (""),
	    _ident (ident),
	    _params (params)
	{}
      	
	void declare () override;

	void declare (semantic::Module) override;
	
	void declareAsExtern (semantic::Module) override;

	Declaration templateDeclReplace (const std::map <std::string, Expression>&) override;
	
	void print (int nb = 0) override {
	    printf ("\n%*c<Import> %s",
		    nb, ' ',
		    this-> _ident.toString ().c_str ()
	    );
	    for (auto it : this-> _params) {
		printf ("%s", it.toString ().c_str ());
	    }
	}
	
    private:

	/**
	 * \return the first existing path to the module over all the include directories 
	 */
	std::string firstExistingPath (std::string file);

	/**
	 * \brief declare the sub module imported\n
	 * Example : 
	 * \verbatim
	 import std.algorithm._; 

	 // Declared Modules : 
	
	 mod std {
	     mod algorithm {
	         mod comparison {}
		     mod searching {} 
		 ...
	     }
	 }
	
	 \endverbatim
	*/
	semantic::Symbol createAndDeclareSubMod (semantic::Module& last, semantic::Namespace space, int current);
	
    };

    typedef IImport* Import;
}
