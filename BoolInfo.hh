#pragma once

#include "TypeInfo.hh"

namespace Semantic {

    struct BoolInfo : TypeInfo {

	BoolInfo ();

	BoolInfo (TypeInfo::BinopLint lint);
	
	BoolInfo (TypeInfo::UnopLint lint);

	TypeInfo * binaryOp (Lexical::TokenPtr, Syntax::Expression*) override;

	TypeInfo * binaryOpRight (Lexical::TokenPtr tok, Syntax::Expression*) override;
	
	TypeInfo * unaryOp (Lexical::TokenPtr tok) override;

	TypeInfo * multOp (Lexical::TokenPtr tok, Syntax::Expression*) override;

	bool Is (TypeEnum) override;


	std::string typeToString () const;

	static TypeInfo * create (std::vector<Syntax::Expression*>) {
	    return new BoolInfo ();
	}

    private:

	static class _init {
	public :
	    _init () {
		TypeInfo::creators["bool"] = &BoolInfo::create;
	    }
	} _initializer;

    private:
	
	
    };
   
};
