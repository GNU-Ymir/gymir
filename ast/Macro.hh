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

    class IMacroElement {};
    
    class IMacroExpr {	
	std::vector <IMacroElement*> elements;
    public:
	IMacroExpr (Word begin, Word end, std::vector <IMacroElement*> elements);
    };

    class IMacroToken : public IMacroElement {
	std::string value;
    public:
	IMacroToken (std::string value);
    };

    
    class IMacroRepeat : public IMacroElement {
	IMacroExpr* content;
	IMacroToken * pass;
	bool oneTime;
    public :	
	IMacroRepeat (IMacroExpr * content, IMacroToken * pass, bool atLeastOneTime);
    };
    
    class IMacroVar : public IMacroElement {
	Word name;
	MacroVarConst type;	
    public:	
	IMacroVar (Word name, MacroVarConst type);	
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

	IMacroCall (Word begin, Word end, Expression left, std::vector<Word> content) :
	    IExpression (begin),
	    end (end),
	    left (left),
	    content (content)
	{}
	
	Expression expression () override {
	    println (this-> content);
	    Ymir::Error::assert ("TODO");
	    return NULL;
	}

	Expression templateExpReplace (const std::map <std::string, Expression>&) override {
	    Ymir::Error::assert ("TODO");
	    return NULL;
	}
	
	static const char * id () {
	    return TYPEID (IMacroCall);
	}
	
	std::vector <std::string> getIds () override {
	    auto ret = IExpression::getIds ();
	    ret.push_back (TYPEID (IMacroCall));
	    return ret;
	}
	
	
    };
	    
    typedef IMacro* Macro;
    typedef IMacroExpr* MacroExpr;
    typedef IMacroCall* MacroCall;
    typedef IMacroElement* MacroElement;
    typedef IMacroVar* MacroVar;
    typedef IMacroToken* MacroToken;
    typedef IMacroRepeat* MacroRepeat;
}


