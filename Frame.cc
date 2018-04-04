#include "semantic/pack/Frame.hh"
#include <ymir/semantic/types/_.hh>
#include <ymir/semantic/pack/Table.hh>
#include <ymir/utils/Range.hh>
#include <ymir/ast/Var.hh>
#include <ymir/ast/Return.hh>
#include <ymir/ast/TypedVar.hh>
#include "ast/ParamList.hh"
#include "errors/Error.hh"
#include <ymir/ast/Function.hh>
#include <ymir/syntax/Keys.hh>
#include <ymir/utils/Mangler.hh>
#include <ymir/semantic/pack/FrameTable.hh>

namespace semantic {
    using namespace syntax;

    const long IFrame::CONST_SAME = 18;
    const long IFrame::SAME = 20;
    const long IFrame::CONST_AFF = 8;    
    const long IFrame::AFF = 10;
    const long IFrame::CHANGE = 14;
    const long IFrame::CONST_CHANGE = 12;			       
    
    IFrame::IFrame (Namespace space, ::syntax::Function func) :
	_function (func),
	_space (space)
    {
	if (func) this-> _attributes = func-> getAttributes ();
    }
    
    ApplicationScore IFrame::isApplicable (::syntax::ParamList params) {
	return this-> isApplicable (
	    this-> _function-> getIdent (),
	    this-> _function-> getParams (),
	    params-> getParamTypes ()
	);
    }

    ApplicationScore IFrame::isApplicable (const std::vector<InfoType> & params) {
	return this-> isApplicable (
	    this-> _function-> getIdent (),
	    this-> _function-> getParams (),
	    params
	);
    }

    Frame IFrame::TempOp (const std::vector<syntax::Expression> &) {
	return NULL;
    }
    
    ApplicationScore IFrame::getScore (Word ident, const std::vector <Var>& attrs, const std::vector <InfoType>& args) {
	auto score = new (Z0)  IApplicationScore (ident);
	if (attrs.size () == 0 && args.size () == 0) {
	    score-> score = AFF;
	    score-> score += this-> currentScore ();
	    return score;
	} else if (attrs.size () == args.size ()) {
	    for (auto it : Ymir::r (0, args.size ())) {
		auto CONST_SAME = this-> CONST_SAME;
		auto SAME = this-> SAME;
		InfoType info = NULL;
		auto param = attrs [it];
		if (auto tvar = param-> to<ITypedVar> ()) {
		    auto getType = tvar-> getType ();
		    if (getType == NULL) return NULL;
		    info = getType-> clone ();
		} else {
		    tvar = param-> setType (args [it]-> clone ());
		    info = tvar-> getType ()-> clone ();
		    CONST_SAME = this-> CONST_CHANGE;
		    SAME = this-> CHANGE;
		}
		
		auto type = args [it]-> CompOp (info);
		if (type != NULL) type = type-> ConstVerif (info);
		else return NULL;
		if (type && type-> isSame (args [it])) {
		    if (args [it]-> isConst () != info-> isConst ())
			score-> score += CONST_SAME;
		    else score-> score += SAME;
		    score-> treat.push_back (type);		
		} else if (type != NULL) {
		    bool changed = false;
		    if (auto ref = args [it]-> to<IRefInfo> ()) {
			if (ref-> content ()-> isSame (type)) 
			    changed = true;			   			
		    }
		    
		    if (args [it]-> isConst () != info-> isConst ())
			score-> score += changed ? CONST_CHANGE : CONST_AFF;
		    else score-> score += changed ? CHANGE : AFF;
		    score-> treat.push_back (type);
		} else return NULL;
	    }
	    
	    score-> score += this-> currentScore ();
	    return score;
	}
	return NULL;
    }
    

    ApplicationScore IFrame::isApplicable (Word ident, const std::vector<Var> & attrs, const std::vector <InfoType>& args) {
	if (this-> _isPrivate && !this-> _space.isSubOf (Table::instance ().space ()))
	    return NULL;

	auto globSpace = Table::instance ().space ();
	Table::instance ().setCurrentSpace (Namespace (this-> _space, ident.getStr ()));

	auto score = getScore (ident, attrs, args);
	Table::instance ().setCurrentSpace (globSpace);
	return score;
    }
	
