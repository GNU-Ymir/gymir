#pragma once

#include "TypeInfo.hh"
#include "Frame.hh"

namespace Semantic {

    struct FunctionInfo : TypeInfo {

	FunctionInfo (const std::string & name);
	
	TypeInfo * binaryOp (Lexical::TokenPtr, Syntax::Expression*) override;
	
	TypeInfo * binaryOpRight (Lexical::TokenPtr tok, Syntax::Expression*) override;
	
	TypeInfo * unaryOp (Lexical::TokenPtr tok) override;
	
	TypeInfo * multOp (Lexical::TokenPtr tok, Syntax::Expression*) override;

	TypeInfo * clone () override;
	
	bool Is (TypeEnum) override;

	std::string typeToString () const;
	
	void insert (Frame fr);

    private:
	
	std::string name;	
	std::vector <Frame> frames;
	
    };
    
}
