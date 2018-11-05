#pragma once

#include "Function.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class ISelf : public IFunction {

	//Word ident;

    public:

	ISelf (Word ident, std::string & docs, Block block) :
	    IFunction (ident, docs, {}, {}, {}, NULL, block)
	{}

	Ymir::json generateDocs () override;
	
	void declare () override;

	void declare (semantic::Module) override;

	void declareAsExtern (semantic::Module) override;
	
	void print (int) override {
	}	
		
    };

    class IDestSelf : public IFunction {

	//Word ident;

    public:

	IDestSelf (Word ident, std::string & docs, Block block) :
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