    FrameProto IFrame::validate (const std::vector<Var> & finalParams) {
	//Table::instance ().setCurrentSpace (Namespace (this-> _space, this-> _function-> getIdent ().getStr ()));

	if (this-> _function-> getType () == NULL)
	    Table::instance ().retInfo ().info = new (Z0)  ISymbol (Word::eof (), new (Z0)  IUndefInfo ());
	else {
	    auto var = this-> _function-> getType ()-> asType ();
	    if (var != NULL) Table::instance ().retInfo ().info = var-> info;
	    else Table::instance ().retInfo ().info = new (Z0)  ISymbol (Word::eof (), new (Z0) IVoidInfo ());
	    Table::instance ().retInfo ().deco = this-> _function-> getType ()-> deco.getStr ();
	    if (Table::instance ().retInfo ().deco != Keys::REF && Table::instance ().retInfo ().deco != Keys::MUTABLE)
		Table::instance ().retInfo ().info-> isConst (true);
	    else
		Table::instance ().retInfo ().info-> isConst (false);
	}
	
	auto proto = new (Z0)  IFrameProto (this-> _function-> name (), this-> _space, Table::instance ().retInfo ().info, finalParams, this-> tempParams, this-> _attributes);

	if (!FrameTable::instance ().existsProto (proto)) {
	    if (!Table::instance ().retInfo ().info-> type-> is <IUndefInfo> ())
		Table::instance ().retInfo ().isImmutable () = true;

	    FrameTable::instance ().insert (proto);
	    Table::instance ().retInfo ().currentBlock () = "true";
	    auto block = this-> _function-> getBlock ()-> block ();
	    
	    if (Table::instance ().retInfo ().info-> type-> is<IUndefInfo> ())
		Table::instance ().retInfo ().info-> type = new (Z0)  IVoidInfo ();	    
	    auto finFrame = new (Z0)  IFinalFrame (Table::instance ().retInfo ().info,
					    this-> _space, this-> _function-> name (),
					    finalParams, block, this-> tempParams);
	    
	    finFrame-> closure () = Table::instance ().retInfo ().closure;
	    finFrame-> isInline () = this-> has (Keys::INLINE);
	    
	    proto-> type () = Table::instance ().retInfo ().info;
	    proto-> attached () = finFrame;
	    proto-> closure () = Table::instance ().retInfo ().closure;
	    FrameTable::instance ().insert (finFrame);

	    finFrame-> file () = LOCATION_FILE (this-> _function-> getIdent ().getLocus ());
	    Table::instance ().quitBlock ();

	    verifyReturn (this-> _function-> getIdent (), proto-> type (), Table::instance ().retInfo ());
	    Table::instance ().quitFrame ();
	    return proto;
	}
	
	Table::instance ().quitBlock ();
	Table::instance ().quitFrame ();
	return FrameTable::instance ().getProto (proto);	
    }
    
    FrameProto IFrame::validate () {
	Ymir::Error::assert ("TODO");
	return NULL;
    }

    FrameProto IFrame::validate (::syntax::ParamList) {
	Ymir::Error::assert ("TODO");
	return NULL;
    }

    FrameProto IFrame::validate (const std::vector<InfoType> &) {
	Ymir::Error::assert ("TODO");
	return NULL;
    }    
    
    FrameProto IFrame::validate (ApplicationScore score, const std::vector<InfoType> & params) {
	if (score-> tmps.size () == 0)
	    return validate (params);
	Ymir::Error::assert ("TODO");
	return NULL;
    }


    void IFrame::verifyReturn (Word loc, Symbol ret, FrameReturnInfo info) {
	if (!ret-> type-> is<IVoidInfo> () && !ret-> type-> is<IUndefInfo> ()) {
	    if (!info.retract ()) {
		Ymir::Error::missingReturn (loc, ret);
	    }
	}
    }

    std::vector <::syntax::Var> IFrame::computeParams (const std::vector<Var> & attr, const std::vector<InfoType>& params) {
	std::vector <Var> finalParams;
	for (auto it : Ymir::r (0, attr.size ())) {
	    if (auto var = attr [it] -> to<ITypedVar> ()) {
		finalParams.push_back (var-> var ());
		if (finalParams.back () == NULL)
		    finalParams.pop_back ();
	    } else {		
		auto v = attr [it]-> setType (params [it]);
		finalParams.push_back (v-> var ());
		if (finalParams.back () == NULL)
		    finalParams.pop_back ();
	    }
	}
	return finalParams;
    }
    
