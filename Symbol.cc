#include "semantic/pack/Symbol.hh"
#include "semantic/types/InfoType.hh"

namespace semantic {

    ISymbol::ISymbol (Word word, InfoType type) :
	_sym (word),
	_type (type)
    {}
    
    bool ISymbol::isConst () {
	return this-> _type-> isConst ();
    }

    bool& ISymbol::isStatic () {
	return this-> _static;
    }
    
    bool& ISymbol::isPublic () {
	return this-> _public;
    }

    bool ISymbol::isScoped () {
	return this-> scoped;
    }

    bool ISymbol::isImmutable () {
	return false;
    }

    void ISymbol::quit (Namespace) {
    }

    std::string ISymbol::typeString () {
	return "";
    }

    Word& ISymbol::sym () {
	return this-> _sym;
    }
    
}
