#include <ymir/ast/_.hh>
#include <ymir/semantic/pack/Table.hh>
#include <ymir/semantic/pack/MacroSolver.hh>
#include <ymir/syntax/Keys.hh>
#include <ymir/syntax/Token.hh>
#include <ymir/semantic/pack/Table.hh>


namespace syntax {

    using namespace semantic;

    ulong ILambdaFunc::__nbLambda__ = 0;

    std::list <Block> IBlock::__currentBlock__;

    
    std::string IInstruction::prettyPrint () {	
	Ymir::OutBuffer buf ("TODO {", this-> getIds (), "}");
	Ymir::Error::assert (buf.str ().c_str ());	
	return "";    
    }
    
    Word& IBlock::getIdent () {
	return this-> _ident;
    }

    std::string IBlock::prettyPrint () {
	Ymir::OutBuffer buf ("{\n");
	for (auto it : this-> _insts) {	    
	    buf.write (it-> prettyPrint ());
	    if (it-> is<IExpression> ())
		buf.write (";");
	    buf.write ("\n");
	}
	buf.write ("}\n");
	return buf.str ();
    }
    
    void IBlock::addFinally (Block block) {
	if (block-> _insts.size () != 0) {
	    if (!block-> _insts [0]-> is<INone> () || block-> _insts.size () != 1)
		this-> _finally.push_back (block);
	}
    }

    void IBlock::addFailure (FailureBlock block) {
	this-> _failures.push_back (block);
    }
    
    void IBlock::addFinallyAtSemantic (Instruction inst) {
	this-> _preFinally.push_back (inst);
    }
    
    void IBlock::addInline (Var var) {
	this-> _inlines.push_back (var);
    }
        
    std::vector <Instruction>& IBlock::getInsts () {
	return this-> _insts;
    }
    
    Block IBlock::getCurrentBlock () {
	return __currentBlock__.front ();
    }

