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
	return "none";
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
	return "none";
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

    std::vector <Var>& IFunction::getParams () {
	return this-> params;
    }
    
    std::vector <Expression>& IFunction::getTemplates () {
	return this-> tmps;
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
    
    std::vector <std::string> IParamList::getIds () {
	auto ret = IExpression::getIds ();
	ret.push_back (TYPEID (IParamList));
	return ret;
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
	
    std::vector <std::string> IVar::getIds () {
	auto ret = IExpression::getIds ();
	ret.push_back (TYPEID (IVar));
	return ret;
    }
    	
    std::vector <std::string> IArrayVar::getIds () {
	auto ret = IVar::getIds ();
	ret.push_back (TYPEID (IArrayVar));
	return ret;
    }     
	
    std::string IVar::prettyPrint () {
	return this-> token.getStr ();
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
	
    Var ITypedVar::typeVar () {
	return this-> type;
    }

    Expression ITypedVar::typeExp () {
	return this-> expType;
    }

    std::vector <std::string> ITypedVar::getIds () {
	auto ret = IVar::getIds ();
	ret.push_back (TYPEID (ITypedVar));
	return ret;
    }

    std::string ITypedVar::prettyPrint () {
	if (this-> type) {
	    return Ymir::format ("% : %",
				 this-> token.getStr ().c_str (),
				 this-> type-> prettyPrint ().c_str ());
	} else {
	    return Ymir::format ("% : %",
				 this-> token.getStr ().c_str (),
				 this-> expType-> info-> type-> typeString ().c_str ());
	}
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
    
    std::vector <std::string> IType::getIds () {
	auto ret = IVar::getIds ();
	ret.push_back (TYPEID (IType));
	return ret;
    }

    std::string IType::prettyPrint () {
	return this-> info-> type-> typeString ();
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

    std::vector <std::string> IArrayAlloc::getIds () {
	auto ret = IExpression::getIds ();
	ret.push_back (TYPEID(IArrayAlloc));
	return ret;
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

    std::vector <std::string> IBinary::getIds () {
	auto ret = IExpression::getIds ();
	ret.push_back (TYPEID(IBinary));
	return ret;
    }

    IFixed::IFixed (Word word, FixedConst type) :
	IExpression (word),
	type (type)
    {
	if (isSigned (this-> type)) this-> value = std::strtol (this-> token.getStr ().c_str (), NULL, 0);
	else this-> uvalue = std::strtoul (this-> token.getStr ().c_str (), NULL, 0);		
    }

    void IFixed::setUValue (ulong val) {
	this-> uvalue = val;
    }

    void IFixed::setValue (long val) {
	this-> value = val;
    }
    
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

    std::string IString::getStr () {
	return this-> content;
    }
    
    std::vector <std::string> IString::getIds () {
	auto ret = IExpression::getIds ();
	ret.push_back (TYPEID (IString));
	return ret;
    }
    
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

    int IConstArray::nbParams () {
	return this-> params.size ();
    }    
	
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

    std::vector <std::string> IDotCall::getIds () {
	auto ret = IExpression::getIds ();
	ret.push_back (TYPEID (IDotCall));
	return ret;
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
	_type (NULL),
	_params (params),
	space (""),
	_isVariadic (isVariadic),
	ident (ident)
    {}

    IProto::IProto (Word ident, Var type, std::vector <Var> params, std::string space, bool isVariadic) :
	_type (type),
	_params (params),
	space (space),
	_isVariadic (isVariadic),
	ident (ident)
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

    std::string IProto::name () {
	return this-> ident.getStr ();
    }

    std::vector <std::string> IConstArray::getIds () {
	auto ret = IExpression::getIds ();
	ret.push_back (TYPEID (IConstArray));
	return ret;
    }

    ITreeExpression::ITreeExpression (Word locus, semantic::InfoType info, Ymir::Tree content) :
	IExpression (locus),	
	_content (content)
    {
	this-> info = new ISymbol (locus, info);
    }

    void ITreeExpression::print (int) {}
    
    IIf::IIf (Word word, Expression test, Block block, bool isStatic) :
	IInstruction (word),
	test (test),
	block (block),
	else_ (NULL)
    {
	if (this-> test)
	    this-> test-> inside = this;
	this-> isStatic = isStatic;
    }
        
    IIf::IIf (Word word, Expression test, Block block, If else_, bool isStatic) : 
	IInstruction (word),
	test (test),
	block (block),
	else_ (else_)
    {
	if (this-> test)
	    this-> test-> inside = this;
	this-> isStatic = isStatic;
	if (this-> else_)
	    this-> else_-> isStatic = isStatic;
    }
    
    void IIf::print (int nb) {
	if (this-> test) {
	    printf ("\n%*c<%sIf> %s",			
		    nb, ' ',
		    this-> isStatic ? "static_" : "",
		    this-> token.toString ().c_str ()
	    );		
	    this-> test-> print (nb + 4);		
	} else {
	    printf ("\n%*c<%sElse> %s",
		    nb, ' ',
		    this-> isStatic ? "static_" : "",
		    this-> token.toString ().c_str ()
	    );
	}

	this-> block-> print (nb + 4);
	if (this-> else_)
	    this-> else_-> print (nb + 8);	    
    }

    IFor::IFor (Word token, Word id, std::vector <Var> var, Expression iter, Block bl) :
	IInstruction (token),
	id (id),
	var (var),
	iter (iter),
	block (bl)
    {
	this-> iter-> inside = this;
    }
    
    void IFor::print (int nb) {
	printf ("\n%*c<For> %s",
		nb, ' ',
		this-> token.toString ().c_str ()
	);
	for (auto it : this-> var) {
	    it-> print (nb + 4);
	}

	this-> iter-> print (nb + 5);
	this-> block-> print (nb + 4);
    }


    IWhile::IWhile (Word token, Word name, Expression test, Block block) :
	IInstruction (token),
	name (name),
	test (test),
	block (block)
    {}
    
    IWhile::IWhile (Word token, Expression test, Block block) :
	IInstruction (token),
	test (test),
	block (block)
    {}
    	
    void IWhile::print (int nb) {
	printf ("\n%*c<While> %s:%s",
		nb, ' ',
		this-> name.isEof () ? "_" : this-> name.getStr ().c_str (),
		this-> token.toString ().c_str ()
	);

	this-> test-> print (nb + 4);
	this-> block-> print (nb + 4);	    
    }

    IUnary::IUnary (Word word, Expression elem) :
	IExpression (word),
	elem (elem)
    {
	this-> elem-> inside = this;
    }


    void IUnary::print (int nb) {
	printf ("\n%*c<Unary> %s",
		nb, ' ',
		this-> token.toString ().c_str ()
	);
	this-> elem-> print (nb + 4);
    }
       
    IAccess::IAccess (Word word, Word end, Expression left, ParamList params) :
	IExpression (word),
	end (end),
	params (params),
	left (left)	    
    {
	this-> left-> inside = this;
	this-> params-> inside = this;
    }
	
    IAccess::IAccess (Word word, Word end) :
	IExpression (word),
	end (end)
    {
    }

    Expression IAccess::getLeft () {
	return this-> left;
    }
    
    std::vector <Expression> IAccess::getParams () {
	return this-> params-> getParams ();
    }
       
    void IAccess::print (int nb) {
	printf ("\n%*c<Access> %s",
		nb, ' ',
		this-> token.toString ().c_str ()
	);
	this-> left-> print (nb + 4);
	this-> params-> print (nb + 4);
    }
          
    IConstTuple::IConstTuple (Word word, Word end, std::vector <Expression> params) :
	IExpression (word),
	end (end),
	params (params)
    {}
    
    std::vector <std::string> IConstTuple::getIds () {
	auto ret = IExpression::getIds ();
	ret.push_back (TYPEID (IConstTuple));
	return ret;
    }

    void IConstTuple::print (int nb) {
	printf ("\n%*c<ConstTuple> %s",
		nb, ' ',
		this-> token.toString ().c_str ()
	);
	
	for (auto it : this-> params)
	    it-> print (nb + 4);	    
    }        
    

    IExpand::IExpand (Word begin, Expression expr) :
	IExpression (begin),
	expr (expr)
    {}
	
    IExpand::IExpand (Word begin, Expression expr, ulong it) :
	IExpression (begin),
	expr (expr),
	it (it)
    {}

    void IExpand::print (int nb) {
	printf("\n%*c<Expand> %s",
	       nb, ' ',
	       this-> token.toString ().c_str ()
	);
	this-> expr-> print (nb + 4);
    }
   
    IReturn::IReturn (Word ident) : IInstruction (ident), elem (NULL), caster (NULL) {}
    
    IReturn::IReturn (Word ident, Expression elem) :
	IInstruction (ident),	
	elem (elem),
	caster (NULL)
    {}

    void IReturn::print (int nb) {
	printf ("\n%*c<Return> %s",
		nb, ' ',
		this-> token.toString ().c_str ()
	);
	
	if (this-> elem)
	    this-> elem-> print (nb + 4);
    }    

    IArrayVar::IArrayVar (Word token, Expression content) :
	IVar (token),
	content (content)
    {}

    Expression IArrayVar::contentExp () {
	return this-> content;
    }
    
    void IArrayVar::print (int nb) {
	printf ("\n%*c <ArrayVar> %s",
		nb, ' ',
		this-> token.toString ().c_str ()
	);	
    }

    IExpression* IExpression::clone () {
	auto ret = this-> onClone ();
	if (this-> info && ret != this) 
	    ret-> info = new (GC) ISymbol (this-> info-> sym, this-> info-> type ? this-> info-> type-> clone () : NULL);
	return ret;
    }

    Expression IExpression::templateExpReplace (std::map <std::string, Expression>) {
	this-> print (0);
	Ymir::Error::assert ((std::string ("TODO") + this-> getIds ().back ()).c_str ());
	return NULL;	    
    }


    
}
