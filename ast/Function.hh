#pragma once

#include "Declaration.hh"
#include "Var.hh"
#include "Expression.hh"
#include "Block.hh"
#include <vector>
#include "../syntax/Word.hh"
#include <ymir/utils/Array.hh>
#include <ymir/semantic/pack/Frame.hh>
#include <ymir/semantic/pack/Namespace.hh>

namespace syntax {
    
    class IFunction : public IDeclaration {
    protected :
	
	Word ident;
	Var type;
	std::vector <Var> params;
	std::vector <Expression> tmps;
	Block block;
	Expression test;

    public:
	
	IFunction (Word ident, std::vector <Var> params, std::vector <Expression> tmps, Expression test, Block block);
	
	IFunction (Word ident, Var type, std::vector <Var> params, std::vector <Expression> tmps, Expression test, Block block);

	Word getIdent ();	
	
	std::string name ();

	std::vector <Var>& getParams ();

	std::vector <Expression>& getTemplates ();

	Declaration templateDeclReplace (std::map <std::string, Expression> tmps);
	
	IFunction* templateReplace (std::map <std::string, Expression> tmps);
	
	Var getType ();

	Block getBlock ();
	
	void declare () override;
	
	void declareAsExtern (semantic::Module) override;
	
	void print (int nb = 0) override;
	
    private:
	
	semantic::Frame verifyPure (semantic::Namespace);
	semantic::Frame verifyPureExtern (semantic::Namespace);
	bool verifyTemplates ();
	
    };

    typedef IFunction* Function;
    
}