    IBlock::~IBlock ()  {
	for (auto it : _decls)
	    delete it;
	    
	for (auto it : _insts)
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
    
    std::string infoname (FixedConst ct) {
	switch (ct) {
	case FixedConst::BYTE : return "I8_info";
	case FixedConst::UBYTE : return "U8_info";
	case FixedConst::SHORT : return "I16_info";
	case FixedConst::USHORT: return "U16_info";
	case FixedConst::INT : return "I32_info";
	case FixedConst::UINT : return "U32_info";
	case FixedConst::LONG : return "I64_info";
	case FixedConst::ULONG : return "U64_info";
	}
	return "none";
    }

    
    IFunction::IFunction (Word ident, const std::string & docs, const std::vector <Word> & attrs, const std::vector<Var> & params, const std::vector <Expression>& tmps, Expression test, Block block) :
	IDeclaration (docs),
	_ident (ident),
	_type (NULL),
	_params (params),
	_tmps (tmps),
	_attrs (attrs),
	_block (block),
	_test (test)
    {
	this-> setPublic (true);
    }    
	
    IFunction::IFunction (Word ident, const std::string & docs, const std::vector <Word> & attrs, Expression type, Word retDeco, const std::vector<Var> & params, const std::vector <Expression>& tmps, Expression test, Block block) :
	IDeclaration (docs),
	_ident (ident),
	_type (type),
	_retDeco (retDeco),
	_params (params),
	_tmps (tmps),
	_attrs (attrs),
	_block (block),
	_test (test)	    
    {
	this-> setPublic (true);
    }

    Ymir::json IFunction::generateDocs () {
	Ymir::json o;
	o ["name"] = this-> _ident.getStr ();
	o ["line"] = this-> _ident.line;
	o ["char"] = this-> _ident.column;
	o ["comment"] = this-> getDocs ();
	
	if (this-> _frame && this-> _frame-> is <IPureFrame> ()) {
	    o ["kind"] = "function";
	    auto proto = this-> _frame-> validate ();
	    o ["type"] = proto-> type ()-> typeString ();
	    for (auto it : proto-> vars ()) {
		Ymir::json param;
		param ["name"] = it-> token.getStr ();
		param ["type"] = it-> info-> typeString ();
		o ["parameters"].push (param);
	    }	    
	} else {
	    o ["kind"] = "template";
	    if (this-> _type) 
		o ["type"] = this-> _retDeco.getStr () + this-> _type-> prettyPrint ();
	    
	    for (auto it : this-> _params) {
		Ymir::json param;
		param ["name"] = it-> token.getStr ();
		if (it-> is <ITypedVar> ()) 
		    param ["type"] = it-> to <ITypedVar> ()-> typeExp ()-> prettyPrint ();
		o ["parameters"].push (param);
	    }

	    for (auto it : this-> _tmps) {
		Ymir::json param;
		param ["value"] = it-> prettyPrint ();
		if (it-> is <IOfVar> ()) 
		    param ["kind"] = "of_var";
		else if (it-> is <IVariadicVar> ())
		    param ["kind"] = "variadic_var";
		else if (it-> is <IVar> ())
		    param ["kind"] = "variable";
		else param ["kind"] = "const";
		o ["members"].push (param);
	    }
	}
	return o;
    }
    
    IFunction::~IFunction () {
	if (this-> _type) delete this-> _type;
	for (auto it : this-> _params)
	    delete it;
	for (auto it : this-> _tmps)
	    delete it;
	
	if (this-> _block) delete this-> _block;
	if (this-> _test) delete this-> _test;
    }
       
    Word IFunction::getIdent () {
	return this-> _ident;
    }

    std::vector <Var>& IFunction::params () {
	return this-> _params;
    }
    
    const std::vector <Var>& IFunction::getParams () {
	return this-> _params;
    }
    
    std::vector <Expression>& IFunction::templates () {
	return this-> _tmps;
    }
    
    const std::vector <Expression>& IFunction::getTemplates () {
	return this-> _tmps;
    }

    const std::vector <Word> & IFunction::getAttributes () {
	return this-> _attrs;
    }
    
    Expression IFunction::getTest () {
	return this-> _test;
    }
    
    Expression IFunction::getType () {
	return this-> _type;
    }

    Word IFunction::getRetDeco () {
	return this-> _retDeco;
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

    const std::string& IFunction::getName () {
	return this-> _ident.getStr ();
    }

    void IFunction::setName (std::string &other) {
	this-> _ident.setStr (other);
    }

    void IFunction::setName (const char* other) {
	this-> _ident.setStr (other);
    }
    
    Block IFunction::getBlock () {
	return this-> _block;
    }

    bool IFunction::has (const std::string & str) {
	for (auto & it : this-> _attrs) {
	    if (it.getStr () == str) return true;
	}
	return false;
    }
    
    IParamList::IParamList (Word ident, const std::vector<Expression> & params) :
	IExpression (ident),
	_params (params)
    {}
    
    std::string IParamList::prettyPrint () {
	Ymir::OutBuffer buf;
	for (auto it : Ymir::r (0, this-> _params.size ())) {
	    buf.write (this-> _params [it]-> prettyPrint ());
	    if (it < (int) this-> _params.size () - 1)
		buf.write (", ");
	}
	return buf.str ();
    }    

    std::string IFuncPtr::prettyPrint () {
	Ymir::OutBuffer buf (this-> token.getStr (), " (");
	for (auto it : Ymir::r (0, this-> _params.size ())) {
	    buf.write (this-> _params [it]-> prettyPrint ());
	    if (it < (int) this-> _params.size () - 1)
		buf.write (", ");
	}
	buf.write (")-> ");
	buf.write (this-> _ret-> prettyPrint ());
	return buf.str ();
    }
    
    IParamList::~IParamList () {
	for (auto it : this-> _params)
	    delete it;
    }
    
    std::vector <Expression>& IParamList::params () {
	return this-> _params;
    }
    
    const std::vector <Expression>& IParamList::getParams () {
	return this-> _params;
    }
    
    std::vector <semantic::InfoType>& IParamList::treats () {
	return this-> _treats;
    }

    const std::vector <semantic::InfoType>& IParamList::getTreats () {
	return this-> _treats;
    }
    
    std::vector <semantic::InfoType> IParamList::getParamTypes () {
	std::vector<semantic::InfoType> paramTypes;
	for (auto it : this-> _params) {
	    it-> info-> type ()-> symbol () = it-> info;
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
	_type (type),
	_size (size),
	_isImmutable (isImmutable)
    {
	if (this-> _size)
	    this-> _size-> inside = this;
	if (this-> _type)
	    this-> _type-> inside = this;
    }    

    Expression IArrayAlloc::getType () {
	return this-> _type;
    }

    Expression IArrayAlloc::getSize () {
	return this-> _size;
    }
    
    std::string IArrayAlloc::prettyPrint () {
	return Ymir::format ("[% ; %]", this-> _type-> prettyPrint ().c_str (),
			     this-> _size-> prettyPrint ().c_str ());
    }

    IArrayAlloc::~IArrayAlloc () {
	delete _type;
	_type = NULL;
	delete _size;
	_size = NULL;
	if (_cster)
	    delete _cster;
    }

    
    std::vector <std::string> IArrayAlloc::getIds () {
	auto ret = IExpression::getIds ();
	ret.push_back (TYPEID(IArrayAlloc));
	return ret;
    }
    
    IAssert::IAssert (Word token, Expression test, Expression msg, bool isStatic) :
	IInstruction (token),
	_expr (test),
	_msg (msg)
    {
	this-> _expr-> inside = this;
	if (this-> _msg)
	    this-> _msg-> inside = this;
	this-> _isStatic = isStatic;
    }

    IAssert::~IAssert ()  {
	delete _expr;
	if (_msg)
	    delete _msg;
    }
    
    IBinary::IBinary (Word word, Expression left, Expression right, Expression ctype) :
	IExpression (word),
	_left (left),
	_right (right),
	_autoCaster (ctype)
    {
	if (this-> _left) this-> _left-> inside = this;
	if (this-> _right) this-> _right-> inside = this;	    
    }

    IBinary::~IBinary () {
	if (this-> _left) delete _left;
	if (this-> _right) delete _right;	    
    }        
    
    Expression& IBinary::getLeft () {
	return this-> _left;
    }

    Expression& IBinary::getRight () {
	return this-> _right;
    }

    Expression& IBinary::getAutoCast () {
	return this-> _autoCaster;
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
	return Ymir::OutBuffer ("(", this-> _left, " ", this-> token.getStr (), " ", this-> _right, ")").str ();
    }
    
    IFixed::IFixed (Word word, FixedConst type) :
	IExpression (word),
	_type (type),
	_mode (FixedMode::BUILTINS)
    {
	if (this-> _mode == FixedMode::DECIMAL) {
	    if (isSigned (this-> _type)) this-> _value = this-> convertS ();	
	    else this-> _uvalue = this-> convertU ();
	}
    }

    IFixed::IFixed (Word word, FixedConst type, FixedMode mode) :
	IExpression (word),
	_type (type),
	_mode (mode)
    {
	if (this-> _mode == FixedMode::DECIMAL) {
	    if (isSigned (this-> _type)) this-> _value = this-> convertS ();	
	    else this-> _uvalue = this-> convertU ();
	} else if (this-> _mode == FixedMode::HEXA) {
	    if (isSigned (this-> _type)) this-> _value = this-> convertSX ();
	    else this-> _uvalue = this-> convertUX ();
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
	
	switch (this-> _type) {
	case FixedConst::UBYTE : overflow = value > UCHAR_MAX; break;
	case FixedConst::USHORT : overflow = value > USHRT_MAX; break;
	case FixedConst::UINT : overflow = value > UINT_MAX; break;
	default: break;
	}

	if (overflow) {
	    Ymir::Error::overflow (this-> token, name (this-> _type));
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
	
	switch (this-> _type) {
	case FixedConst::BYTE : overflow = value > SCHAR_MAX || value < SCHAR_MIN; break;
	case FixedConst::SHORT : overflow = value > SHRT_MAX || value < SHRT_MIN; break;
	case FixedConst::INT : overflow = value > INT_MAX || value < INT_MIN; break;
	default: break;
	}

	if (overflow) {
	    Ymir::Error::overflow (this-> token, name (this-> _type));
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
	
	switch (this-> _type) {
	case FixedConst::UBYTE : overflow = value > UCHAR_MAX; break;
	case FixedConst::USHORT : overflow = value > USHRT_MAX; break;
	case FixedConst::UINT : overflow = value > UINT_MAX; break;
	default: break;
	}

	if (overflow) {
	    Ymir::Error::overflow (this-> token, name (this-> _type));
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
	
	switch (this-> _type) {
	case FixedConst::BYTE : overflow = value > SCHAR_MAX || value < SCHAR_MIN; break;
	case FixedConst::SHORT : overflow = value > SHRT_MAX || value < SHRT_MIN; break;
	case FixedConst::INT : overflow = value > INT_MAX || value < INT_MIN; break;
	default: break;
	}

	if (overflow) {
	    Ymir::Error::overflow (this-> token, name (this-> _type));
	} 
	return value;
    }
    
    void IFixed::setUValue (ulong val) {
	this-> _uvalue = val;
    }

    void IFixed::setValue (long val) {
	this-> _value = val;
    }


    std::string IFixed::prettyPrint () {
	if (isSigned (this-> _type))
	    return Ymir::OutBuffer (this-> _value).str ();
	else
	    return Ymir::OutBuffer (this-> _uvalue).str ();
    }
    
    
    IChar::IChar (Word word, ubyte code) :
	IExpression (word),
	_code (code) {
    }
    
    char IChar::toChar () {
	return this-> _code;
    }

    std::string IChar::prettyPrint () {
	return Ymir::OutBuffer ((char) this-> _code).str ();
    }

    IFloat::IFloat (Word word) : IExpression (word), _type (FloatConst::DOUBLE) {
	this-> _totale = "0." + word.getStr ();
    }
    
    IFloat::IFloat (Word word, std::string suite) :
	IExpression (word),
	_suite (suite),
	_type (FloatConst::DOUBLE)
    {
	this-> _totale = this-> token.getStr () + "." + suite;
    }

    IFloat::IFloat (Word word, std::string suite, FloatConst type) :
	IExpression (word),
	_suite (suite),
	_type (type)
    {
	this-> _totale = this-> token.getStr () + "." + suite;
    }

    IFloat::IFloat (Word word, FloatConst type) :
	IExpression (word),
	_suite (""),
	_type (type)
    {
	this-> _totale = "0." + this-> token.getStr ();
    }

    void IFloat::setValue (float val) {
	this-> _totale = Ymir::OutBuffer (val).str ();
    }

    void IFloat::setValue (double val) {
	this-> _totale = Ymir::OutBuffer (val).str ();
    }    

    std::string IFloat::getValue () {
	return this-> _totale;
    }
    
    std::string IFloat::prettyPrint () {
	return this-> _totale;
    }

    IBool::IBool (Word token) : IExpression (token) {
	this-> _value = token == Keys::TRUE_;
    }
    
    std::string IBool::prettyPrint () {	
	return this-> token.getStr ();
    }

    bool &IBool::value () {
	return this-> _value;
    }
    
    std::string INull::prettyPrint () {
	return "null";
    }

    std::string IIgnore::prettyPrint () {
	return "_";
    }
    
    IString::IString (Word word, std::string content) :
	IExpression (word),
	_content (content)
    {}
    
    std::string IString::getStr () {
	return this-> _content;
    }

    std::string IString::prettyPrint () {
	return Ymir::OutBuffer ("\"", this-> _content, "\"").str ();
    }
    
    std::vector <std::string> IString::getIds () {
	auto ret = IExpression::getIds ();
	ret.push_back (TYPEID (IString));
	return ret;
    }
    
    IBreak::IBreak (Word token) : IInstruction (token) {
	this-> _ident.setEof ();
    }

    IBreak::IBreak (Word token, Word ident) :
	IInstruction (token),
	_ident (ident) {
    }

    
    ICast::ICast (Word begin, Expression type, Expression expr) :
	IExpression (begin),
	_type (type),
	_expr (expr) {
	this-> _type-> inside = this;
	this-> _expr-> inside = this;
    }

    ICast::~ICast () {
	delete _type;
	delete _expr;
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
    
    int IConstArray::getNbParams () {
	return this-> params.size ();
    }    
	    
    IDColon::IDColon (Word token, Expression left, Expression right) :
	IExpression (token),
	_left (left),
	_right (right)
    {}

    std::string IDColon::prettyPrint () {
	return Ymir::OutBuffer (this-> _left-> prettyPrint (), "::", this-> _right-> prettyPrint ()).str ();
    }

    Expression IDColon::getLeft () {
	return this-> _left;
    }
    
    Expression IDColon::getRight () {
	return this-> _right;
    }
    
    IDColon::~IDColon () {	
	delete this-> _left;
	delete this-> _right;	
    }
    
    
    IDot::IDot (Word word, Expression left, Expression right) :
	IExpression (word),
	_left (left),
	_right (right)
    {
	if (this-> _left) this-> _left-> inside = this;
	//if (this-> right) this-> _right-> inside = this;
    }

    std::string IDot::prettyPrint () {
	return Ymir::OutBuffer (this-> _left-> prettyPrint (), ".", this-> _right-> prettyPrint ()).str ();
    }
    
    IDot::~IDot () {
	delete this-> _left;
	delete this-> _right;
    }
    
    bool IDot::isLvalue () {
	return true;
    }
    
    Expression IDot::getLeft () {
	return this-> _left;
    }   

    Expression IDot::getRight () {
	return this-> _right;
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
	_end (end),
	_params (NULL),
	_left (NULL),
	_dotCall (NULL),
	_opCall (false),
	_score (NULL)
    {}

    IPar::IPar (Word word, Word end, Expression left, ParamList params, bool fromOpCall) :
	IExpression (word),
	_end (end),
	_params (params),
	_left (left),
	_dotCall (NULL),
	_opCall (fromOpCall),
	_score (NULL)
    {
	this-> _left-> inside = this;
	this-> _params-> inside = this;	    	    
    }

    std::string IPar::prettyPrint () {
	return Ymir::OutBuffer (this-> _left-> prettyPrint (), "(", this-> _params-> prettyPrint (), ")").str ();	
    }

    bool IPar::isLvalue () {
	if (this-> _score-> proto && this-> _score-> proto-> isLvalue ())
	    return true;
	return this-> info-> type ()-> is<IRefInfo> ();
    }
    
    IPar::~IPar () {
	delete this-> _params;
	delete this-> _left;
	if (this-> _dotCall) delete this-> _dotCall;
	if (this-> _score) delete this-> _score;
    }
    
    ParamList& IPar::paramList () {
	return this-> _params;
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
        
    IProto::IProto (Word ident, const std::string & docs, const std::vector<Var> & params, bool isVariadic) :
	IDeclaration (docs),
	_type (NULL),
	_params (params),
	_space (""),
	_isVariadic (isVariadic),
	_ident (ident)
    {}

    IProto::IProto (Word ident, const std::string & docs, Expression type, Word retDeco, const std::vector<Var> & params, std::string space, bool isVariadic) :
	IDeclaration (docs),
	_type (type),
	_retDeco (retDeco),
	_params (params),
	_space (space),
	_isVariadic (isVariadic),
	_ident (ident)
    {}

       
    IProto::~IProto () {
	if (this-> _type) delete this-> _type;
	for (auto it : this-> _params)
	    delete it;
    }
    
    Ymir::json IProto::generateDocs () {
	Ymir::json o;
	o ["name"] = this-> _ident.getStr ();
	o ["line"] = this-> _ident.line;
	o ["char"] = this-> _ident.column;
	o ["comment"] = this-> getDocs ();
	o ["kind"] = "function";
	auto proto = this-> _frame-> validate ();
	o ["type"] = proto-> type ()-> typeString ();
	for (auto it : proto-> vars ()) {
	    Ymir::json param;
	    param ["name"] = it-> token.getStr ();
	    param ["type"] = it-> info-> typeString ();
	    o ["parameters"].push (param);
	}
	o ["linkage"] = this-> _from;
	if (this-> _space != "")
	    o ["space"] = this-> _space;
	return o;
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

    const Word & IProto::getIdent () {
	return this-> _ident;
    }
    
    const std::string & IProto::getName () {
	return this-> _ident.getStr ();
    }

    std::string & IProto::externLang () {
	return this-> _from;
    }

    std::string & IProto::externLangSpace () {
	return this-> _space;
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

    ISemanticConst::ISemanticConst (Word locus, semantic::InfoType type, semantic::InfoType auxType) :
	IExpression (locus),
	_type (type),
	_auxType (auxType)
    {
	this-> info = new (Z0) ISymbol (locus, this, type);
    }

    Expression ISemanticConst::templateExpReplace (const std::map <std::string, Expression>&) {
	return this;
    }

    Expression ISemanticConst::expression () {
	return this;
    }
    
    void ISemanticConst::print (int) {}
    
    IIf::IIf (Word word, Expression test, Block block, bool isStatic) :
	IInstruction (word),
	_test (test),
	_block (block),
	_else (NULL)
    {
	if (this-> _test)
	    this-> _test-> inside = this;
	this-> _isStatic = isStatic;
    }
    
    IIf::IIf (Word word, Expression test, Block block, If else_, bool isStatic) : 
	IInstruction (word),
	_test (test),
	_block (block),
	_else (else_)
    {
	if (this-> _test)
	    this-> _test-> inside = this;
	this-> _isStatic = isStatic;
	if (this-> _else)
	    this-> _else-> _isStatic = isStatic;
    }    
    
    void IIf::print (int nb) {
	if (this-> _test) {
	    printf ("\n%*c<%sIf> %s",			
		    nb, ' ',
		    this-> _isStatic ? "static_" : "",
		    this-> token.toString ().c_str ()
	    );		
	    this-> _test-> print (nb + 4);		
	} else {
	    printf ("\n%*c<%sElse> %s",
		    nb, ' ',
		    this-> _isStatic ? "static_" : "",
		    this-> token.toString ().c_str ()
	    );
	}

	this-> _block-> print (nb + 4);
	if (this-> _else)
	    this-> _else-> print (nb + 8);	    
    }

    IFor::IFor (Word token, Word id, const std::vector<Var> & var, Expression iter, Block bl, std::vector <bool> _const) :
	IInstruction (token),
	_id (id),
	_var (var),
	_iter (iter),
	_block (bl),
	_const (_const)
    {
	this-> _iter-> inside = this;
    }

    
    IFor::~IFor ()  {
	for (auto it : this-> _var)
	    delete it;
	delete this-> _iter;
	delete this-> _block;
    }
    
    void IFor::print (int nb) {
	printf ("\n%*c<For> %s",
		nb, ' ',
		this-> token.toString ().c_str ()
	);
	for (auto it : this-> _var) {
	    it-> print (nb + 4);
	}

	this-> _iter-> print (nb + 5);
	this-> _block-> print (nb + 4);
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
    
    const std::vector <Expression> & IStructCst::getExprs () {
	return this-> params-> getParams ();
    }
    
    std::string IStructCst::prettyPrint () {
	Ymir::OutBuffer buf (this-> left-> prettyPrint (), "{", this-> params-> prettyPrint (), "}");
	return buf.str ();
    }
    
    IAccess::IAccess (Word word, Word end, Expression left, ParamList params) :
	IExpression (word),
	_end (end),
	_params (params),
	_left (left)	    
    {
	this-> _left-> inside = this;
	this-> _params-> inside = this;
    }
	
    IAccess::IAccess (Word word, Word end) :
	IExpression (word),
	_end (end)
    {
    }

    bool IAccess::isLvalue () {
	return true;
    }
    
    std::string IAccess::prettyPrint () {
	return Ymir::OutBuffer (this-> _left-> prettyPrint (), "[", this-> _params-> prettyPrint (), "]").str ();	
    }
    
    IAccess::~IAccess () {
	delete _params;
	delete _left;
	for (auto &it : _treats)
	    delete it;
	_treats.clear ();
    }
    
    Expression IAccess::getLeft () {
	return this-> _left;
    }
    
    std::vector <Expression> IAccess::getParams () {
	return this-> _params-> getParams ();
    }
       
    void IAccess::print (int nb) {
	printf ("\n%*c<Access> %s",
		nb, ' ',
		this-> token.toString ().c_str ()
	);
	this-> _left-> print (nb + 4);
	this-> _params-> print (nb + 4);
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
	_expr (expr)
    {}
	
    IExpand::IExpand (Word begin, Expression expr, ulong it) :
	IExpression (begin),
	_expr (expr),
	_it (it)
    {}
    
	
    IExpand::~IExpand () {
	delete this-> _expr;
    }
    
    void IExpand::print (int nb) {
	printf("\n%*c<Expand> %s",
	       nb, ' ',
	       this-> token.toString ().c_str ()
	);
	this-> _expr-> print (nb + 4);
    }

    std::string IExpand::prettyPrint () {
	if (auto tu = this-> _expr-> to <IConstTuple> ()) {
	    return tu-> getExprs () [this-> _it]-> prettyPrint ();
	} else return this-> _expr-> prettyPrint ();
    }
    
    IReturn::IReturn (Word ident) : IInstruction (ident), _elem (NULL), _caster (NULL) {}
    
    IReturn::IReturn (Word ident, Expression elem) :
	IInstruction (ident),	
	_elem (elem),
	_caster (NULL)
    {}

    Expression IReturn::getExpr () {
	return this-> _elem;
    }

    bool& IReturn::isUseless () {
	return this-> _isUseless;
    }
    
    InfoType& IReturn::caster () {
	return this-> _caster;
    }

    InfoType IReturn::getCaster () {
	return this-> _caster;
    }
    
    IReturn::~IReturn () {
	if (this-> _caster) delete this-> _caster;
	if (this-> _elem) delete this-> _elem;
    }
    
    void IReturn::print (int nb) {
	printf ("\n%*c<Return> %s",
		nb, ' ',
		this-> token.toString ().c_str ()
	);
	
	if (this-> _elem)
	    this-> _elem-> print (nb + 4);
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
	_params (params),
	_ret (type),
	_block (block),
	_id (getLastNb ())
    {}

    ILambdaFunc::ILambdaFunc (Word begin, std::vector <semantic::Frame> frame) : 
	IExpression (begin),	
	_params (),
	_ret (NULL),
	_block (NULL),
	_id (0),
	_frame (frame)
    {}

    
    ILambdaFunc::ILambdaFunc (Word begin, std::vector <Var> params, Block block) : 
	IExpression (begin),
	_params (params),
	_ret (NULL),
	_block (block),
	_id (getLastNb ())
    {}
	
    ILambdaFunc::ILambdaFunc (Word begin, std::vector <Var> params, Expression ret) :
	IExpression (begin),
	_params (params),
	_ret (NULL),
	_block (NULL),
	_expr (ret),
	_id (getLastNb ())
    {}

    std::string ILambdaFunc::prettyPrint () {
	auto ident = Ymir::OutBuffer ("Lambda_", this-> _id, " (");
	for (auto it : Ymir::r (0, this-> _params.size ())) {
	    ident.write (this-> _params [it]-> prettyPrint ());
	    if (it < (int) this-> _params.size () - 1)
		ident.write (", ");
	}
	ident.write (")");
	return ident.str ();
    }
    
    bool& ILambdaFunc::isMoved () {
	return this-> _isMoved;
    }
    
    const std::vector <Var> & ILambdaFunc::getParams () {
	return this-> _params;
    }
    
    Expression ILambdaFunc::getExpr () {
	return this-> _expr;
    }

    Block ILambdaFunc::getBlock () {
	return this-> _block;
    }
    
    ulong ILambdaFunc::getLastNb () {
	__nbLambda__ ++;
	return __nbLambda__;
    }
    
    ILambdaFunc::~ILambdaFunc () {
	for (auto it : this-> _params) delete it;
	if (this-> _ret) delete this-> _ret;
	if (this-> _block) delete this-> _block;
	if (this-> _expr) delete this-> _expr;
    }


    IFuncPtr::IFuncPtr (Word begin, std::vector <Expression> params, Expression type) :
	IExpression (begin),
	_params (params),
	_ret (type)
    {
	this-> _ret-> inside = this;
    }    


    // std::vector <Expression> & IFuncPtr::params () {
    // 	return this-> _params;
    // }

    const std::vector <Expression> & IFuncPtr::getParams () {
	return this-> _params;
    }

    Expression IFuncPtr::getRet () {
	return this-> _ret;
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
	for (auto it : this-> _params)
	    delete it;
	delete this-> _ret;
    }
    

    IStruct::IStruct (Word ident, const std::string & docs, std::vector <std::string> innerDocs, std::vector <Expression> tmps, std::vector <Var> params, std::vector <Word> udas, bool isUnion) :
	IDeclaration (docs),
	ident (ident),
	_innerDocs (innerDocs),
	params (params),
	tmps (tmps),
	_udas (udas),
	_isUnion (isUnion)
    {
	this-> _isPublic = true;
    }

    IStruct::~IStruct () {
	for (auto it : params)
	    delete it;
	for (auto it : tmps)
	    delete it;
    }

    Ymir::json IStruct::generateDocs () {
	Ymir::json o;
	o ["name"] = this-> ident.getStr ();
	o ["kind"] = "struct";
	o ["line"] = this-> ident.line;
	o ["char"] = this-> ident.column;
	if (this-> tmps.size () != 0) {
	    for (auto it : this-> tmps) {
		Ymir::json param;
		param ["value"] = it-> prettyPrint ();
		if (it-> is <IOfVar> ()) 
		    param ["kind"] = "of_var";
		else if (it-> is <IVariadicVar> ())
		    param ["kind"] = "variadic_var";
		else if (it-> is <IVar> ())
		    param ["kind"] = "variable";
		else param ["kind"] = "const";
		if (it-> is <IVar> ()) {
		    param ["name"] = it-> token.getStr ();
		}
		o ["parameters"].push (param);
	    }

	    for (auto it : Ymir::r (0, this-> params.size ())) {
		Ymir::json elem;
		elem ["name"] = this-> params [it]-> token.getStr ();
		elem ["type"] = this-> params [it]-> to <ITypedVar> ()-> typeExp ()-> prettyPrint ();
		elem ["comment"] = this-> _innerDocs [it];
		o ["members"].push (elem);
	    }
	    
	} else {
	    auto info = this-> _info-> type ()-> TempOp ({})-> to <IStructInfo> ();
	    for (auto it : Ymir::r (0, this-> params.size ())) {
		Ymir::json elem;
		elem ["name"] = this-> params [it]-> token.getStr ();
		elem ["type"] = info-> getTypes () [it]-> typeString ();
		elem ["comment"] = this-> _innerDocs [it];
		o ["members"].push (elem);
	    }
	}
	
	o ["comment"] = this-> getDocs ();
	return o;
    }
    
    IIs::IIs (Word begin, Expression expr, Expression type, std::vector <Expression> tmps) :
	IExpression (begin),
	_tmps (tmps),
	_left (expr),
	_type (type),
	_expType (Word::eof ())
    {
	if (this-> _left) this-> _left-> inside = this;
	if (this-> _type) this-> _type-> inside = this;
    }

    IIs::IIs (Word begin, Expression expr, Word type) :
	IExpression (begin),
	_left (expr),
	_type (NULL),
	_expType (type)
    {
	if (this-> _left) this-> _left-> inside = this;
    }

    std::string IIs::prettyPrint () {
	if (this-> _type)
	    return Ymir::OutBuffer ("is (", this-> _left-> prettyPrint (), " : ", this-> _type-> prettyPrint (), ")").str ();
	else
	    return Ymir::OutBuffer ("is (", this-> _left-> prettyPrint (), " : ", this-> _expType.getStr (), ")").str ();
    }
    
    IIs::~IIs () {
	delete this-> _left;
	if (this-> _type) delete this-> _type;
	for (auto it : this-> _tmps)
	    delete it;
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
    
    IEnum::IEnum (Word ident, const std::string & docs, Expression type, std::vector <std::string> docName, std::vector <Word> names, std::vector <Expression> values) :
	IDeclaration (docs),
	ident (ident),
	_type (type),
	_docs (docName),
	_names (names),
	_values (values)
    {
	this-> setPublic (true);
    }

    Ymir::json IEnum::generateDocs () {
	Ymir::json o;
	o ["name"] = this-> ident.getStr ();
	o ["kind"] = "enum";
	o ["line"] = this-> ident.line;
	o ["char"] = this-> ident.column;
	o ["comment"] = this-> getDocs ();
	if (this-> _info && this-> _info-> type ()-> is <IEnumCstInfo> ()) {
	    auto names = this-> _info-> type ()-> to <IEnumCstInfo> ()-> getNames ();
	    auto values = this-> _info-> type ()-> to <IEnumCstInfo> ()-> getValues ();
	    o ["baseDeco"] = this-> _info-> type ()-> to <IEnumCstInfo> ()-> type-> typeString ();
	    for (auto it : Ymir::r (0, names.size ())) {
		Ymir::json member;
		member ["kind"] = "enum member";
		member ["name"] = names [it];
		if (this-> _docs [it] != "") member ["comment"] = this-> _docs [it];
		if (values [it]-> info && values [it]-> info-> isImmutable ()) {
		    member ["value"] = values [it]-> info-> value ()-> toString ();
		} else {
		    member ["value"] = values [it]-> prettyPrint ();
		}
		
		o ["members"].push (member);
	    }
	}
	return o;
    }
    
    IEnum::~IEnum () {
	for (auto it : _values)
	    delete it;
    }

    IAlias::IAlias (Word ident, std::string & docs, Expression value) :
	IDeclaration (docs),
	_ident (ident),
	_value (value)
    {
	this-> setPublic (true);
    }

    Ymir::json IAlias::generateDocs () {
	Ymir::json o;
	o ["name"] = this-> _ident.getStr ();
	o ["kind"] = "variable";
	o ["line"] = this-> _ident.line;
	o ["char"] = this-> _ident.column;
	o ["comment"] = this-> getDocs ();
	o ["deco"] = "S5alias";
	o ["init"] = this-> _value-> prettyPrint ();
	return o;
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
    	
    IMatch::IMatch (Word word, Expression expr, const std::vector<Expression> &values, const std::vector <Block> &block) :
	IExpression (word),
	_expr (expr),
	_values (values),
	_block (block)
    {}
    
    IMatch::IMatch (Word word, Expression expr) :
	IExpression (word),
	_expr (expr)
    {}

    IMatch::~IMatch () {
	delete this-> _expr;
	for (auto it : this-> _values)
	    delete it;
	for (auto it : this-> _block)
	    delete it;
    }
    
    IGlobal::IGlobal (Word ident, const std::string & docs, Expression type, bool isExternal) :
	IDeclaration (docs),
	_ident (ident),
	_expr (NULL),
	_type (type),
	_isExternal (isExternal)
    {}

    IGlobal::IGlobal (Word ident, const std::string & docs, Expression expr, Expression type) :
	IDeclaration (docs),
	_ident (ident),
	_expr (expr),
	_type (type)
    {}

    Ymir::json IGlobal::generateDocs () {
	Ymir::json o;
	o ["name"] = this-> _ident.getStr ();
	o ["kind"] = "variable";
	if (this-> _isExternal) {
	    Ymir::json cl;
	    cl = "extern";
	    o ["storageClass"].push (cl);
	} else {
	    Ymir::json cl;
	    cl = "static";
	    o ["storageClass"].push (cl);
	}

	if (this-> _sym && this-> _sym-> type ()) {
	    o ["deco"] = this-> _sym-> typeString ();
	}

	if (this-> _expr) {
	    o ["init"] = this-> _expr-> prettyPrint ();
	}
	
	o ["comment"] = this-> getDocs ();
	
	return o;
    }
    
    bool & IGlobal::isImut () {
	return this-> _isImut;
    }

    void IGlobal::setFrom (const std::string & from) {
	this-> _from = from;
    }

    void IGlobal::setSpace (const std::string & space) {
	this-> _space = space;
    }
    
    bool IGlobal::isFromC () {
	return this-> _from == Keys::CLANG;
    }

    Expression IGlobal::getExpr () {
	return this-> _expr;
    }
    
    IGlobal::~IGlobal () {
	if (this-> _type) delete this-> _type;
	if (this-> _expr) delete this-> _expr;
    }
           
    Ymir::json IModDecl::generateDocs () {
	Ymir::json o;
	o ["name"] = this-> _ident.getStr ();
	
	if (this-> _tmps.size () == 0)
	    o ["kind"] = "module";
	else {
	    o ["kind"] = "template";
	    for (auto it : this-> _tmps) {
		Ymir::json param;
		param ["value"] = it-> prettyPrint ();
		if (it-> is <IOfVar> ()) 
		    param ["kind"] = "of_var";
		else if (it-> is <IVariadicVar> ())
		    param ["kind"] = "variadic_var";
		else if (it-> is <IVar> ())
		    param ["kind"] = "variable";
		else param ["kind"] = "const";
		if (it-> is <IVar> ()) {
		    param ["name"] = it-> token.getStr ();
		}
		
		o ["parameters"].push (param);
	    }
	}
	
	for (auto decl : this-> _decls) {
	    o ["members"].push (decl-> generateDocs ());
	}
	
	return o;
    }

    std::vector <Expression> & IModDecl::templates () {
	return this-> _tmps;
    }
    
    const std::vector <Expression> & IModDecl::getTemplates () {
	return this-> _tmps;
    }

    const std::vector <Declaration> & IModDecl::getDecls () {
	return this-> _decls;
    }
    
    IScope::IScope (Word token, Block block) :
	IInstruction (token),
	_block (block)
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
	_params (params)
    {}

    std::string IPragma::prettyPrint () {
	return "";
    }
    
    IMacroExpr::IMacroExpr (Word, Word, std::vector <MacroElement> elements) :
	_elements (elements)
    {}

    const std::vector <MacroElement> &IMacroExpr::getElements () {
	return this-> _elements;
    }
    
    IMacro::IMacro (Word ident, const std::string & docs, std::vector <std::string> innerDocs, const std::vector <MacroExpr> &exprs, const std::vector <Block> & blocks) :
	IDeclaration (docs),
	_ident (ident),
	_innerDocs (innerDocs),
	_exprs (exprs),
	_blocks (blocks)
    {}

    Ymir::json IMacro::generateDocs () {
	Ymir::json o;
	o ["name"] = this-> _ident.getStr ();
	o ["kind"] = "macro";
	o ["comment"] = this-> getDocs ();
	for (auto it : Ymir::r (0, this-> _exprs.size ())) {
	    Ymir::json expr = this-> _exprs [it]-> generateDocs ();;
	    expr ["comment"] = this-> _innerDocs [it];
	    o ["members"].push (expr);
	}
	return o;
    }

    Ymir::json IMacroExpr::generateDocs () {
	Ymir::json o;
	for (auto elem : this-> _elements) {
	    o ["parameters"].push (elem-> generateDocs ());
	}
	return o;
    }
    
    Ymir::json IMacroToken::generateDocs () {
	Ymir::json o;
	o ["name"] = this-> token.getStr ();
	o ["value"] = this-> _value;
	o ["kind"] = "macro token";
	return o;
    }

    Ymir::json IMacroRepeat::generateDocs () {
	Ymir::json o;
	o ["name"] = this-> token.getStr ();
	o ["kind"] = "macro repeat";
	o ["at_least_one_time"] = this-> _oneTime ? "true" : "false";
	o ["content"] = this-> _content-> generateDocs ();
	o ["pass"] = this-> _pass-> generateDocs ();
	return o;
    }

    Ymir::json IMacroVar::generateDocs () {
	Ymir::json o;
	o ["name"] = this-> name.getStr ();
	if (this-> _token)
	    o ["token"] = this-> _token-> generateDocs ();
	o ["kind"] = "macro var";
	switch (this-> _type) {
	case MacroVarConst::EXPR : o ["type"] = "expr"; break;
	case MacroVarConst::IDENT : o ["type"] = "ident"; break;
	case MacroVarConst::BLOCK : o ["type"] = "block"; break;
	case MacroVarConst::TOKEN : o ["type"] = "token"; break;
	}
	return o;
    }

    Ymir::json IMacroEnum::generateDocs () {
	Ymir::json o;
	o ["name"] = this-> token.getStr ();
	o ["kind"] = "enum";
	for (auto it : this-> _elems)
	    o ["members"].push (it-> generateDocs ());
	return o;
    }
    
    const std::vector <MacroExpr>& IMacro::getExprs () {
	return this-> _exprs;
    }

    const std::vector <Block>& IMacro::getBlocks () {
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
	_type (type),
	_token (NULL),
	name (name)
    {}

    IMacroVar::IMacroVar (Word name, MacroToken type) :
	IMacroElement (name),
	_type (MacroVarConst::TOKEN),
	_token (type),
	name (name)
    {}

    
    std::vector <Word> IMacroVar::toTokens (bool& success) {
	if (auto elem = this-> _content-> to<IMacroElement> ()) {
	    return elem-> toTokens (success);
	} else if (this-> _content-> to<IVar> () && this-> _type == MacroVarConst::IDENT) {
	    return {this-> _content-> token};
	}
	return IMacroElement::toTokens (success);
    }
    
    std::string IMacroVar::prettyPrint () {
	if (this-> _content) {
	    return Ymir::OutBuffer ("[", this-> _content-> prettyPrint (), "]").str ();
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
	return this-> _type;
    }

    MacroToken IMacroVar::getToken () {
	return this-> _token;
    }
    
    void IMacroVar::setContent (Expression expr) {
	this-> _content = expr;
    }
    
    MacroVar IMacroVar::clone () {
	return new (Z0) IMacroVar (this-> name, this-> _type);
    }
    
    IMacroToken::IMacroToken (Word tok, std::string value) :
	IMacroElement (tok),
	_value (value)
    {}

    std::vector <Word> IMacroToken::toTokens (bool& success) {
	success = true;
	return {{this-> token, this-> _value}};	
    }
    
    std::string IMacroToken::prettyPrint () {
	return this-> _value;
    }

    const std::string & IMacroToken::getValue () {
	return this-> _value;
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
	return new (Z0) IMacroToken (this-> token, this-> _value);
    }
       
    IMacroRepeat::IMacroRepeat (Word ident, MacroExpr content, MacroToken pass, bool oneTime) :
	IMacroElement (ident),
	_content (content),
	_pass (pass),
	_oneTime (oneTime),
	ident (ident)
    {}

    std::vector <Word> IMacroRepeat::toTokens (bool& success) {
	std::vector <Word> tokens;
	success = true;
	for (auto soluce : this-> _soluce) {
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
	for (auto it : this-> _soluce) {
	    buf.write (it.elements);
	}
	buf.write (")");
	return buf.str ();
    }
        
    MacroRepeat IMacroRepeat::clone () {
	return new (Z0) IMacroRepeat (this-> ident, this-> _content, this-> _pass, this-> _oneTime);
    }
    
    MacroToken IMacroRepeat::getClose () {
	return this-> _pass;
    }

    MacroExpr IMacroRepeat::getExpr () {
	return this-> _content;
    }

    bool IMacroRepeat::isOneTime () {
	return this-> _oneTime;
    }

    void IMacroRepeat::addSolution (semantic::MacroSolution soluce) {
	this-> _soluce.push_back (soluce);
    }

    const std::vector <semantic::MacroSolution>& IMacroRepeat::getSolution () {
	return this-> _soluce;
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
	return this-> _soluce;
    }

    const std::vector <MacroExpr> & IMacroEnum::getElems () {
	return this-> _elems;
    }
    
    std::vector<Word> IMacroEnum::toTokens (bool& success) {
	std::vector <Word> tokens;
	success = true;
	for (auto it : this-> _soluce.elements) {
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
	buf.write (this-> _soluce.elements);	
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
	_end (end),
	_left (left),
	_content (content)
    {}

    const std::vector <Word> & IMacroCall::getTokens () {
	return this-> _content;
    }

    std::string IMacroCall::prettyPrint () {
	if (this-> _bl)
	    return this-> _bl-> prettyPrint ();
	else {
	    Ymir::OutBuffer buf (this-> _left-> prettyPrint (), ": {");
	    for (auto it : this-> _content)
		buf.write (it.getStr ());
	    buf.write ("}");
	    return buf.str ();
	}
    }
    
    void IMacroCall::setSolution (std::map <std::string, Expression> exprs) {
	this-> _soluce = exprs;
    }
    
    std::vector <std::string> IMacroCall::getIds () {
	auto ret = IExpression::getIds ();
	ret.push_back (TYPEID (IMacroCall));
	return ret;
    }

    MacroCall IMacroCall::solve (const std::map <std::string, Expression> & values) {
	auto expr = this-> _left-> expression ();
	if (expr == NULL) return NULL;	
	auto mac = expr-> info-> type ()-> to <IMacroInfo> ();
	if (mac == NULL) {
	    Ymir::Error::notAMacro (this-> _left-> token);
	    return NULL;
	}
	
	auto soluce = mac-> resolve (this);
	if (!soluce.valid) {
	    Ymir::Error::macroResolution (this-> _left-> token);
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
	auto aux = new (Z0) IMacroCall (this-> token, this-> _end, this-> _left-> templateExpReplace ({}), this-> _content);
	aux-> setSolution (soluce.elements);
	aux-> _bl = block;	
	return aux;
    }

    IAffectGeneric::IAffectGeneric (Word word, Expression left, Expression right, bool addr) :
	IExpression (word),
	_left (left),
	_right (right),
	_addr (addr)
    {}

    IFakeDecl::IFakeDecl (Word word, Var left, Expression right, bool const_, bool addr) :
	IExpression (word),
	_left (left),
	_right (right),
	_addr (addr),
	_const (const_)
    {}

    ITypeCreator::ITypeCreator (Word ident, const std::string & docs, TypeForm form, const std::vector <Expression> & who, const std::vector <Expression> & tmps, bool isUnion) :
	IDeclaration (docs)
    {
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

    std::vector <Block> & ITypeCreator::getStaticConstructs () {
	return this-> _staticConstruct;
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

    ITypeAlias::ITypeAlias (Word ident, const std::string & docs, Expression value, bool isConst, bool isStatic) :
	IDeclaration (docs),
	_ident (ident),
	_value (value),	
	_isConst (isConst),
	_isStatic (isStatic),
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

    bool ITypeAlias::isStatic () {
	return this-> _isStatic;
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


    ITrait::ITrait (Word ident, std::string & docs, std::vector <TraitProto> protos, std::vector <TypedVar> attrs) :
	IDeclaration (docs),
	_ident (ident),
	_protos (protos),
	_attrs (attrs)
    {}

    std::vector <TraitProto> & ITrait::getProtos () {
	return this-> _protos;
    }

    std::vector <TypedVar> & ITrait::getAttrs () {
	return this-> _attrs;
    }

    Word & ITrait::getIdent () {
	return this-> _ident;
    }

    std::string & IDeclaration::docs () {
	return this-> _docs;
    }
    
    const std::string & IDeclaration::getDocs () {
	return this-> _docs;
    }

    Ymir::json IDeclaration::generateDocs () {
	return Ymir::json::undef ();
    }    

    Ymir::json IProgram::generateDocs () {
	Ymir::json o;
	std::string name = this-> _locus.getFile ();
	auto dot = name.find_last_of ('.');
	if (dot != name.npos && name.substr (dot, name.length () - dot) == ".yr") {
	    name = name.substr (0, dot);
	}
	
	o ["name"] = name;
	o ["file"] = this-> _locus.getFile ();
	o ["kind"] = "module";
	
	if (this-> _decls.size () != 0) {
	    if (auto mod = this-> _decls [0]-> to <IModDecl> ()) {
		if (mod-> isGlobal ()) o ["name"] = mod-> getIdent ().getStr ();
	    }
	}
	
	for (auto decl : this-> _decls) {
	    o ["members"].push (decl-> generateDocs ());
	}
		
	return o;
    }
    
    Ymir::json ISelf::generateDocs () {
	auto o = IFunction::generateDocs ();
	Ymir::json cl;
	cl = "static";
	o ["storageClass"].push (cl);
	return o;
    }

    Ymir::json IDestSelf::generateDocs () {
	auto o = IFunction::generateDocs ();
	Ymir::json cl;
	cl = "static";
	o ["storageClass"].push (cl);
	return o;
    }


    IFailureBlock::IFailureBlock (Word locus, Block block, TypedVar var) :
	IExpression (locus),
	_block (block),
	_var (var)
    {
    }

}
