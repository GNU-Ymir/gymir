#pragma once


#include "Var.hh"
#include "Declaration.hh"
#include <ymir/utils/Array.hh>
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class IProto : public IDeclaration {

	Expression _type;
	Word _retDeco;
	std::vector <Var> _params;
	std::string space;
	bool _isVariadic;
	
	semantic::Frame _frame;
	
    public:

	Word ident;
	std::string from;
	
	IProto (Word ident, const std::string & docs, const std::vector <Var>& params, bool isVariadic);

	IProto (Word ident, const std::string & docs, Expression type, Word retDeco, const std::vector <Var>& params, std::string space, bool isVariadic);

	Ymir::json generateDocs () override;
	
	void declare () override;
	
	void declare (semantic::Module) override;
	
	void declareAsExtern (semantic::Module) override;
	
	Declaration templateDeclReplace (const std::map <std::string, Expression>&) override;

	std::string name ();
	
	Expression& type ();

	std::string & externLang ();
	
	std::string & externLangSpace ();
	
	Word& retDeco ();
	
	bool& isVariadic ();

	std::vector <Var>& params ();
	
	void print (int nb = 0) override;

	virtual ~IProto ();
	
    };

    typedef IProto* Proto;
}
