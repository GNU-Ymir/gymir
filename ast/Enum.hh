#pragma once

#include "Declaration.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"
#include <ymir/utils/Array.hh>

namespace syntax {

    class IEnum : public IDeclaration {
	Word ident;
	Var type;
	std::vector <Word> names;
	std::vector <Expression> values;

    public:

	IEnum (Word ident, Var type, std::vector <Word> names, std::vector <Expression> values);

	void declare () override;	

	void declareAsExtern (semantic::Module mod) override;
	
	void print (int nb = 0) override;

	virtual ~IEnum ();
	
    };

    typedef IEnum* Enum;
    
}
