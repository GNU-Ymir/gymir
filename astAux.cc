#include <ymir/ast/_.hh>
#include <ymir/semantic/pack/Table.hh>
#include <ymir/semantic/pack/MacroSolver.hh>
#include <ymir/syntax/Keys.hh>
#include <ymir/syntax/Token.hh>
#include <ymir/semantic/pack/Table.hh>

namespace syntax {

    using namespace semantic;

    ulong ILambdaFunc::__nbLambda__ = 0;

    std::map <Expression, Ymir::Tree> IExpand::__values__;
    std::vector <Block> IBlock::currentBlock;
    
    std::string IInstruction::prettyPrint () {	
	Ymir::OutBuffer buf ("TODO {", this-> getIds (), "}");
	Ymir::Error::assert (buf.str ().c_str ());	
	return "";    
    }
    
    Word& IBlock::getIdent () {
	return this-> ident;
    }

    std::string IBlock::prettyPrint () {
	Ymir::OutBuffer buf ("{\n");
	for (auto it : this-> insts) {	    
	    buf.write (it-> prettyPrint ());
	    if (it-> is<IExpression> ())
		buf.write (";");
	    buf.write ("\n");
	}
	buf.write ("}\n");
	return buf.str ();
    }
    
    void IBlock::addFinally (Block block) {
	if (block-> insts.size () != 0) {
	    if (!block-> insts [0]-> is<INone> () || block-> insts.size () != 1)
		this-> finally.push_back (block);
	}
    }

    void IBlock::addFinallyAtSemantic (Instruction inst) {
	this-> _preFinally.push_back (inst);
    }
    
    void IBlock::addInline (Var var) {
	this-> inlines.push_back (var);
    }
        
    std::vector <Instruction>& IBlock::getInsts () {
	return this-> insts;
    }
    
