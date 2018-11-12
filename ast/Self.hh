#pragma once

#include "Function.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    /**
     * \struct ISelf
     * The syntaxic node representation of a module constructor
     * \verbatim
     self := 'self' '(' ')' block
     \endverbatim
     */
    class ISelf : public IFunction {
    public:

	ISelf (Word ident, const std::string & docs, Block block) :
	    IFunction (ident, docs, {}, {}, {}, NULL, block)
	{}

	Ymir::json generateDocs () override;
	
	void declare () override;

	void declare (semantic::Module) override;

	void declareAsExtern (semantic::Module) override;
	
	void print (int) override {
	}	
		
    };

    
    /**
     * \struct ISelf
     * The syntaxic node representation of a module destructor
     * \verbatim
     dself := '~' 'self' '(' ')' block
     \endverbatim
     */
    class IDestSelf : public IFunction {
    public:

	IDestSelf (Word ident, const std::string & docs, Block block) :
	    IFunction ({ident, "~" + ident.getStr ()}, docs, {}, {}, {}, NULL, block)
	{}

	Ymir::json generateDocs () override;
	
	void declare () override;

	void declare (semantic::Module) override;

	void declareAsExtern (semantic::Module) override;
	
	void print (int) override {
	}	
		
    };

    typedef IDestSelf* DestSelf;
    typedef ISelf* Self;
    
}