    std::vector <::syntax::Var> IFrame::computeParams (const std::vector<::syntax::Var> & params) {
	std::vector <Var> finalParams;
	for (auto it : Ymir::r (0, params.size ())) {
	    auto info = params [it]-> var ();
	    if (info != NULL) 
		finalParams.push_back (info);
	}
	return finalParams;
    }

    std::vector <::syntax::Var> IFrame::copyParams (const std::vector <::syntax::Var> & params) {
	std::vector <Var> finalParams;
	for (auto it : params) {
	    finalParams.push_back (new (Z0) IType (it-> token, it-> info-> type-> clone ()));
	}
	return finalParams;
    }
    
    FrameProto IFrame::validate (Word name, Namespace space, Namespace from, Symbol ret, const std::vector<Var> & params, Block block, const std::vector <Expression>& tmps, bool isVariadic) {
	//Table::instance ().setCurrentSpace (Namespace (space, name.getStr ()));
	struct Exit {
	    Namespace last;
	    Frame self;

	    Exit (Frame self) : last (Table::instance ().templateNamespace ()), self (self) {
	    }
	    
	    FrameProto operator () (Word name, Namespace space, Namespace from, Symbol ret, const std::vector<Var> & params, Block block, const std::vector <Expression>& tmps, bool isVariadic) {
		Table::instance ().templateNamespace () = from;
		Namespace finalNamespace (space, from.toString ());
		if (ret == NULL) 
		    Table::instance ().retInfo ().info = new (Z0)  ISymbol (Word::eof (), new (Z0)  IUndefInfo ());
		else
		    Table::instance ().retInfo ().info = ret;
		
		auto proto = new (Z0)  IFrameProto (name.getStr (), finalNamespace, Table::instance ().retInfo ().info, params, tmps, self-> _attributes);
		if (!FrameTable::instance ().existsProto (proto)) {
		    if (!Table::instance ().retInfo ().info-> type-> is <IUndefInfo> ())
			Table::instance ().retInfo ().isImmutable () = true;
		    
		    FrameTable::instance ().insert (proto);

		    Table::instance ().retInfo ().currentBlock () = "true";
		    block = block-> block ();

		    if (Table::instance ().retInfo ().info-> type-> is <IUndefInfo> ())
			Table::instance ().retInfo ().info-> type = new (Z0)  IVoidInfo ();

		    auto finFrame = new (Z0)  IFinalFrame (Table::instance ().retInfo ().info,
							   finalNamespace, name.getStr (),
							   params, block, tmps);
		    
		    finFrame-> isVariadic () = isVariadic;
		    finFrame-> isInline () = self-> has (Keys::INLINE);
		    
		    proto-> type () = Table::instance ().retInfo ().info;
		    finFrame-> closure () = Table::instance ().retInfo ().closure;
		    proto-> closure () = Table::instance ().retInfo ().closure;
		    proto-> attached () = finFrame;
		    FrameTable::instance ().insert (finFrame);

		    finFrame-> file () = name.getFile ();
		    Table::instance ().quitBlock ();
		    verifyReturn (name, proto-> type (), Table::instance ().retInfo ());
		    Table::instance ().quitFrame ();
		    return proto;
		}
		Table::instance ().quitBlock ();
		Table::instance ().quitFrame ();
		return FrameTable::instance ().getProto (proto);	
	    }

	    ~Exit () {
		Table::instance ().templateNamespace () = last;
	    }
	    
	};

	Exit ex (this);
	return ex (name, space, from, ret, params, block, tmps, isVariadic);	
    }

