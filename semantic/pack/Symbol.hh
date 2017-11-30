#pragma once

#include <gc/gc_cpp.h>
#include "../../syntax/Word.hh"
#include "Namespace.hh"

namespace semantic {

    class IInfoType;
    typedef IInfoType* InfoType;

    class IValue;
    typedef IValue* Value;
    
    class ISymbol : public gc {

	bool scoped;
	bool _public;
	bool _static;
	
    public:

	Word sym;
	InfoType type;
	Value value;
	
	ISymbol (Word word, InfoType);
	
	bool isConst ();

	bool& isPublic ();

	bool& isStatic ();

	bool isType ();

	bool isScoped ();

	bool isImmutable ();

	void quit (Namespace space);

	std::string typeString ();

	std::string simpleTypeString ();
	
    };
    
    typedef ISymbol* Symbol;
    
}
