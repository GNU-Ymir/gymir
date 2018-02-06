#include <ymir/ast/_.hh>
#include <ymir/semantic/pack/Table.hh>
#include <ymir/syntax/Keys.hh>
#include <ymir/syntax/Token.hh>

namespace syntax {

    using namespace semantic;

    ulong ILambdaFunc::__nbLambda__ = 0;

    std::map <Expression, Ymir::Tree> IExpand::__values__;
    
    Word& IBlock::getIdent () {
	return this-> ident;
    }

    IBlock::~IBlock ()  {
	for (auto it : decls)
	    delete it;
	    
	for (auto it : insts)
	    delete it;
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
    
    
    IFunction::IFunction (Word ident, const std::vector<Var> & params, const std::vector <Expression>& tmps, Expression test, Block block) :
	ident (ident),
	type (NULL),
	params (params),
	tmps (tmps),
	block (block),
	test (test)	    
    {
	this-> is_public (true);
    }    
	
    IFunction::IFunction (Word ident, Var type, const std::vector<Var> & params, const std::vector <Expression>& tmps, Expression test, Block block) :
	ident (ident),
	type (type),
	params (params),
	tmps (tmps),
	block (block),
	test (test)	    
    {
	this-> is_public (true);
    }

    IFunction::~IFunction () {
	if (type) delete type;
	for (auto it : params)
	    delete it;
	for (auto it : tmps)
	    delete it;
	
	if (block) delete block;
	if (test) delete test;
    }
       
    Word IFunction::getIdent () {
	return this-> ident;
    }

    std::vector <Var>& IFunction::getParams () {
	return this-> params;
    }
    
    std::vector <Expression>& IFunction::getTemplates () {
	return this-> tmps;
    }

    Expression IFunction::getTest () {
	return this-> test;
    }
    
    Var IFunction::getType () {
	return this-> type;
    }

    std::string IFunction::name () {
	return this-> ident.getStr ();
    }

    void IFunction::name (std::string &other) {
	this-> ident.setStr (other);
    }

    void IFunction::name (const char* other) {
	this-> ident.setStr (other);
    }
    
    Block IFunction::getBlock () {
	return this-> block;
    }
       
    IParamList::IParamList (Word ident, const std::vector<Expression> & params) :
	IExpression (ident),
	params (params)
    {}

       
    IParamList::~IParamList () {
	for (auto it : params)
	    delete it;
    }
    
    std::vector <Expression>& IParamList::getParams () {
	return this-> params;
    }
    
    std::vector <semantic::InfoType>& IParamList::getTreats () {
	return this-> treats;
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

    IVar::IVar (Word ident, const std::vector<Expression> & tmps) :
	IExpression (ident),
	templates (tmps)
    {
	for (auto it : this-> templates) {
	    it-> inside = this;
	}
    }

    IVar::~IVar () {
	for (auto it : templates)
	    delete it;
    }
    
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
	Ymir::OutBuffer buf;
	buf.write (this-> token.getStr ());
	if (this-> templates.size () != 0) {
	    buf.write ("!(");
	    int i = 0;	
	    for (auto __attribute__ ((unused)) it : this-> templates) {
		buf.write ("_");
		if (i < (int) this-> templates.size () - 1)
		    buf.write (", ");
		i = i + 1;
	    }
	    buf.write (")");
	}
	return buf.str ();
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

    ITypedVar::~ITypedVar () {
	if (type) delete type;
	if (expType) delete expType;
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
				 this-> expType-> prettyPrint ().c_str ());
	}
    }
    
    bool IVar::hasTemplate () {
	return this-> templates.size () != 0;
    }


    
    IType::IType (Word token, InfoType type) :
	IVar (token),
	_type (type)
    {
	this-> info = new (Z0)  ISymbol (token, type);
    }    

    IType::~IType () {
	if (_type) delete _type;
    }
    
    InfoType IType::type () {
	return this-> _type;
    }