    FrameProto IFrame::validate (std::string& name, Namespace space, const std::vector<Var> & params, Expression _block) {
	//Table::instance ().setCurrentSpace (Namespace (space, name));
	Table::instance ().retInfo ().info = new (Z0) ISymbol (Word::eof (), new (Z0) IUndefInfo ());

	auto proto = new (Z0) IFrameProto (name, space, Table::instance ().retInfo ().info, params, {}, this-> _attributes);
	
	if (!FrameTable::instance ().existsProto (proto)) {
	    if (!Table::instance ().retInfo ().info-> type-> is <IUndefInfo> ())
		Table::instance ().retInfo ().isImmutable () = true;

	    FrameTable::instance ().insert (proto);
	    Table::instance ().retInfo ().currentBlock () = "true";
	    auto exprBlock = _block-> expression ();
	    if (exprBlock == NULL) return NULL;
	    
	    Table::instance ().retInfo ().info-> type = exprBlock-> info-> type;
	    Block block = NULL;
	    if (!exprBlock-> info-> type-> is<IVoidInfo> ()) {
		auto ret = new (Z0) IReturn (_block-> token, exprBlock);
		ret-> getCaster () = exprBlock-> info-> type-> CompOp (Table::instance ().retInfo ().info-> type);
		block = new (Z0) IBlock (exprBlock-> token, {}, {ret});
	    } else {		    
		block = new (Z0) IBlock (exprBlock-> token, {}, {exprBlock});
	    }
	    
	    auto finFrame = new (Z0) IFinalFrame (Table::instance ().retInfo ().info,
						  space, name, params, block, {});
	    proto-> type () = Table::instance ().retInfo ().info;
	    finFrame-> closure () = Table::instance ().retInfo ().closure;
	    finFrame-> isInline () = this-> has (Keys::INLINE);
	    
	    proto-> closure () = Table::instance ().retInfo ().closure;
	    proto-> attached () = finFrame;

	    FrameTable::instance ().insert (finFrame);
	    finFrame-> file () = LOCATION_FILE (_block-> token.getLocus ());
	    Table::instance ().quitBlock ();
	    Table::instance ().quitFrame ();
	    return proto;
	}
	Table::instance ().quitBlock ();
	Table::instance ().quitFrame ();
	return FrameTable::instance ().getProto (proto);	
    }

    FrameProto IFrame::validate (std::string& name, Namespace space, const std::vector<Var> & params, Block _block) {
	//Table::instance ().setCurrentSpace (Namespace (space, name));
	Table::instance ().retInfo ().info = new (Z0) ISymbol (Word::eof (), new (Z0) IUndefInfo ());

	auto proto = new (Z0) IFrameProto (name, space, Table::instance ().retInfo ().info, params, {}, this-> _attributes);
	
	if (!FrameTable::instance ().existsProto (proto)) {
	    if (!Table::instance ().retInfo ().info-> type-> is <IUndefInfo> ())
		Table::instance ().retInfo ().isImmutable () = true;

	    FrameTable::instance ().insert (proto);
	    Table::instance ().retInfo ().currentBlock () = "true";
	    auto block = _block-> block ();	    

	    if (Table::instance ().retInfo ().info-> type-> is <IUndefInfo> ())
		Table::instance ().retInfo ().info-> type = new (Z0)  IVoidInfo ();
	    
	    auto finFrame = new (Z0) IFinalFrame (Table::instance ().retInfo ().info,
						  space, name, params, block, {});
	    proto-> type () = Table::instance ().retInfo ().info;
	    finFrame-> closure () = Table::instance ().retInfo ().closure;
	    finFrame-> isInline () = this-> has (Keys::INLINE);
	    
	    proto-> closure () = Table::instance ().retInfo ().closure;
	    proto-> attached () = finFrame;

	    FrameTable::instance ().insert (finFrame);
	    finFrame-> file () = LOCATION_FILE (_block-> token.getLocus ());
	    Table::instance ().quitBlock ();
	    Table::instance ().quitFrame ();
	    return proto;
	}
	Table::instance ().quitBlock ();
	Table::instance ().quitFrame ();
	return FrameTable::instance ().getProto (proto);	
    }

    
    
