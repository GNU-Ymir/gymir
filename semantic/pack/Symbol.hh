#pragma once

#include <gc/gc_cpp.h>
#include "../../syntax/Word.hh"
#include "Namespace.hh"

namespace semantic {

    class ISymbol : public gc {

	bool scoped;
	bool _public;
	Word _sym;
	// InfoType type;
	bool _static;

    public:

	ISymbol (Word word /*, InfoType */);
	
	//bool isConst ();

	bool& isPublic ();

	bool& isStatic ();

	bool isType ();

	bool isScoped ();

	bool isImmutable ();

	void quit (Namespace space);

	std::string typeString ();

	Word& sym ();
	
    };
    
    typedef ISymbol* Symbol;
    
}