    Expression IType::onClone () {
	return new (Z0) IType (this-> token, this-> _type);
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


    IVarDecl::IVarDecl (Word word, const std::vector<Word> & decos, const std::vector <Var>& decls, const std::vector <Expression>& insts) :
	IInstruction (word),
	decls (decls),
	insts (insts),
	decos (decos)
    {}
    
    IVarDecl::IVarDecl (Word word) :
	IInstruction (word)
    {}

    IVarDecl::~IVarDecl () {
	for (auto it : decls)
	    delete it;
	
	for (auto it : insts) {
	    if (it) {
		if (auto bin = it-> to<IBinary> ()) {
		    bin-> getLeft () = NULL;
		}
		delete it;
	    }
	}
    }
    
    IArrayAlloc::IArrayAlloc (Word token, Expression type, Expression size, bool isImmutable) :
	IExpression (token),
	type (type),
	size (size),
	isImmutable (isImmutable)
    {
	this-> size-> inside = this;
	if (this-> type)
	    this-> type-> inside = this;
    }    

    Expression IArrayAlloc::getType () {
	return this-> type;
    }

    Expression IArrayAlloc::getSize () {
	return this-> size;
    }
    
    std::string IArrayAlloc::prettyPrint () {
	return Ymir::OutBuffer ("arrayalloc TODO").str ();
    }

    IArrayAlloc::~IArrayAlloc () {
	delete type;
	type = NULL;
	delete size;
	size = NULL;
	if (cster)
	    delete cster;
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

    IAssert::~IAssert ()  {
	delete expr;
	if (msg)
	    delete msg;
    }
    
    IBinary::IBinary (Word word, Expression left, Expression right) :
	IExpression (word),
	left (left),
	right (right)
    {
	if (this-> left) this-> left-> inside = this;
	if (this-> right) this-> right-> inside = this;	    
    }

    IBinary::~IBinary () {
	if (this-> left) delete left;
	if (this-> right) delete right;	    
    }        
    
    Expression& IBinary::getLeft () {
	return this-> left;
    }

    Expression& IBinary::getRight () {
	return this-> right;
    }
    
    std::vector <std::string> IBinary::getIds () {
	auto ret = IExpression::getIds ();
	ret.push_back (TYPEID(IBinary));
	return ret;
    }

    std::string IBinary::prettyPrint () {
	return Ymir::OutBuffer ("(", this-> left, " ", this-> token.getStr (), " ", this-> right, ")").str ();
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


    std::string IFixed::prettyPrint () {
	if (isSigned (this-> type))
	    return Ymir::OutBuffer (this-> value).str ();
	else
	    return Ymir::OutBuffer (this-> uvalue).str ();
    }
    
    
    IChar::IChar (Word word, ubyte code) :
	IExpression (word),
	code (code) {
    }
    
    std::string IChar::prettyPrint () {
	return Ymir::OutBuffer ((char) this-> code).str ();
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

    IFloat::IFloat (Word word, std::string suite, FloatConst type) :
	IExpression (word),
	suite (suite),
	_type (type)
    {
	this-> totale = this-> token.getStr () + "." + suite;
    }

    IFloat::IFloat (Word word, FloatConst type) :
	IExpression (word),
	suite (""),
	_type (type)
    {	
    }

    void IFloat::setValue (float val) {
	this-> totale = Ymir::OutBuffer (val).str ();
    }

    void IFloat::setValue (double val) {
	this-> totale = Ymir::OutBuffer (val).str ();
    }    

    std::string IFloat::getValue () {
	return this-> totale;
    }
    
    std::string IFloat::prettyPrint () {
	return this-> totale;
    }

    IBool::IBool (Word token) : IExpression (token) {
	this-> value = token == Keys::TRUE_;
    }
    
    std::string IBool::prettyPrint () {	
	return this-> token.getStr ();
    }

    bool &IBool::getValue () {
	return this-> value;
    }
    
    std::string INull::prettyPrint () {
	return "null";
    }

    std::string IIgnore::prettyPrint () {
	return "_";
    }
    
    IString::IString (Word word, std::string content) :
	IExpression (word),
	content (content)
    {}

    std::string IString::getStr () {
	return this-> content;
    }

    std::string IString::prettyPrint () {
	return Ymir::OutBuffer ("\"", this-> content, "\"").str ();
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

    ICast::~ICast () {
	delete type;
	delete expr;
    }
    
    IConstArray::IConstArray (Word token, const std::vector<Expression> & params) :
	IExpression (token),
	params (params)
    {}

    IConstArray::~IConstArray () {
	for (auto it : params)
	    delete it;
	for (auto it : casters)
	    delete it;
    }
    
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

    IConstRange::~IConstRange () {
	delete left;
	delete right;
    }    
    
    IDColon::IDColon (Word token, Expression left, Expression right) :
	IExpression (token),
	left (left),
	right (right)
    {}

    IDColon::~IDColon () {	
	delete left;
	delete right;	
    }
    
    
    IDot::IDot (Word word, Expression left, Expression right) :
	IExpression (word),
	left (left),
	right (right)
    {
	if (this-> left) this-> left-> inside = this;
	//if (this-> right) this-> right-> inside = this;
    }

    IDot::~IDot () {
	delete left;
	delete right;
    }
    
    Expression IDot::getLeft () {
	return this-> left;
    }   
    
    IDotCall::IDotCall (Instruction inside, Word token, Expression call, Expression firstPar) :
	IExpression (token),
	_call (call),
	_firstPar (firstPar)
    {
	this-> inside = inside;
	this-> info = call-> info;
    }

    IDotCall::~IDotCall () {
	delete _call;
	delete _firstPar;
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

    IPar::~IPar () {
	delete params;
	delete _left;
	if (_dotCall) delete _dotCall;
	if (_score) delete _score;
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
        
    IProto::IProto (Word ident, const std::vector<Var> & params, bool isVariadic) :
	_type (NULL),
	_params (params),
	space (""),
	_isVariadic (isVariadic),
	ident (ident)
    {}

    IProto::IProto (Word ident, Var type, const std::vector<Var> & params, std::string space, bool isVariadic) :
	_type (type),
	_params (params),
	space (space),
	_isVariadic (isVariadic),
	ident (ident)
    {}

    IProto::~IProto () {
	if (_type) delete _type;
	for (auto it : _params)
	    delete it;
    }
    
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
	this-> info = new (Z0)  ISymbol (locus, info);
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

    IFor::IFor (Word token, Word id, const std::vector<Var> & var, Expression iter, Block bl) :
	IInstruction (token),
	id (id),
	var (var),
	iter (iter),
	block (bl)
    {
	this-> iter-> inside = this;
    }

    IFor::~IFor ()  {
	for (auto it : var)
	    delete it;
	delete iter;
	delete block;
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

    IWhile::~IWhile () {
	if (test) delete test;
	if (block) delete block;
    }
    
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

    IUnary::~IUnary () {
	if (type) delete type;
	delete elem;
    }    

    void IUnary::print (int nb) {
	printf ("\n%*c<Unary> %s",
		nb, ' ',
		this-> token.toString ().c_str ()
	);
	this-> elem-> print (nb + 4);
    }
       
    IStructCst::IStructCst (Word word, Word end, Expression left, ParamList params) :
	IExpression (word),
	end (end),
	params (params),
	left (left)	    
    {
	this-> left-> inside = this;
	this-> params-> inside = this;
    }

    IStructCst::IStructCst (Word word, Word end) :
	IExpression (word),
	end (end)
    {
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

    IAccess::~IAccess () {
	delete params;
	delete left;
	for (auto &it : treats)
	    delete it;
	treats.clear ();
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
          
    IConstTuple::IConstTuple (Word word, Word end, const std::vector<Expression> & params) :
	IExpression (word),
	end (end),
	params (params)
    {}

    IConstTuple::~IConstTuple () {
	for (auto it : params)
	    delete it;
	for (auto it : casters)
	    delete it;
    }
    
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
    
	
    IExpand::~IExpand () {
	delete expr;
    }
    
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

    InfoType& IReturn::getCaster () {
	return this-> caster;
    }
    
    IReturn::~IReturn () {
	if (caster) delete caster;
	if (elem) delete elem;
    }
    
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

    IArrayVar::~IArrayVar () {
	if (content) delete content;
    }
    
    Expression IArrayVar::contentExp () {
	return this-> content;
    }
    
    void IArrayVar::print (int nb) {
	printf ("\n%*c <ArrayVar> %s",
		nb, ' ',
		this-> token.toString ().c_str ()
	);	
    }

    IExpression::~IExpression () {
    }
    
    IExpression* IExpression::clone () {
	auto ret = this-> onClone ();
	if (this-> info && ret != this) 
	    ret-> info = new (Z0)  ISymbol (this-> info-> sym, this-> info-> type ? this-> info-> type-> clone () : NULL);
	return ret;
    }

    Expression IExpression::templateExpReplace (const std::map <std::string, Expression>&) {
	this-> print (0);
	Ymir::Error::assert ((std::string ("TODO") + this-> getIds ().back ()).c_str ());
	return NULL;	    
    }

    void IExpression::print (int) {
	Ymir::OutBuffer buf ("TODO {", this-> getIds (), "}");
	Ymir::Error::assert (buf.str ().c_str ());
    }

    std::string IExpression::prettyPrint () {
	Ymir::OutBuffer buf ("TODO {", this-> getIds (), "}");
	Ymir::Error::assert (buf.str ().c_str ());

	return "";
    }    
    
    ILambdaFunc::ILambdaFunc (Word begin, std::vector <Var> params, Var type, Block block) : 
	IExpression (begin),
	params (params),
	ret (type),
	block (block),
	id (getLastNb ())
    {}

    ILambdaFunc::ILambdaFunc (Word begin, semantic::LambdaFrame frame) : 
	IExpression (begin),	
	params (),
	ret (NULL),
	block (NULL),
	id (0),
	frame (frame)
    {}

    
    ILambdaFunc::ILambdaFunc (Word begin, std::vector <Var> params, Block block) : 
	IExpression (begin),
	params (params),
	ret (NULL),
	block (block),
	id (getLastNb ())
    {}
	
    ILambdaFunc::ILambdaFunc (Word begin, std::vector <Var> params, Expression ret) :
	IExpression (begin),
	params (params),
	ret (NULL),
	block (NULL),
	expr (ret),
	id (getLastNb ())
    {}

    std::string ILambdaFunc::prettyPrint () {
	auto ident = Ymir::OutBuffer ("Lambda_", this-> id, " (");
	for (auto it : Ymir::r (0, this-> params.size ())) {
	    ident.write (this-> params [it]-> prettyPrint ());
	    if (it < (int) this-> params.size () - 1)
		ident.write (", ");
	}
	ident.write (")");
	return ident.str ();
    }
    
    std::vector <Var> & ILambdaFunc::getParams () {
	return this-> params;
    }
    
    Expression ILambdaFunc::getExpr () {
	return this-> expr;
    }

    Block ILambdaFunc::getBlock () {
	return this-> block;
    }
    
    ulong ILambdaFunc::getLastNb () {
	__nbLambda__ ++;
	return __nbLambda__;
    }
    
    ILambdaFunc::~ILambdaFunc () {
	for (auto it : params) delete it;
	if (ret) delete ret;
	if (block) delete block;
	if (expr) delete expr;
    }


    IFuncPtr::IFuncPtr (Word begin, std::vector <Var> params, Var type, Expression expr) :
	IExpression (begin),
	params (params),
	ret (type),
	expr (expr)
    {
	this-> ret-> inside = this;
	if (this-> expr)
	    this-> expr-> inside = this;
    }    

    std::vector <Var> & IFuncPtr::getParams () {
	return this-> params;
    }

    Var IFuncPtr::getRet () {
	return this-> ret;
    }
    
    const char * IFuncPtr::id () {
	return TYPEID (IFuncPtr);
    }

    std::vector <std::string> IFuncPtr::getIds () {
	auto ret = IExpression::getIds ();
	ret.push_back (TYPEID (IFuncPtr));
	return ret;
    }

    
    IFuncPtr::~IFuncPtr () {
	for (auto it : params)
	    delete it;
	delete ret;
	if (expr)
	    delete expr;
    }
    

    IStruct::IStruct (Word ident, std::vector <Expression> tmps, std::vector <Var> params) :
	ident (ident),
	params (params),
	tmps (tmps)	      
    {
	this-> isPublic = true;
    }

    IStruct::~IStruct () {
	for (auto it : params)
	    delete it;
	for (auto it : tmps)
	    delete it;
    }
       
    IIs::IIs (Word begin, Expression expr, Expression type) :
	IExpression (begin),
	left (expr),
	type (type),
	expType (Word::eof ())
    {
	if (left) this-> left-> inside = this;
	if (type) this-> type-> inside = this;
    }

    IIs::IIs (Word begin, Expression expr, Word type) :
	IExpression (begin),
	left (expr),
	type (NULL),
	expType (type)
    {
	if (left) this-> left-> inside = this;
    }

    IIs::~IIs () {
	delete left;
	if (type) delete type;
    }

    ITupleDest::ITupleDest (Word token, bool isVariadic, std::vector <Var> decls, Expression right) :
	IInstruction (token),
	decls (decls),
	right (right),
	isVariadic (isVariadic)	    
    {}

    ITupleDest::ITupleDest (Word token, std::vector <Expression> insts, Expression right) :
	IInstruction (token),
	insts (insts),
	right (right),
	isVariadic (false)
    {}

    
    ITupleDest::~ITupleDest () {
	for (auto it : decls)
	    delete it;
	for (auto it : insts)
	    delete it;
	if (right) delete right;
    }
    
    IEnum::IEnum (Word ident, Var type, std::vector <Word> names, std::vector <Expression> values) :
	ident (ident),
	type (type),
	names (names),
	values (values)
    {}

    IEnum::~IEnum () {
	for (auto it : values)
	    delete it;
    }

}
