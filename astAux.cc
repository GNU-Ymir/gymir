#include <ymir/ast/_.hh>
#include <ymir/semantic/pack/Table.hh>
#include <ymir/syntax/Keys.hh>
#include <ymir/syntax/Token.hh>

namespace syntax {

    using namespace semantic;

    Word& IBlock::getIdent () {
	return this-> ident;
    }


    bool isSigned (FixedConst ct) {
	return ((int) ct) % 2 == 0;
    }

    std::string name (FixedConst ct) {
	switch (ct) {
	case FixedConst::BYTE : return "byte";
	case FixedConst::UBYTE : return "ubyte";
	case FixedConst::SHORT : return "short";
	case FixedConst::USHORT: return "ushort";
	case FixedConst::INT : return "int";
	case FixedConst::UINT : return "uint";
	case FixedConst::LONG : return "long";
	case FixedConst::ULONG : return "ulong";
	}
    }

    std::string sname (FixedConst ct) {
	switch (ct) {
	case FixedConst::BYTE : return "g";
	case FixedConst::UBYTE : return "h";
	case FixedConst::SHORT : return "s";
	case FixedConst::USHORT: return "t";
	case FixedConst::INT : return "i";
	case FixedConst::UINT : return "k";
	case FixedConst::LONG : return "l";
	case FixedConst::ULONG : return "m";
	}
    }
    
    
    IFunction::IFunction (Word ident, std::vector <Var> params, std::vector <Expression> tmps, Expression test, Block block) :
	ident (ident),
	type (NULL),
	params (params),
	tmps (tmps),
	block (block),
	test (test)	    
    {}
    
	
    IFunction::IFunction (Word ident, Var type, std::vector <Var> params, std::vector <Expression> tmps, Expression test, Block block) :
	ident (ident),
	type (type),
	params (params),
	tmps (tmps),
	block (block),
	test (test)	    
    {}

    Word IFunction::getIdent () {
	return this-> ident;
    }

    std::vector <Var> IFunction::getParams () {
	return this-> params;
    }

    Var IFunction::getType () {
	return this-> type;
    }

    std::string IFunction::name () {
	return this-> ident.getStr ();
    }
    
    Block IFunction::getBlock () {
	return this-> block;
    }


    
    IParamList::IParamList (Word ident, std::vector <Expression> params) :
	IExpression (ident),
	params (params)
    {}

    std::vector <Expression>& IParamList::getParams () {
	return this-> params;
    }

    std::vector <semantic::InfoType> IParamList::getParamTypes () {
	std::vector<semantic::InfoType> paramTypes;
	for (auto it : this-> params) {
	    paramTypes.push_back (it-> info-> type);
	}
	return paramTypes;
    }

    
    IVar::IVar (Word ident) : IExpression (ident) {}

    IVar::IVar (Word ident, Word deco) :
	IExpression (ident),
	deco (deco)
    {}

    IVar::IVar (Word ident, std::vector <Expression> tmps) :
	IExpression (ident),
	templates (tmps)
    {}

    const char* IVar::id () {
	return "IVar";
    }
	
    const char* IVar::getId () {
	return IVar::id ();
    }
	


    ITypedVar::ITypedVar (Word ident, Var type) :
	IVar (ident),
	type (type)
    {}

    ITypedVar::ITypedVar (Word ident, Var type, Word deco) :
	IVar (ident),
	type (type)
    {
	this-> deco = deco;
    }

    ITypedVar::ITypedVar (Word ident, Expression type) :
	IVar (ident),
	expType (type)
    {}

    ITypedVar::ITypedVar (Word ident, Expression type, Word deco) :
	IVar (ident),
	expType (type)
    {
	this-> deco = deco;
    }
	
    const char * ITypedVar::id () {
	return "ITypedVar";
    }

    const char * ITypedVar::getId () {
	return ITypedVar::id ();
    }

    bool IVar::hasTemplate () {
	return this-> templates.size () != 0;
    }


    
    IType::IType (Word token, InfoType type) :
	IVar (token),
	_type (type)
    {
	this-> info = new ISymbol (token, type);
    }    

    InfoType IType::type () {
	return this-> _type;
    }

    const char * IType::id () {
	return "IType";
    }
    
    const char * IType::getId () {
	return IType::id ();
    }


    std::vector <Expression>& IVar::getTemplates () {
	return this-> templates;
    }

    Word& IVar::getDeco () {
	return this-> deco;
    }


    IVarDecl::IVarDecl (Word word, std::vector <Word> decos, std::vector <Var> decls, std::vector <Expression> insts) :
	IInstruction (word),
	decls (decls),
	insts (insts),
	decos (decos)
    {}

    IVarDecl::IVarDecl (Word word) :
	IInstruction (word)
    {}

    
    

}
