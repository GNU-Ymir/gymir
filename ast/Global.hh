#pragma once

#include "Declaration.hh"
#include "Expression.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace semantic {
    class ISymbol;
    typedef ISymbol* Symbol;
}

namespace syntax {

    class IGlobal : public IDeclaration {

	Word ident;
	Expression expr, type;
	bool isExternal;
	bool _isImut = false;
	
    public:

	semantic::Symbol sym;
	std::string from;
	std::string space;

	IGlobal (Word ident, Expression expr, Expression type = NULL);

	IGlobal (Word ident, Expression type, bool isExternal);

	bool & isImut ();

	bool fromC ();

	void declare () override;
	
	void declare (semantic::Module) override;

	void declareAsExtern (semantic::Module) override;

	Declaration templateDeclReplace (const std::map <std::string, Expression>&) override;
	
	Expression getExpr ();
	
	void print (int nb = 0) override;
	
	virtual ~IGlobal ();
	
    };

    typedef IGlobal* Global;
    
}
