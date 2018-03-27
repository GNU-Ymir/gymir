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

    class IProto;
    typedef IProto* Proto;
    
    class IFunction : public IDeclaration {
    protected :
	
	Word ident;
	Var type;
	std::vector <Var> params;
	std::vector <Expression> tmps;
	std::vector <Word> attrs;
	Block block;
	Expression test;

    public:
	
	IFunction (Word ident, const std::vector <Word>& attrs, const std::vector<Var> & params, const std::vector <Expression>& tmps, Expression test, Block block);
	
	IFunction (Word ident, const std::vector <Word>& attrs, Var type, const std::vector<Var> & params, const std::vector <Expression>& tmps, Expression test, Block block);

	Word getIdent ();	
	
	std::string name ();

	void name (std::string &);

	void name (const char*);

	std::vector <Var>& getParams ();

	std::vector <Expression>& getTemplates ();

	std::vector <semantic::Symbol> allInnerDecls ();

	std::vector <Word> & getAttributes ();

	Declaration templateDeclReplace (const std::map <std::string, Expression>& tmps);
	
	IFunction* templateReplace (const std::map <std::string, Expression>& tmps);
	
	Var getType ();

	Expression getTest ();
	
	Block getBlock ();
	
	void declare () override;
	
	void declare (semantic::Module)	override;

	void declareAsExtern (semantic::Module) override;
	
	void print (int nb = 0) override;

	virtual ~IFunction ();
	
    protected:
	
	semantic::Frame verifyPure (semantic::Namespace);
	semantic::Frame verifyPureExtern (semantic::Namespace);
	Proto toProto ();
	bool verifyTemplates ();
	
    };

    typedef IFunction* Function;
    
}
