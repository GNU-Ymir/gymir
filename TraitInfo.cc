#include <ymir/semantic/object/TraitInfo.hh>
#include <ymir/ast/_.hh>
#include <ymir/semantic/object/AggregateInfo.hh>
#include <ymir/semantic/object/MethodInfo.hh>
#include <ymir/syntax/Keys.hh>
#include <ymir/syntax/Word.hh>

using namespace syntax;

namespace semantic {

    ITraitInfo::ITraitInfo (Namespace space, Trait info) :
	IInfoType (true),
	_space (space),
	_info (info)
    {
	this-> isType (false);
    }

    bool ITraitInfo::isSame (InfoType o) {
	return o == this;
    }

    bool ITraitInfo::validate (InfoType o) {
	AggregateInfo info = NULL;
	if (o-> is <IAggregateCstInfo> ()) {
	    auto aux = o-> TempOp ({});
	    if (aux) info = aux-> to <IAggregateInfo> ();
	} else info = o-> to <IAggregateInfo> ();
	if (info == NULL) return false;

	auto tok = this-> _info-> getIdent ();
	for (auto tvar : this-> _info-> getAttrs ()) {
	    auto type = tvar-> getType ();
	    auto inAggr = info-> DotOp (tvar);

	    if (!inAggr) return false;
	    else if (!inAggr-> isSame (type)) {		
		if (auto meth = inAggr-> to <IMethodInfo> ()) {
		    if (!meth-> isAttribute ()) return false;
		}
	    }
	}

	auto methods = info-> getAllMethods ();
	for (auto tmeth : this-> _info-> getProtos ()) {
	    std::vector <InfoType> params;
	    InfoType ret;
	    for (auto it : tmeth._types) {
		auto type = it-> toType ();
		if (type == NULL) return false;
		params.push_back (type-> info-> type ());
	    }
	    
	    if (tmeth._ret == NULL) ret = new (Z0) IVoidInfo ();
	    else {
		auto retType = tmeth._ret-> toType ();	    
		if (!retType) return false;
		ret = retType-> info-> type ();
	    }

	    bool changed = false;
	    for (auto & it : methods) {
		if (tmeth._name.getStr () == it-> name () && tmeth._isSelf) {
		    auto lparams = it-> frame ()-> func ()-> getParams ();
		    if (lparams.size () != params.size () + 1) continue;
		    bool valid = true;
		    for (auto it : Ymir::r (1, lparams.size ())) {
			if (!lparams [it]-> is <ITypedVar> ()) {
			    valid = false;
			    break;
			}
		    }

		    if (!valid) continue;
		    auto proto = it-> frame ()-> validate ();
		    if (proto != NULL) {
			for (auto it : Ymir::r (0, params.size ())) {
			    if (!params [it]-> isSame (proto-> vars ()[it + 1]-> info-> type ())) {
				valid = false;
				break;
			    }
			}
			if (!valid) continue;
			if (!proto-> type ()-> type ()-> isSame (ret)) continue;
			changed = true;
			break;
		    }
		}
	    }

	    if (!changed) return false;
	}

	return true;
    }

    
    InfoType ITraitInfo::onClone () {
	return this;
    }

    InfoType ITraitInfo::TempOp (const std::vector <Expression> & params) {
	if (params.size () == 0) return this;
	return NULL;
    }
    
    std::string ITraitInfo::innerTypeString () {
	return "trait " + this-> _info-> getIdent ().getStr ();
    }

    std::string ITraitInfo::innerSimpleTypeString () {
	return this-> innerTypeString ();
    }

    const char* ITraitInfo::getId () {
	return ITraitInfo::id ();
    }
    
}
