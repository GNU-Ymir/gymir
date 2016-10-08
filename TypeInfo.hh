#pragma once

#include "Tree.hh"
#include "Token.hh"
#include <vector>
#include <map>

namespace Syntax {
    struct Expression;
}

namespace Semantic {
    
    enum TypeEnum {
	INT,
	FLOAT,
	CHAR,
	BOOL,
	FUNCTION,
	UNDEF,
	VOID,
	STRUCT
    };

    struct TypeInfo {

	TypeInfo ();
	
	Ymir::Tree buildBinaryOp (Syntax::Expression* left, Syntax::Expression* right);	
	Ymir::Tree buildUnaryOp (Syntax::Expression* elem);
	Ymir::Tree buildMultOp (Syntax::Expression* left, Syntax::Expression* rights);


	virtual std::string typeToString () const = 0;
	virtual bool Is (TypeEnum) = 0;
	virtual TypeInfo * binaryOp (Lexical::TokenPtr tok, Syntax::Expression*) = 0;
	virtual TypeInfo * binaryOpRight (Lexical::TokenPtr tok, Syntax::Expression*) = 0;
	virtual TypeInfo * unaryOp (Lexical::TokenPtr tok) = 0;
	virtual TypeInfo * multOp (Lexical::TokenPtr tok, Syntax::Expression*) = 0;
	
	static TypeInfo* create_type (Lexical::TokenPtr tok, std::vector <Syntax::Expression*> tpls) {
	    auto it = creators.find (tok->getStr ());
	    if (it != creators.end ()) return it->second (tpls);
	    return NULL;
	}
	
	static bool exist_type (Lexical::TokenPtr tok) {
	    auto it = creators.find (tok->getStr());
	    if (it != creators.end ()) return true;
	    return false;
	}
	
	virtual ~TypeInfo () {
	    all[id] = NULL;
	}
	
	static void clear () {
	    for (auto it : all) {
		if (it != NULL)
		    delete it;
	    }
	}


	
    protected:

	typedef Ymir::Tree (*BinopLint) (Syntax::Expression*, Syntax::Expression*);
	typedef Ymir::Tree (*UnopLint) (Syntax::Expression*);

	typedef TypeInfo* (*TypeCreator) (std::vector<Syntax::Expression*>);
	
	static std::map <std::string, TypeCreator> creators;
	
	BinopLint binopFoo;
	UnopLint unopFoo;
	BinopLint multFoo;
	
    private:

	ulong id;	
	static std::vector <TypeInfo*> all;
	
    };
        
}