    FrameProto IFrame::validate (Namespace space, Namespace from, const std::vector<Var> & params, bool isVariadic) {
	//Table::instance ().setCurrentSpace (Namespace (space, this-> _function-> getIdent ().getStr ()));
	struct Exit {
	    Namespace last;
	    Frame self;

	    Exit (Frame self) : last (Table::instance ().templateNamespace ()), self (self) {
	    }

	    FrameProto operator () (Namespace space, Namespace from, const std::vector<Var> & params, bool isVariadic) {
		Namespace finalNamespace (space, from.toString ());
		Table::instance ().templateNamespace () = from;
		
		if (self-> _function-> getType () == NULL) 
		    Table::instance ().retInfo ().info = new (Z0)  ISymbol (Word::eof (), new (Z0)  IUndefInfo ());
		else {
		    auto type = self-> _function-> getType ()-> asType ();
		    if (type) Table::instance ().retInfo ().info = type-> info;
		    else Table::instance ().retInfo ().info = new (Z0)  ISymbol (Word::eof (), new (Z0) IVoidInfo ());
		    
		    Table::instance ().retInfo ().deco = self-> _function-> getType ()-> deco.getStr ();
		    if (Table::instance ().retInfo ().deco != Keys::REF && Table::instance ().retInfo ().deco != Keys::MUTABLE)
			Table::instance ().retInfo ().info-> isConst (true);
		}

		auto proto = new (Z0)  IFrameProto (self-> _function-> getIdent ().getStr (), finalNamespace, Table::instance ().retInfo ().info, params, self-> tempParams, self-> _attributes);
		
		if (!FrameTable::instance ().existsProto (proto)) {
		    if (!Table::instance ().retInfo ().info-> type-> is <IUndefInfo> ())
			Table::instance ().retInfo ().isImmutable () = true;

		    FrameTable::instance ().insert (proto);
		    Table::instance ().retInfo ().currentBlock () = "true";
		    auto block = self-> _function-> getBlock ()-> block ();

		    if (Table::instance ().retInfo ().info-> type-> is <IUndefInfo> ())
			Table::instance ().retInfo ().info-> type = new (Z0)  IVoidInfo ();

		    auto finFrame = new (Z0)  IFinalFrame (Table::instance ().retInfo ().info,
							   finalNamespace, self-> _function-> getIdent ().getStr (),
							   params, block, self-> tempParams);

		    finFrame-> isVariadic () = isVariadic;
		    finFrame-> isInline () = self-> has (Keys::INLINE);
		    
		    proto-> type () = Table::instance ().retInfo ().info;
		    finFrame-> closure () = Table::instance ().retInfo ().closure;
		    proto-> closure () = Table::instance ().retInfo ().closure;
		    proto-> attached () = finFrame;
		    
		    FrameTable::instance ().insert (finFrame);

		    finFrame-> file () = self-> _function-> getIdent ().getFile ();
		    Table::instance ().quitBlock ();
		    verifyReturn (self-> _function-> getIdent (), proto-> type (), Table::instance ().retInfo ());
		    Table::instance ().quitFrame ();
		    return proto;
		}
		Table::instance ().quitBlock ();
		Table::instance ().quitFrame ();
		return FrameTable::instance ().getProto (proto);	
	    }
	    
	    ~Exit () {
		Table::instance ().templateNamespace () = last;
	    }
	    
	};
	Exit ex (this);
	return ex (space, from, params, isVariadic);
    }
    
    
    
    Namespace& IFrame::space () {
	return this-> _space;
    }

    int& IFrame::currentScore () {
	return this-> _currentScore;
    }

    bool& IFrame::isInternal () {
	return this-> _isInternal;
    }

    bool IFrame::isVariadic () const {
	return this-> _isVariadic;
    }

    void IFrame::isVariadic (bool isVariadic) {
	this-> _isVariadic = isVariadic;
    }

    ::syntax::Function IFrame::func () {
	return this-> _function;
    }

    bool IFrame::isPure () {
	return false;
    }

    std::vector <InfoType> IFrame::getParamTypes () {
	return {};	
    }

    InfoType IFrame::getRetType () {
	return NULL;
    }
    
    bool& IFrame::isPrivate () {
	return this-> _isPrivate;
    }
    
    Word IFrame::ident () {
	return this-> _function-> getIdent ();
    }

    bool IFrame::has (std::string attrs) {
	for (auto it : this-> _attributes)
	    if (it == attrs) return true;
	return NULL;
    }
    
    std::vector <syntax::Expression> & IFrame::templateParams () {
	return this-> tempParams;
    }

    std::vector <Word> & IFrame::attributes () {
	return this-> _attributes;
    }
    
    std::string IFrame::toString () {
	Ymir::OutBuffer buf;
	buf.write ("(");
	for (auto it : Ymir::r (0, this-> _function-> getParams ().size ())) {
	    auto var = this-> _function-> getParams () [it];
	    buf.write (var-> prettyPrint ());
	    if (it < (int) this-> _function-> getParams ().size () - 1)
		buf.write (", ");
	}
	
	buf.write (")");
	
	if (auto t = this-> _function-> getType ())
	    buf.write ("-> ", t-> prettyPrint ());
	return buf.str ();
    }
    

}
