#include "FunctionInfo.hh"

#include "Error.hh"

namespace Semantic {

    FunctionInfo::FunctionInfo (const std::string & name)
	: name (name)
    {}

    TypeInfo * FunctionInfo::binaryOp (Lexical::TokenPtr, Syntax::Expression*) {
	return NULL;
    }
    
    TypeInfo * FunctionInfo::binaryOpRight (Lexical::TokenPtr, Syntax::Expression*) {
	return NULL;
    }
    
    TypeInfo * FunctionInfo::unaryOp (Lexical::TokenPtr) {
	return NULL;
    }
    
    TypeInfo * FunctionInfo::multOp (Lexical::TokenPtr, Syntax::Expression*) {
	return NULL;
    }

    TypeInfo * FunctionInfo::clone () {
	return this;
    }
    
    bool FunctionInfo::Is (TypeEnum type) {
	return type == FUNCTION;
    };

    std::string FunctionInfo::typeToString () const {
	return "Function";
    }
    
    void FunctionInfo::insert (Frame fr) {
	this->frames.push_back (fr);
    }
    

    Ymir::Tree FunctionInfo::toGeneric () {
	Ymir::Error::fatal (UNKNOWN_LOCATION, "Erreur interne ");
	return Ymir::Tree ();
    }
    


}
