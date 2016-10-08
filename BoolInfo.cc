#include "BoolInfo.hh"

namespace Semantic {

    BoolInfo::_init BoolInfo::_initializer;

    BoolInfo::BoolInfo () {}

    BoolInfo::BoolInfo (TypeInfo::BinopLint lint) {
	this-> binopFoo = lint;
    }

    BoolInfo::BoolInfo (TypeInfo::UnopLint lint) {
	this-> unopFoo = lint;
    }

    TypeInfo * BoolInfo::binaryOp (Lexical::TokenPtr tok, Syntax::Expression * right) {
	return NULL;
    }

    TypeInfo * BoolInfo::binaryOpRight (Lexical::TokenPtr tok, Syntax::Expression*) {
	return NULL;
    }

    TypeInfo * BoolInfo::unaryOp (Lexical::TokenPtr tok) {
	return NULL;
    }

    TypeInfo * BoolInfo::multOp (Lexical::TokenPtr tok, Syntax::Expression*) {
	return NULL;
    }

    std::string BoolInfo::typeToString () const {
	return "bool";
    }

    bool BoolInfo::Is (TypeEnum info) {
	return info == BOOL;
    }

};