    Block IBlock::getCurrentBlock () {
	return currentBlock.back ();
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
	case FixedConst::BYTE : return "i8";
	case FixedConst::UBYTE : return "u8";
	case FixedConst::SHORT : return "i16";
	case FixedConst::USHORT: return "u16";
	case FixedConst::INT : return "i32";
	case FixedConst::UINT : return "u32";
	case FixedConst::LONG : return "i64";
	case FixedConst::ULONG : return "u64";
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
    
    
    IFunction::IFunction (Word ident, const std::vector <Word> & attrs, const std::vector<Var> & params, const std::vector <Expression>& tmps, Expression test, Block block) :
	ident (ident),
	type (NULL),
	params (params),
	tmps (tmps),
	attrs (attrs),
	block (block),
	test (test)
    {
	this-> is_public (true);
    }    
	
    IFunction::IFunction (Word ident, const std::vector <Word> & attrs, Expression type, Word retDeco, const std::vector<Var> & params, const std::vector <Expression>& tmps, Expression test, Block block) :
	ident (ident),
	type (type),
	retDeco (retDeco),
	params (params),
	tmps (tmps),
	attrs (attrs),
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

    std::vector <Word> & IFunction::getAttributes () {
	return this-> attrs;
    }
    
    Expression IFunction::getTest () {
	return this-> test;
    }
    
    Expression IFunction::getType () {
	return this-> type;
    }

    Word IFunction::getRetDeco () {
	return this-> retDeco;
    }
    
    Block& IFunction::post () {
	return this-> _post;
    }

    Block & IFunction::pre () {
	return this-> _pre;
    }

    Var & IFunction::postVar () {
	return this-> _postVar;
    }
    
    const char* IFunction::id () {
	return TYPEID (IFunction);
    }
	
    std::vector <std::string> IFunction::getIds () {
	auto ret = IDeclaration::getIds ();
	ret.push_back (TYPEID (IFunction));
	return ret;
    }
    
    std::string & IFunction::externLang () {
	return this-> _externLang;
    }

    std::string & IFunction::externLangSpace () {
	return this-> _externLangSpace;
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

    bool IFunction::has (const std::string & str) {
	for (auto & it : this-> attrs) {
	    if (it.getStr () == str) return true;
	}
	return false;
    }
    
    IParamList::IParamList (Word ident, const std::vector<Expression> & params) :
	IExpression (ident),
	params (params)
    {}

    std::string IParamList::prettyPrint () {
	Ymir::OutBuffer buf;
	for (auto it : Ymir::r (0, this-> params.size ())) {
	    buf.write (this-> params [it]-> prettyPrint ());
	    if (it < (int) this-> params.size () - 1)
		buf.write (", ");
	}
	return buf.str ();
    }    

    std::string IFuncPtr::prettyPrint () {
	Ymir::OutBuffer buf (this-> token.getStr (), " (");
	for (auto it : Ymir::r (0, this-> params.size ())) {
	    buf.write (this-> params [it]-> prettyPrint ());
	    if (it < (int) this-> params.size () - 1)
		buf.write (", ");
	}
	buf.write (")-> ");
	buf.write (this-> ret-> prettyPrint ());
	if (this-> expr) {
	    buf.write ("(", this-> prettyPrint (), ")");
	}
	return buf.str ();
    }
    
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
	    paramTypes.push_back (it-> info-> type ());
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

    bool IVar::isLvalue () {
	return true;
    }
    
    bool IVar::fromClosure () {
	return this-> _fromClosure;
    }

    semantic::Symbol& IVar::lastInfo () {
	return this-> _lastInfo;
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
	if (!this-> deco.isEof ())
	    buf.write (this-> deco.getStr (), " ");
	buf.write (this-> token.getStr ());
	if (this-> templates.size () != 0) {
	    buf.write ("!(");
	    int i = 0;	
	    for (auto it : this-> templates) {
		buf.write (it-> prettyPrint ());
		if (i < (int) this-> templates.size () - 1)
		    buf.write (", ");
		i = i + 1;
	    }
	    buf.write (")");
	}
	return buf.str ();
    }    

    ITypedVar::ITypedVar (Word ident, Expression type) :
	IVar (ident),
	type (type)
    {}

    ITypedVar::ITypedVar (Word ident, Expression type, Word deco) :
	IVar (ident),
	type (type)
    {
	this-> deco = deco;
    }

    ITypedVar::~ITypedVar () {
	if (type) delete type;
    }
    
    Expression ITypedVar::typeExp () {
	return this-> type;
    }

    std::vector <std::string> ITypedVar::getIds () {
	auto ret = IVar::getIds ();
	ret.push_back (TYPEID (ITypedVar));
	return ret;
    }

    std::string ITypedVar::prettyPrint () {
	Ymir::OutBuffer buf;
	if (!this-> deco.isEof ()) {
	    buf.write (this-> deco.getStr (), " ");
	}

	if (this-> type) 
	    buf.write (this-> token.getStr ().c_str (), " : ", this-> type-> prettyPrint ().c_str ());	    

	return buf.str ();
    }
    
    bool IVar::hasTemplate () {
	return this-> templates.size () != 0;
    }


    
    IType::IType (Word token, InfoType type) :
	IVar (token),
	_type (type)
    {
	this-> info = new (Z0)  ISymbol (token, this, type);
    }    

    IType::~IType () {
	if (_type) delete _type;
    }
    
    InfoType IType::type () {
	return this-> _type;
    }

    Expression IVar::onClone () {
	return new (Z0) IType (this-> token, this-> info-> type ());
    }
    
    Expression IType::onClone () {
	return new (Z0) IType (this-> token, this-> _type);
    }

    Expression IArrayAlloc::onClone () {
	return new (Z0) IType (this-> token, this-> info-> type ());
    }
    
    std::vector <std::string> IType::getIds () {
	auto ret = IVar::getIds ();
	ret.push_back (TYPEID (IType));
	return ret;
    }

    std::string IType::prettyPrint () {
	if (this-> info && this-> info-> type ())
	    return this-> info-> type ()-> typeString ();
	else
	    return this-> token.getStr ();
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

    std::vector <Var> &IVarDecl::getDecls () {
	return this-> decls;
    }

    std::vector <Expression> &IVarDecl::getInsts () {
	return this-> insts;
    }
    
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
	if (this-> size)
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
	return Ymir::format ("[% ; %]", this-> type-> prettyPrint ().c_str (),
			     this-> size-> prettyPrint ().c_str ());
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

    bool IBinary::isLvalue () {
	if (this-> token == Token::EQUAL) return true;
	return false;
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
	type (type),
	mode (FixedMode::BUILTINS)
    {
	if (this-> mode == FixedMode::DECIMAL) {
	    if (isSigned (this-> type)) this-> value = this-> convertS ();	
	    else this-> uvalue = this-> convertU ();
	}
    }

    IFixed::IFixed (Word word, FixedConst type, FixedMode mode) :
	IExpression (word),
	type (type),
	mode (mode)
    {
	if (this-> mode == FixedMode::DECIMAL) {
	    if (isSigned (this-> type)) this-> value = this-> convertS ();	
	    else this-> uvalue = this-> convertU ();
	} else if (this-> mode == FixedMode::HEXA) {
	    if (isSigned (this-> type)) this-> value = this-> convertSX ();
	    else this-> uvalue = this-> convertUX ();
	}
    }

    std::string IFixed::removeUnder () {
	Ymir::OutBuffer fin;
	for (auto it : this-> token.getStr ())
	    if (it != '_') fin.write (it);
	return fin.str ();
    }   

    ulong IFixed::convertUX () {
	char * temp; errno = 0;
	auto val = "0x" + removeUnder ();
	ulong value = std::strtoul (val.c_str (), &temp, 0);
	bool overflow = false;
	if (temp == this-> token.getStr ().c_str () || *temp != '\0' ||
	    ((value == 0 || value == ULONG_MAX) && errno == ERANGE)) {
	    overflow = true;
	}
	
	switch (this-> type) {
	case FixedConst::UBYTE : overflow = value > UCHAR_MAX; break;
	case FixedConst::USHORT : overflow = value > USHRT_MAX; break;
	case FixedConst::UINT : overflow = value > UINT_MAX; break;
	default: break;
	}

	if (overflow) {
	    Ymir::Error::overflow (this-> token, name (this-> type));
	}
	return value;
    }

    long IFixed::convertSX () {
	char * temp; errno = 0;
	auto val = "0x" + removeUnder ();
	long value = std::strtol (val.c_str (), &temp, 0);
	bool overflow = false;
	if (temp == this-> token.getStr ().c_str () || *temp != '\0' ||
	    ((value == LONG_MIN || value == LONG_MAX) && errno == ERANGE)) {
	    overflow = true;
	}
	
	switch (this-> type) {
	case FixedConst::BYTE : overflow = value > SCHAR_MAX || value < SCHAR_MIN; break;
	case FixedConst::SHORT : overflow = value > SHRT_MAX || value < SHRT_MIN; break;
	case FixedConst::INT : overflow = value > INT_MAX || value < INT_MIN; break;
	default: break;
	}

	if (overflow) {
	    Ymir::Error::overflow (this-> token, name (this-> type));
	} 
	return value;
    }
    
    ulong IFixed::convertU () {
	char * temp; errno = 0;
	ulong value = std::strtoul (removeUnder ().c_str (), &temp, 0);
	bool overflow = false;
	if (temp == this-> token.getStr ().c_str () || *temp != '\0' ||
	    ((value == 0 || value == ULONG_MAX) && errno == ERANGE)) {
	    overflow = true;
	}
	
	switch (this-> type) {
	case FixedConst::UBYTE : overflow = value > UCHAR_MAX; break;
	case FixedConst::USHORT : overflow = value > USHRT_MAX; break;
	case FixedConst::UINT : overflow = value > UINT_MAX; break;
	default: break;
	}

	if (overflow) {
	    Ymir::Error::overflow (this-> token, name (this-> type));
	}
	return value;
    }

    long IFixed::convertS () {
	char * temp; errno = 0;
	long value = std::strtol (removeUnder ().c_str (), &temp, 0);
	bool overflow = false;
	if (temp == this-> token.getStr ().c_str () || *temp != '\0' ||
	    ((value == LONG_MIN || value == LONG_MAX) && errno == ERANGE)) {
	    overflow = true;
	}
	
	switch (this-> type) {
	case FixedConst::BYTE : overflow = value > SCHAR_MAX || value < SCHAR_MIN; break;
	case FixedConst::SHORT : overflow = value > SHRT_MAX || value < SHRT_MIN; break;
	case FixedConst::INT : overflow = value > INT_MAX || value < INT_MIN; break;
	default: break;
	}

	if (overflow) {
	    Ymir::Error::overflow (this-> token, name (this-> type));
	} 
	return value;
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
    
    char IChar::toChar () {
	return code;
    }

    std::string IChar::prettyPrint () {
	return Ymir::OutBuffer ((char) this-> code).str ();
    }

    IFloat::IFloat (Word word) : IExpression (word), _type (FloatConst::DOUBLE) {
	this-> totale = "0." + word.getStr ();
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
	this-> totale = "0." + this-> token.getStr ();
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

    std::string IDColon::prettyPrint () {
	return Ymir::OutBuffer (this-> left-> prettyPrint (), "::", this-> right-> prettyPrint ()).str ();
    }

    Expression IDColon::getLeft () {
	return this-> left;
    }
    
    Expression IDColon::getRight () {
	return this-> right;
    }
    
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

    std::string IDot::prettyPrint () {
	return Ymir::OutBuffer (this-> left-> prettyPrint (), ".", this-> right-> prettyPrint ()).str ();
    }
    
    IDot::~IDot () {
	delete left;
	delete right;
    }
    
    bool IDot::isLvalue () {
	return true;
    }
    
    Expression IDot::getLeft () {
	return this-> left;
    }   

    Expression IDot::getRight () {
	return this-> right;
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

    std::string IPar::prettyPrint () {
	return Ymir::OutBuffer (this-> _left-> prettyPrint (), "(", this-> params-> prettyPrint (), ")").str ();	
    }

    bool IPar::isLvalue () {
	if (this-> _score-> proto && this-> _score-> proto-> isLvalue ())
	    return true;
	return this-> info-> type ()-> is<IRefInfo> ();
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

    IProto::IProto (Word ident, Expression type, Word retDeco, const std::vector<Var> & params, std::string space, bool isVariadic) :
	_type (type),
	_retDeco (retDeco),
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
    
    Expression& IProto::type () {
	return this-> _type;
    }

    Word & IProto::retDeco () {
	return this-> _retDeco;
    }

    std::string IProto::name () {
	return this-> ident.getStr ();
    }

    std::string & IProto::externLang () {
	return this-> from;
    }

    std::string & IProto::externLangSpace () {
	return this-> space;
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
	this-> info = new (Z0)  ISymbol (locus, this, info);
    }

    Expression ITreeExpression::templateExpReplace (const std::map <std::string, Expression>&) {
	return this;
    }

    Expression ITreeExpression::expression () {
	return this;
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

    IFor::IFor (Word token, Word id, const std::vector<Var> & var, Expression iter, Block bl, std::vector <bool> _const) :
	IInstruction (token),
	id (id),
	var (var),
	iter (iter),
	block (bl),
	_const (_const)
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

    Expression IUnary::getElem () {
	return this-> elem;
    }
    
    std::string IUnary::prettyPrint () {
	Ymir::OutBuffer buf (this-> token.getStr ());
	buf.write ("(", this-> elem-> prettyPrint (), ")");
	return buf.str ();
    }
    
    bool IUnary::isLvalue () {
	if (this-> token == Token::AND) return false;
	return true;
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
	if (this-> left)
	    this-> left-> inside = this;
	if (this-> params)
	    this-> params-> inside = this;
    }

    IStructCst::IStructCst (Word word, Word end) :
	IExpression (word),
	end (end)
    {
    }

    Expression IStructCst::getLeft () {
	return this-> left;
    }
    
    std::vector <Expression> & IStructCst::getExprs () {
	return this-> params-> getParams ();
    }
    
    std::string IStructCst::prettyPrint () {
	Ymir::OutBuffer buf (this-> left-> prettyPrint (), "{", this-> params-> prettyPrint (), "}");
	return buf.str ();
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

    bool IAccess::isLvalue () {
	return true;
    }
    
    std::string IAccess::prettyPrint () {
	return Ymir::OutBuffer (this-> left-> prettyPrint (), "[", this-> params-> prettyPrint (), "]").str ();	
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

    std::string IConstTuple::prettyPrint () {
	Ymir::OutBuffer buf ("(");
	for (auto it : Ymir::r (0, params.size ())) {
	    buf.write (params [it]);
	    if (it < (int) params.size () - 1)
		buf.write (", ");
	}
	if (params.size () == 1)
	    buf.write (",");
	buf.write (")");
	return buf.str ();
    }    
    
    IConstTuple::~IConstTuple () {
	for (auto it : params)
	    delete it;
	for (auto it : casters)
	    delete it;
    }

    std::vector <semantic::InfoType> & IConstTuple::getCasters () {
	return this-> casters;
    }
    
    std::vector <Expression> & IConstTuple::getExprs () {
	return this-> params;
    }

    bool& IConstTuple::isFake () {
	return this-> _isFake;
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

    std::string IExpand::prettyPrint () {
	if (auto tu = this-> expr-> to <IConstTuple> ()) {
	    return tu-> getExprs () [this-> it]-> prettyPrint ();
	} else return this-> expr-> prettyPrint ();
    }
    
    IReturn::IReturn (Word ident) : IInstruction (ident), elem (NULL), caster (NULL) {}
    
    IReturn::IReturn (Word ident, Expression elem) :
	IInstruction (ident),	
	elem (elem),
	caster (NULL)
    {}

    Expression IReturn::getExpr () {
	return this-> elem;
    }

    bool& IReturn::isUseless () {
	return this-> _isUseless;
    }
    
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

    IArrayVar::IArrayVar (Word token, Expression content, Expression len) :
	IVar (token),
	content (content),
	len (len)
    {}
    
    IArrayVar::~IArrayVar () {
	if (content) delete content;
	if (len) delete len;
    }
    
    Expression IArrayVar::contentExp () {
	return this-> content;
    }

    Expression IArrayVar::getLen () {
	return this-> len;
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
	    ret-> info = new (Z0)  ISymbol (this-> info-> sym, ret, this-> info-> type () ? this-> info-> type ()-> clone () : NULL);
	return ret;
    }

    Expression IExpression::templateExpReplace (const std::map <std::string, Expression>&) {
	this-> print (0);
	println (this-> token);
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

    bool IExpression::isLvalue () {
	return false;
    }
    
    ILambdaFunc::ILambdaFunc (Word begin, std::vector <Var> params, Var type, Block block) : 
	IExpression (begin),
	params (params),
	ret (type),
	block (block),
	id (getLastNb ())
    {}

    ILambdaFunc::ILambdaFunc (Word begin, std::vector <semantic::Frame> frame) : 
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
    
    bool& ILambdaFunc::isMoved () {
	return this-> _isMoved;
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


    IFuncPtr::IFuncPtr (Word begin, std::vector <Expression> params, Expression type, Expression expr) :
	IExpression (begin),
	params (params),
	ret (type),
	expr (expr)
    {
	this-> ret-> inside = this;
	if (this-> expr)
	    this-> expr-> inside = this;
    }    

    std::vector <Expression> & IFuncPtr::getParams () {
	return this-> params;
    }

    Expression IFuncPtr::getRet () {
	return this-> ret;
    }

    Expression IFuncPtr::body () {
	return this-> expr;
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
    

    IStruct::IStruct (Word ident, std::vector <Expression> tmps, std::vector <Var> params, std::vector <Word> udas, bool isUnion) :
	ident (ident),
	params (params),
	tmps (tmps),
	_udas (udas),
	_isUnion (isUnion)
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

    std::string IIs::prettyPrint () {
	if (this-> type)
	    return Ymir::OutBuffer ("is (", this-> left-> prettyPrint (), " : ", this-> type-> prettyPrint (), ")").str ();
	else
	    return Ymir::OutBuffer ("is (", this-> left-> prettyPrint (), " : ", this-> expType.getStr (), ")").str ();
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
    
    IEnum::IEnum (Word ident, Expression type, std::vector <Word> names, std::vector <Expression> values) :
	ident (ident),
	type (type),
	names (names),
	values (values)
    {
	this-> is_public (true);
    }

    IEnum::~IEnum () {
	for (auto it : values)
	    delete it;
    }

    IAlias::IAlias (Word ident, Expression value) :
	_ident (ident),
	_value (value)
    {
	this-> is_public (true);
    }

    void IAlias::print (int) {	
    }
    
    ITypeOf::ITypeOf (Word begin, Expression expr, bool mut) :
	IExpression (begin),
	expr (expr),
	_mut (mut)
    {}

    std::string ITypeOf::prettyPrint () {
	Ymir::OutBuffer buf ("typeof (");
	if (this-> _mut) buf.write ("mut ");
	buf.write (this-> expr-> prettyPrint ());
	buf.write (")");
	return buf.str ();
    }
    
    ITypeOf::~ITypeOf () {
	delete expr;
    }

    IStringOf::IStringOf (Word begin, Expression expr) :
	IExpression (begin),
	expr (expr)
    {}
    
    IStringOf::~IStringOf () {
	delete expr;
    }

    IMatchPair::IMatchPair (Word token, Expression left, Expression right) :
	IExpression (token),
	left (left),
	right (right)
    {}

    Expression IMatchPair::getLeft () {
	return this-> left;
    }

    Expression IMatchPair::getRight () {
	return this-> right;
    }    
    
    IMatchPair::~IMatchPair () {
	delete left;
	delete right;
    }
    	
    IMatch::IMatch (Word word, Expression expr, std::vector<Expression> values, std::vector <Block> block) :
	IExpression (word),
	expr (expr),
	values (values),
	block (block)
    {}
    
    IMatch::IMatch (Word word, Expression expr) :
	IExpression (word),
	expr (expr)
    {}

    IMatch::~IMatch () {
	delete expr;
	for (auto it : values)
	    delete it;
	for (auto it : block)
	    delete it;
    }
    
    IGlobal::IGlobal (Word ident, Expression type, bool isExternal) :
	ident (ident),
	expr (NULL),
	type (type),
	isExternal (isExternal)
    {}

    IGlobal::IGlobal (Word ident, Expression expr, Expression type) :
	ident (ident),
	expr (expr),
	type (type)
    {}

    bool & IGlobal::isImut () {
	return this-> _isImut;
    }

    bool IGlobal::fromC () {
	return this-> from == "C";
    }

    Expression IGlobal::getExpr () {
	return this-> expr;
    }
    
    IGlobal::~IGlobal () {
	if (type) delete type;
	if (expr) delete expr;
    }

    
    
    IConstructor::IConstructor (Word token, std::vector <Var> params, Block bl) :
	IFunction (Word (token.getLocus(), token.getStr () + "__cst__"),
		   {},
		   params,
		   {}, NULL, bl)	    	    
    {
	this-> params.insert (this-> params.begin (), new (Z0) IVar (token));
    }
    
	
    std::vector <Expression> & IModDecl::getTemplates () {
	return this-> tmps;
    }

    std::vector <Declaration> & IModDecl::getDecls () {
	return this-> decls;
    }
    
    IScope::IScope (Word token, Block block) :
	IInstruction (token),
	block (block)
    {}

    Expression IConstArray::getParam (int nb) {
	return this-> params [nb];
    }

    std::string IConstArray::prettyPrint () {
	Ymir::OutBuffer buf ("[");
	for (auto it : Ymir::r (0, this-> params.size ())) {
	    buf.write (this-> params [it]);
	    if (it < (int) this-> params.size () - 1)
		buf.write (", ");
	}
	buf.write ("]");
	return buf.str ();
    }

    IPragma::IPragma (Word token, ParamList params) :
	IExpression (token),
	params (params)
    {}

    std::string IPragma::prettyPrint () {
	return "";
    }
    
    IMacroExpr::IMacroExpr (Word, Word, std::vector <MacroElement> elements) :
	elements (elements)
    {}

    std::vector <MacroElement> &IMacroExpr::getElements () {
	return this-> elements;
    }
    
    IMacro::IMacro (Word ident, std::vector <MacroExpr> exprs, std::vector <Block> blocks) :
	ident (ident),
	_exprs (exprs),
	_blocks (blocks)
    {}
        
    std::vector <MacroExpr>& IMacro::getExprs () {
	return this-> _exprs;
    }

    std::vector <Block>& IMacro::getBlocks () {
	return this-> _blocks;
    }

    std::vector <Word> IMacroElement::toTokens (bool& success) {
	success = false;
	return {};
    }

    const char* IMacroElement::id () {
	return "IMacroElement";
    }
	
    std::vector <std::string> IMacroElement::getIds () {
	auto ids = IExpression::getIds ();
	ids.push_back (IMacroElement::id ());
	return ids;
    }
    
    IMacroVar::IMacroVar (Word name, MacroVarConst type) :
	IMacroElement (name),
	type (type),
	_token (NULL),
	name (name)
    {}

    IMacroVar::IMacroVar (Word name, MacroToken type) :
	IMacroElement (name),
	type (MacroVarConst::TOKEN),
	_token (type),
	name (name)
    {}

    
    std::vector <Word> IMacroVar::toTokens (bool& success) {
	if (auto elem = this-> content-> to<IMacroElement> ()) {
	    return elem-> toTokens (success);
	} else if (this-> content-> to<IVar> () && this-> type == MacroVarConst::IDENT) {
	    return {this-> content-> token};
	}
	return IMacroElement::toTokens (success);
    }
    
    std::string IMacroVar::prettyPrint () {
	if (this-> content) {
	    return Ymir::OutBuffer ("[", this-> content-> prettyPrint (), "]").str ();
	} else {
	    return Ymir::OutBuffer (this-> name, ": null").str (); 
	}
    }
    
    const char* IMacroVar::id () {
	return "IMacroVar";
    }
	
    std::vector <std::string> IMacroVar::getIds () {
	auto ids = IMacroElement::getIds ();
	ids.push_back (IMacroVar::id ());
	return ids;
    }

    MacroVarConst IMacroVar::getType () {
	return this-> type;
    }

    MacroToken IMacroVar::getToken () {
	return this-> _token;
    }
    
    void IMacroVar::setContent (Expression expr) {
	this-> content = expr;
    }
    
    MacroVar IMacroVar::clone () {
	return new (Z0) IMacroVar (this-> name, this-> type);
    }
    
    IMacroToken::IMacroToken (Word tok, std::string value) :
	IMacroElement (tok),
	value (value)
    {}

    std::vector <Word> IMacroToken::toTokens (bool& success) {
	success = true;
	return {{this-> token, this-> value}};	
    }
    
    std::string IMacroToken::prettyPrint () {
	return this-> value;
    }

    std::string & IMacroToken::getValue () {
	return this-> value;
    }
    
    const char* IMacroToken::id () {
	return "IMacroToken";
    }

    std::vector <std::string> IMacroToken::getIds () {
	auto ids = IMacroElement::getIds ();
	ids.push_back (IMacroToken::id ());
	return ids;
    }
    
    MacroToken IMacroToken::clone () {
	return new (Z0) IMacroToken (this-> token, this-> value);
    }
       
    IMacroRepeat::IMacroRepeat (Word ident, MacroExpr content, MacroToken pass, bool oneTime) :
	IMacroElement (ident),
	content (content),
	pass (pass),
	oneTime (oneTime),
	ident (ident)
    {}

    std::vector <Word> IMacroRepeat::toTokens (bool& success) {
	std::vector <Word> tokens;
	success = true;
	for (auto soluce : this-> soluce) {
	    for (auto it : soluce.elements) {
		if (auto elem = it.second-> to<IMacroElement> ()) {
		    auto current = elem-> toTokens (success);
		    if (!success) return IMacroElement::toTokens (success);
		    else {
			tokens.insert (tokens.end (), current.begin (), current.end ());
		    }
		}
	    }
	}
	return tokens;
    }
    
    std::string IMacroRepeat::prettyPrint () {
	Ymir::OutBuffer buf ("(");
	for (auto it : this-> soluce) {
	    buf.write (it.elements);
	}
	buf.write (")");
	return buf.str ();
    }
        
    MacroRepeat IMacroRepeat::clone () {
	return new (Z0) IMacroRepeat (this-> ident, this-> content, this-> pass, this-> oneTime);
    }
    
    MacroToken IMacroRepeat::getClose () {
	return this-> pass;
    }

    MacroExpr IMacroRepeat::getExpr () {
	return this-> content;
    }

    bool IMacroRepeat::isOneTime () {
	return this-> oneTime;
    }

    void IMacroRepeat::addSolution (semantic::MacroSolution soluce) {
	this-> soluce.push_back (soluce);
    }

    std::vector <semantic::MacroSolution>& IMacroRepeat::getSolution () {
	return this-> soluce;
    }
    
    const char* IMacroRepeat::id () {
	return "IMacroRepeat";
    }
	
    std::vector <std::string> IMacroRepeat::getIds () {
	auto ids = IMacroElement::getIds ();
	ids.push_back (IMacroRepeat::id ());
	return ids;
    }

    IMacroEnum::IMacroEnum (Word name, std::vector <MacroExpr> elems) :
	IMacroElement (name),
	_elems (elems)
    {}

    semantic::MacroSolution& IMacroEnum::getSoluce () {
	return this-> soluce;
    }

    const std::vector <MacroExpr> & IMacroEnum::getElems () {
	return this-> _elems;
    }
    
    std::vector<Word> IMacroEnum::toTokens (bool& success) {
	std::vector <Word> tokens;
	success = true;
	for (auto it : this-> soluce.elements) {
	    if (auto elem = it.second-> to <IMacroElement> ()) {
		auto current = elem-> toTokens (success);
		if (!success) return IMacroElement::toTokens (success);
		else {
		    tokens.insert (tokens.end (), current.begin (), current.end ());
		}
	    } 
	}
	return tokens;
    }

    std::string IMacroEnum::prettyPrint () {
	Ymir::OutBuffer buf ("(");
	buf.write (this-> soluce.elements);	
	buf.write (")");
	return buf.str ();
    }

    MacroEnum IMacroEnum::clone () {
	return new (Z0) IMacroEnum (this-> token, this-> _elems);
    }
    
    const char* IMacroEnum::id () {
	return "IMacroEnum";
    }

    std::vector <std::string> IMacroEnum::getIds () {
	auto ids = IMacroElement::getIds ();
	ids.push_back (IMacroEnum::id ());
	return ids;
    }
    
    IMacroCall::IMacroCall (Word begin, Word end, Expression left, std::vector<Word> content) :
	IExpression (begin),
	end (end),
	left (left),
	content (content)
    {}

    std::vector <Word> & IMacroCall::getTokens () {
	return this-> content;
    }

    std::string IMacroCall::prettyPrint () {
	if (this-> bl)
	    return this-> bl-> prettyPrint ();
	else {
	    Ymir::OutBuffer buf (this-> left-> prettyPrint (), ": {");
	    for (auto it : this-> content)
		buf.write (it.getStr ());
	    buf.write ("}");
	    return buf.str ();
	}
    }
    
    void IMacroCall::setSolution (std::map <std::string, Expression> exprs) {
	this-> soluce = exprs;
    }
    
    std::vector <std::string> IMacroCall::getIds () {
	auto ret = IExpression::getIds ();
	ret.push_back (TYPEID (IMacroCall));
	return ret;
    }

    MacroCall IMacroCall::solve (const std::map <std::string, Expression> & values) {
	auto expr = this-> left-> expression ();
	if (expr == NULL) return NULL;	
	auto mac = expr-> info-> type ()-> to <IMacroInfo> ();
	if (mac == NULL) {
	    Ymir::Error::notAMacro (this-> left-> token);
	    return NULL;
	}
	
	auto soluce = mac-> resolve (this);
	if (!soluce.valid) {
	    Ymir::Error::macroResolution (this-> left-> token);
	    return NULL;
	}

	if (!semantic::Table::instance ().addCall (this-> token)) return NULL;
	semantic::Table::instance ().enterPhantomBlock ();
	Table::instance ().retInfo ().info = new (Z0) ISymbol (this-> token, NULL, new (Z0) IVoidInfo ());

	Block block = (Block) soluce.block-> templateExpReplace (soluce.elements);
	if (!block) return NULL;
	
	block = (Block) block-> templateExpReplace (values);
	if (!block) return NULL;

	semantic::Table::instance ().quitFrame ();
	auto aux = new (Z0) IMacroCall (this-> token, this-> end, this-> left-> templateExpReplace ({}), this-> content);
	aux-> setSolution (soluce.elements);
	aux-> bl = block;	
	return aux;
    }

    IAffectGeneric::IAffectGeneric (Word word, Expression left, Expression right, bool addr) :
	IExpression (word),
	left (left),
	right (right),
	_addr (addr)
    {}

    IFakeDecl::IFakeDecl (Word word, Var left, Expression right, bool const_, bool addr) :
	IExpression (word),
	left (left),
	right (right),
	_addr (addr),
	_const (const_)
    {}

    ITypeCreator::ITypeCreator (Word ident, TypeForm form, const std::vector <Expression> & who, const std::vector <Expression> & tmps, bool isUnion) {
	this-> _ident = ident;
	this-> _form = form;
	this-> _who = who;
	this-> _tmps = tmps;
	this-> _isUnion = isUnion;
    }

    std::vector <TypeConstructor> & ITypeCreator::getConstructors () {
	return this-> _constr;
    }

    std::vector <TypeDestructor> & ITypeCreator::getDestructors () {
	return this-> _destr;
    }

    std::vector <TypeMethod> & ITypeCreator::getMethods () {
	return this-> _methods;
    }

    std::vector <TypeAlias> & ITypeCreator::getAlias () {
	return this-> _alias;
    }
    
    ITypeConstructor::ITypeConstructor (Word ident, const std::vector <Var> & params, Block block, bool isCopy) :
	_ident (ident),
	_params (params),
	_block (block),
	_isCopy (isCopy)
    {}

    InnerProtection& ITypeConstructor::getProtection () {
	return this-> _prot;
    }

    std::vector <Var> & ITypeConstructor::getParams () {
	return this-> _params;
    }

    Block& ITypeConstructor::getBlock () {
	return this-> _block;
    }

    Word ITypeConstructor::getIdent () {
	return this-> _ident;
    }
    
    ITypeDestructor::ITypeDestructor (Word ident, Block block)
	:
	_ident (ident),
	_block (block)
    {}

    Block & ITypeDestructor::getBlock () {
	return this-> _block;
    }

    Word & ITypeDestructor::getIdent () {
	return this-> _ident;
    }
    
    InnerProtection& ITypeDestructor::getProtection () {
	return this-> _prot;
    }
    
    ITypeMethod::ITypeMethod (Function func, bool over) 
	: IFunction (*func),
	  _isOver (over)
    {}

    InnerProtection& ITypeMethod::getProtection () {
	return this-> _prot;
    }

    ITypeAlias::ITypeAlias (Word ident, Expression value, bool isConst) :
	_ident (ident),
	_value (value),	
	_isConst (isConst),
	_space ("")
    {
    }

    Word ITypeAlias::getIdent () {
	return this-> _ident;
    }

    Expression ITypeAlias::getValue () {
	return this-> _value;
    }

    bool ITypeAlias::isConst () {
	return this-> _isConst;
    }

    InnerProtection & ITypeAlias::getProtection () {
	return this-> _prot;
    }
    
    bool ITypeAlias::isPrivate () {
	return this-> _prot == InnerProtection::PRIVATE;
    }
    
    bool ITypeAlias::isProtected () {
	return this-> _prot == InnerProtection::PROTECTED;
    }
    
    semantic::Namespace & ITypeAlias::space () {
	return this-> _space;
    }

    IEvaluatedExpr::IEvaluatedExpr (Expression value) :
	IExpression (value-> token),
	_value (value)
    {}

    void IEvaluatedExpr::print (int) {}
    
}
