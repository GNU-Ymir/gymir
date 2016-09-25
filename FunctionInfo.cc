#include "FunctionInfo.hh"

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
    
    bool FunctionInfo::Is (TypeEnum type) {
	return type == FUNCTION;
    };

    std::string FunctionInfo::typeToString () const {
	return "Function";
    }
    
    void FunctionInfo::insert (Frame fr) {
	this->frames.push_back (fr);
    }
    
    
}
