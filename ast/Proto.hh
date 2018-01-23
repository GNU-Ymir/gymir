#pragma once


#include "Var.hh"
#include "Declaration.hh"
#include <ymir/utils/Array.hh>
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class IProto : public IDeclaration {

	Var _type;
	std::vector <Var> _params;
	std::string space;
	bool _isVariadic;
	
    public:

	Word ident;
	std::string from;
	
	IProto (Word ident, const std::vector <Var>& params, bool isVariadic);

	IProto (Word ident, Var type, const std::vector <Var>& params, std::string space, bool isVariadic);

	void declare () override;

	void declareAsExtern (semantic::Module) override;
	
	std::string name ();
	
	Var& type ();
	
	bool& isVariadic ();

	std::vector <Var>& params ();
	
	void print (int nb = 0) override;

	virtual ~IProto ();
	
    };

    typedef IProto* Proto;
}
