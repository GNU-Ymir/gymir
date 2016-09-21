#pragma once

#include "TypeInfo.hh"


namespace Semantic {

    struct IntInfo : TypeInfo {

	IntInfo (TypeInfo::BinopLint lint);

	IntInfo (TypeInfo::UnopLint lint);
	
	TypeInfo * binaryOp (Lexical::TokenPtr, Syntax::Expression*) override;
	
	TypeInfo * binaryOpRight (Lexical::TokenPtr tok, Syntax::Expression*) override;
	
	TypeInfo * unaryOp (Lexical::TokenPtr tok) override;
	
	TypeInfo * multOp (Lexical::TokenPtr tok, Syntax::Expression*) override;

	bool Is (TypeEnum) override;
	
    private:
	
	TypeInfo * Affect (TypeInfo*);
	
    };
    

}
