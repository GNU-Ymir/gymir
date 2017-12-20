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
    
    ApplicationScore IFrame::getScore (Word ident, std::vector <Var> attrs, std::vector <InfoType> args) {
	auto score = new IApplicationScore (ident);
	if (attrs.size () == 0 && args.size () == 0) {
	    score-> score = AFF;
	    score-> score += this-> currentScore ();
	    return score;
	} else if (attrs.size () == args.size ()) {
	    for (auto it : Ymir::r (0, args.size ())) {
		InfoType info = NULL;
		auto param = attrs [it];
		if (auto tvar = param-> to<ITypedVar> ()) {
		    info = tvar-> getType ()-> clone ();
		    auto type = args [it]-> CompOp (info);
		    if (type != NULL) type = type-> ConstVerif (info);
		    else return NULL;

		    if (type && type-> isSame (info)) {
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
		} else {
		    if (args [it]-> is<IFunctionInfo> ())
			Ymir::Error::assert ("TODO, passage function en param");
		    auto var = attrs [it]-> to<IVar> ();
		    if (var-> deco == Keys::REF && !args [it]-> is<IRefInfo> ()) {
			auto type = args [it]-> CompOp (new IRefInfo (args [it]-> clone ()));
			if (type == NULL) return NULL;
			score-> treat.push_back (type);
		    } else if (var-> deco == Keys::CONST) {
			score-> treat.push_back (args [it]-> cloneConst ());
		    } else {
			score-> treat.push_back (args [it]-> clone ());
		    }
		    score-> score += CHANGE;
		}
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


    void IFrame::verifyReturn (Word, Symbol ret, FrameReturnInfo info) {
	if (!ret-> type-> is<IVoidInfo> () && !ret-> type-> is<IUndefInfo> ()) {
	    if (!info.retract ()) {
		Ymir::Error::assert ("TODO, gerer l'erreur");
	    }
	}
    }

    std::vector <::syntax::Var> IFrame::computeParams (std::vector<Var> attr, std::vector<InfoType> params) {
	std::vector <Var> finalParams;
	for (auto it : Ymir::r (0, attr.size ())) {
	    if (!attr [it] -> is<ITypedVar> ()) {
		auto var = attr [it]-> setType (params [it]);
		finalParams.push_back ((Var) var-> expression ());
	    } else {
		finalParams.push_back (attr [it]-> var ());
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

    Word IFrame::ident () {
	return this-> _function-> getIdent ();
    }
    

}
