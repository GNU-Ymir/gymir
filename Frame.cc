#include "semantic/pack/Frame.hh"
#include <ymir/semantic/types/_.hh>
#include <ymir/semantic/pack/Table.hh>
#include <ymir/utils/Range.hh>
#include <ymir/ast/Var.hh>
#include <ymir/ast/TypedVar.hh>
#include "ast/ParamList.hh"
#include "errors/Error.hh"
#include <ymir/ast/Function.hh>
#include <ymir/syntax/Keys.hh>
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
    {}
    
    ApplicationScore IFrame::isApplicable (::syntax::ParamList params) {
	return this-> isApplicable (this-> _function-> getIdent (),
				    this-> _function-> getParams (),
				    params-> getParamTypes ()
	);
    }
    
    ApplicationScore IFrame::isApplicable (std::vector <InfoType> params) {
	return this-> isApplicable (this-> _function-> getIdent (),
				    this-> _function-> getParams (),
				    params
	);
    }

    Frame IFrame::TempOp (std::vector <syntax::Expression>) {
	return NULL;
    }
    
    ApplicationScore IFrame::getScore (Word ident, const std::vector <Var>& attrs, const std::vector <InfoType>& args) {
	auto score = new IApplicationScore (ident);
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
		    tvar = param-> setType (new (GC) IUndefInfo ());
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
		    if (args [it]-> isConst () != info-> isConst ())
			score-> score += CONST_AFF;
		    else score-> score += AFF;
		    score-> treat.push_back (type);
		} else return NULL;		
	    }
	    score-> score += this-> currentScore ();
	    return score;
	}
	return NULL;
    }
    

    ApplicationScore IFrame::isApplicable (Word ident, std::vector <Var> attrs, std::vector <InfoType> args) {
	if (this-> _isPrivate && !this-> _space.isSubOf (Table::instance ().space ()))
	    return NULL;

	auto globSpace = Table::instance ().space ();
	Table::instance ().setCurrentSpace (Namespace (this-> _space, ident.getStr ()));

	auto score = getScore (ident, attrs, args);
	Table::instance ().setCurrentSpace (globSpace);
	return score;
    }
	
    FrameProto IFrame::validate (std::vector <Var> finalParams) {
	Table::instance ().setCurrentSpace (Namespace (this-> _space, this-> _function-> getIdent ().getStr ()));

	if (this-> _function-> getType () == NULL)
	    Table::instance ().retInfo ().info = new ISymbol (Word::eof (), new IUndefInfo ());
	else
	    Table::instance ().retInfo ().info = this-> _function-> getType ()-> asType ()-> info;
	
	auto proto = new IFrameProto (this-> _function-> name (), this-> _space, Table::instance ().retInfo ().info, finalParams, this-> tempParams);

	if (!FrameTable::instance ().existsProto (proto)) {
	    if (!Table::instance ().retInfo ().info-> type-> is <IUndefInfo> ())
		Table::instance ().retInfo ().isImmutable () = true;

	    FrameTable::instance ().insert (proto);
	    Table::instance ().retInfo ().currentBlock () = "true";
	    auto block = this-> _function-> getBlock ()-> block ();

	    if (Table::instance ().retInfo ().info-> type-> is<IUndefInfo> ())
		Table::instance ().retInfo ().info-> type = new IVoidInfo ();

	    auto finFrame = new IFinalFrame (Table::instance ().retInfo ().info,
					    this-> _space, this-> _function-> name (),
					    finalParams, block, this-> tempParams);
	    
	    proto-> type () = Table::instance ().retInfo ().info;
	    FrameTable::instance ().insert (finFrame);

	    finFrame-> file () = LOCATION_FILE (this-> _function-> getIdent ().getLocus ());
	    Table::instance ().quitBlock ();

	    verifyReturn (this-> _function-> getIdent (), proto-> type (), Table::instance ().retInfo ());
	    Table::instance ().quitFrame ();
	    return proto;
	}
	
	Table::instance ().quitBlock ();
	Table::instance ().quitFrame ();
	return proto;
	
    }
    
    FrameProto IFrame::validate () {
	Ymir::Error::assert ("TODO");
	return NULL;
    }

    FrameProto IFrame::validate (::syntax::ParamList) {
	Ymir::Error::assert ("TODO");
	return NULL;
    }

    FrameProto IFrame::validate (std::vector <InfoType>) {
	Ymir::Error::assert ("TODO");
	return NULL;
    }    
    
    FrameProto IFrame::validate (ApplicationScore score, std::vector <InfoType> params) {
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

    std::vector <::syntax::Var> IFrame::computeParams (std::vector<Var> attr, std::vector<InfoType> params) {
	std::vector <Var> finalParams;
	for (auto it : Ymir::r (0, attr.size ())) {
	    if (auto var = attr [it] -> to<ITypedVar> ()) {
		finalParams.push_back (var-> var ());
	    } else {		
		auto v = attr [it]-> setType (params [it]);
		finalParams.push_back (v-> var ());
	    }
	}
	return finalParams;
    }
    
    std::vector <::syntax::Var> IFrame::computeParams (std::vector<::syntax::Var> params) {
	std::vector <Var> finalParams;
	for (auto it : Ymir::r (0, params.size ())) {
	    auto info = params [it]-> var ();
	    finalParams.push_back (info);
	}
	return finalParams;
    }

    FrameProto IFrame::validate (Word name, Namespace space, Namespace from, Symbol ret, std::vector <Var> params, Block block, std::vector <Expression> tmps, bool isVariadic) {
	Table::instance ().setCurrentSpace (Namespace (space, name.getStr ()));
	struct Exit {
	    Namespace last;

	    Exit () : last (Table::instance ().templateNamespace ()) {
	    }
	    
	    FrameProto operator () (Word name, Namespace from, Symbol ret, std::vector <Var> params, Block block, std::vector <Expression> tmps, bool isVariadic) {
		Table::instance ().templateNamespace () = from;
		if (ret == NULL) 
		    Table::instance ().retInfo ().info = new (GC) ISymbol (Word::eof (), new (GC) IUndefInfo ());
		else
		    Table::instance ().retInfo ().info = ret;

		auto proto = new (GC) IFrameProto (name.getStr (), from, Table::instance ().retInfo ().info, params, tmps);
		if (!FrameTable::instance ().existsProto (proto)) {
		    if (!Table::instance ().retInfo ().info-> type-> is <IUndefInfo> ())
			Table::instance ().retInfo ().isImmutable () = true;
		    
		    FrameTable::instance ().insert (proto);
		    Table::instance ().retInfo ().currentBlock () = "true";
		    block = block-> block ();

		    if (Table::instance ().retInfo ().info-> type-> is <IUndefInfo> ())
			Table::instance ().retInfo ().info-> type = new (GC) IVoidInfo ();

		    auto finFrame = new (GC) IFinalFrame (Table::instance ().retInfo ().info,
							  from, name.getStr (),
							  params, block, tmps);

		    finFrame-> isVariadic () = isVariadic;
		    proto-> type () = Table::instance ().retInfo ().info;
		    FrameTable::instance ().insert (finFrame);

		    finFrame-> file () = name.getFile ();
		    Table::instance ().quitBlock ();
		    verifyReturn (name, proto-> type (), Table::instance ().retInfo ());
		    Table::instance ().quitFrame ();
		    return proto;
		}
		Table::instance ().quitBlock ();
		Table::instance ().quitFrame ();
		return proto;
	    }

	    ~Exit () {
		Table::instance ().templateNamespace () = last;
	    }
	    
	};

	Exit ex;
	return ex (name, from, ret, params, block, tmps, isVariadic);	
    }

    FrameProto IFrame::validate (Namespace space, Namespace from, std::vector <Var> params, bool isVariadic) {
	Table::instance ().setCurrentSpace (Namespace (space, this-> _function-> getIdent ().getStr ()));
	struct Exit {
	    Namespace last;
	    Frame self;

	    Exit (Frame self) : last (Table::instance ().templateNamespace ()), self (self) {
	    }

	    FrameProto operator () (Namespace from, std::vector <Var> params, bool isVariadic) {		
		if (self-> _function-> getType () == NULL) 
		    Table::instance ().retInfo ().info = new (GC) ISymbol (Word::eof (), new (GC) IUndefInfo ());
		else
		    Table::instance ().retInfo ().info = self-> _function-> getType ()-> asType ()-> info;

		auto proto = new (GC) IFrameProto (self-> _function-> getIdent ().getStr (), from, Table::instance ().retInfo ().info, params, self-> tempParams);
		
		if (!FrameTable::instance ().existsProto (proto)) {
		    if (!Table::instance ().retInfo ().info-> type-> is <IUndefInfo> ())
			Table::instance ().retInfo ().isImmutable () = true;

		    FrameTable::instance ().insert (proto);
		    Table::instance ().retInfo ().currentBlock () = "true";
		    auto block = self-> _function-> getBlock ()-> block ();

		    if (Table::instance ().retInfo ().info-> type-> is <IUndefInfo> ())
			Table::instance ().retInfo ().info-> type = new (GC) IVoidInfo ();

		    auto finFrame = new (GC) IFinalFrame (Table::instance ().retInfo ().info,
							  from, self-> _function-> getIdent ().getStr (),
							  params, block, self-> tempParams);

		    finFrame-> isVariadic () = isVariadic;
		    proto-> type () = Table::instance ().retInfo ().info;
		    FrameTable::instance ().insert (finFrame);

		    finFrame-> file () = self-> _function-> getIdent ().getFile ();
		    Table::instance ().quitBlock ();
		    verifyReturn (self-> _function-> getIdent (), proto-> type (), Table::instance ().retInfo ());
		    Table::instance ().quitFrame ();
		    return proto;
		}
		Table::instance ().quitBlock ();
		Table::instance ().quitFrame ();
		return proto;		
	    }
	    
	    ~Exit () {
		Table::instance ().templateNamespace () = last;
	    }
	    
	};
	Exit ex (this);
	return ex (from, params, isVariadic);
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

    bool& IFrame::isPrivate () {
	return this-> _isPrivate;
    }
    
    Word IFrame::ident () {
	return this-> _function-> getIdent ();
    }

    std::vector <syntax::Expression> & IFrame::templateParams () {
	return this-> tempParams;
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
