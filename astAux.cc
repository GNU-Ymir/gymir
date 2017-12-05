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
    
    const char* IParamList::getId () {
	return IParamList::id ();
    }

    const char * IParamList::id () {
	return "IParamList";
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
       
    IArrayAlloc::IArrayAlloc (Word token, Expression type, Expression size) :
	IExpression (token),
	type (type),
	size (size)
    {
	this-> size-> inside = this;
	if (this-> type)
	    this-> type-> inside = this;
    }    

    IAssert::IAssert (Word token, Expression test, Expression msg, bool isStatic) :
	IInstruction (token),
	expr (test),
	msg (msg)
    {
	this-> expr-> inside = this;
	if (this-> msg)
	    this-> msg-> inside = this;
	this-> isStatic = isStatic;
    }
            
    IBinary::IBinary (Word word, Expression left, Expression right) :
	IExpression (word),
	left (left),
	right (right)
    {
	if (this-> left) this-> left-> inside = this;
	if (this-> right) this-> right-> inside = this;	    
    }

    const char* IBinary::getId () {
	return IBinary::id ();
    }

    IFixed::IFixed (Word word, FixedConst type) :
	IExpression (word),
	type (type)
    {}

    
    IChar::IChar (Word word, ubyte code) :
	IExpression (word),
	code (code) {
    }

    
    IFloat::IFloat (Word word) : IExpression (word), _type (FloatConst::DOUBLE) {
	this-> totale = "0." + this-> token.getStr ();
    }
    
    IFloat::IFloat (Word word, std::string suite) :
	IExpression (word),
	suite (suite),
	_type (FloatConst::DOUBLE)
    {
	this-> totale = this-> token.getStr () + "." + suite;
    }

    
    IString::IString (Word word, std::string content) :
	IExpression (word),
	content (content)
    {}
    
    IBreak::IBreak (Word token) : IInstruction (token) {
	this-> ident.setEof ();
    }

    IBreak::IBreak (Word token, Word ident) :
	IInstruction (token),
	ident (ident) {
    }
    
    ICast::ICast (Word begin, Expression type, Expression expr) :
	IExpression (begin),
	type (type),
	expr (expr) {
	this-> type-> inside = this;
	this-> expr-> inside = this;
    }

    IConstArray::IConstArray (Word token, std::vector <Expression> params) :
	IExpression (token),
	params (params)
    {}
    
	
    IConstRange::IConstRange (Word token, Expression left, Expression right) :
	IExpression (token),
	left (left),
	right (right)
    {
	this-> left-> inside = this;
	this-> right-> inside = this;
    }

    IDColon::IDColon (Word token, Expression left, Expression right) :
	IExpression (token),
	left (left),
	right (right)
    {}

    
    IDot::IDot (Word word, Expression left, Expression right) :
	IExpression (word),
	left (left),
	right (right)
    {}

    
    IDotCall::IDotCall (Instruction inside, Word token, Expression call, Expression firstPar) :
	IExpression (token),
	_call (call),
	_firstPar (firstPar)
    {
	this-> inside = inside;
	this-> info = call-> info;
    }

    Expression& IDotCall::call () {
	return this->_call;
    }

    Expression& IDotCall::firstPar () {
	return this-> _firstPar;
    }
    
    IPar::IPar (Word word, Word end) :
	IExpression (word),
	end (end),
	params (NULL),
	_left (NULL),
	_dotCall (NULL),
	_opCall (false),
	_score (NULL)
    {}

    IPar::IPar (Word word, Word end, Expression left, ParamList params, bool fromOpCall) :
	IExpression (word),
	end (end),
	params (params),
	_left (left),
	_dotCall (NULL),
	_opCall (fromOpCall),
	_score (NULL)
    {
	this-> _left-> inside = this;
	this-> params-> inside = this;	    	    
    }
    
    ParamList& IPar::paramList () {
	return this-> params;
    }
    
    Expression& IPar::left () {
	return this-> _left;
    }
    
    semantic::ApplicationScore& IPar::score () {
	return this-> _score;
    }
    
    IDotCall*& IPar::dotCall () {
	return this-> _dotCall;
    }
        
    IProto::IProto (Word ident, std::vector <Var> params, bool isVariadic) :
	ident (ident),
	_type (NULL),
	_params (params),
	space (""),
	_isVariadic (isVariadic)
    {}

    IProto::IProto (Word ident, Var type, std::vector <Var> params, std::string space, bool isVariadic) :
	ident (ident),
	_type (type),
	_params (params),
	space (space),
	_isVariadic (isVariadic)
    {}

    bool& IProto::isVariadic () {
	return this-> _isVariadic;
    }
    
    std::vector <Var>& IProto::params () {
	return this-> _params;
    }

    Var& IProto::type () {
	return this-> _type;
    }
    
}
