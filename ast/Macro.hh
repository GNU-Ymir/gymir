#pragma once

#include "Declaration.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"
#include <ymir/utils/Array.hh>

namespace syntax {
       
    enum class MacroVarConst {
	EXPR,
	IDENT
    };

    class IMacroElement {
    public: 
	semantic::Symbol info;

	virtual IMacroElement* clone () = 0;
    };
    
    class IMacroExpr {	
	std::vector <IMacroElement*> elements;
    public:
	IMacroExpr (Word begin, Word end, std::vector <IMacroElement*> elements);

    };

    class IMacroToken : public IMacroElement {
	std::string value;
    public:
	IMacroToken (std::string value);
	
	IMacroToken* clone () override;
    };

    
    class IMacroRepeat : public IMacroElement {
	Word ident;
	IMacroExpr* content;
	IMacroToken * pass;
	bool oneTime;
    public :	
	IMacroRepeat (Word ident, IMacroExpr * content, IMacroToken * pass, bool atLeastOneTime);

	IMacroRepeat* clone () override;
    };
    
    class IMacroVar : public IMacroElement {
	Word name;
	MacroVarConst type;	
    public:	
	IMacroVar (Word name, MacroVarConst type);

	IMacroVar* clone () override;
    };
    

    class IMacro : public IDeclaration {
	Word ident;
	std::vector <IMacroExpr*> _exprs;
	std::vector <Block> _blocks;
	
    public:

	IMacro (Word ident, std::vector <IMacroExpr*> _exprs, std::vector <Block> _blocks);
	void declare () override;
	
	void declare (semantic::Module) override;
	
	void declareAsExtern (semantic::Module) override;
	
    };

    class IMacroCall : public IExpression {
	Word end;
	Expression left;
	std::vector <Word> content;
	
    public :

	IMacroCall (Word begin, Word end, Expression left, std::vector<Word> content);
	
	Expression expression () override;
	
	Expression templateExpReplace (const std::map <std::string, Expression>&) override;
	
	static const char * id () {
	    return TYPEID (IMacroCall);
	}
	
	std::vector <std::string> getIds () override;	
	
    };
	    
    typedef IMacro* Macro;
    typedef IMacroExpr* MacroExpr;
    typedef IMacroCall* MacroCall;
    typedef IMacroElement* MacroElement;
    typedef IMacroVar* MacroVar;
    typedef IMacroToken* MacroToken;
    typedef IMacroRepeat* MacroRepeat;
}


