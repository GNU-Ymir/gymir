#include "semantic/pack/Symbol.hh"

namespace semantic {

    ISymbol::ISymbol (Word word) :
	_sym (word)
    {}
    
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

    void ISymbol::quit (Namespace space) {
    }

    std::string ISymbol::typeString () {
	return "";
    }

    Word& ISymbol::sym () {
	return this-> _sym;
    }
    
}
