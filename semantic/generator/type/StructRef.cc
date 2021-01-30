#include <ymir/semantic/generator/type/StructRef.hh>
#include <ymir/utils/OutBuffer.hh>
#include <ymir/semantic/symbol/Struct.hh>
#include <ymir/semantic/generator/value/VarDecl.hh>
#include <ymir/semantic/generator/value/Struct.hh>

namespace semantic {
    namespace generator {

	std::string StructRef::INIT_NAME = "init";
	std::string StructRef::TUPLEOF   = "tupleof";
	std::string StructRef::DIRECT_FIELDS   = "fields_address";
	
	StructRef::StructRef () :
	    Type ()
	{}

	StructRef::StructRef (const lexing::Word & loc, const Symbol & ref) :
	    Type (loc, loc.getStr ())
	{
	    auto aux = ref;
	    this-> _ref = aux.getPtr ();
	    if (!this-> _ref.lock ()) { 
		this-> isComplex (
		    true
		);
	    }
	}

	Generator StructRef::init (const lexing::Word&  loc, const Symbol & ref) {
	    return Generator {new (NO_GC) StructRef (loc, ref)};
	}

	Generator StructRef::clone () const {
	    return Generator {new (NO_GC) StructRef (*this)};
	}

	bool StructRef::equals (const Generator & gen) const {
	    if (!gen.is<StructRef> ()) return false;
	    auto str = gen.to <StructRef> ();
	    return (Symbol {this-> _ref}).equals (Symbol {str._ref});
	}

	bool StructRef::isRefOf (const Symbol & sym) const {
	    return Symbol {this-> _ref}.isSameRef (sym);
	}

	Symbol StructRef::getRef () const {
	    return Symbol {this-> _ref};
	}

	int StructRef::mutabilityLevel (int level) const {
	    if (this-> isMutable ()) {
		auto ref = Symbol {this-> _ref};
		if (ref.to <semantic::Struct> ().getGenerator ().to <generator::Struct> ().hasComplexField ()) {
		    auto max = level;
		    for (auto & it : ref.to <semantic::Struct> ().getGenerator ().to <generator::Struct> ().getFields ()) {
			auto mut = it.to <Value> ().getType ().to <Type> ().mutabilityLevel (level + 1);
			if (mut > max) max = mut;
		    }
		    return max;
		}
		return level + 1;
	    } else return level;
	}	

	bool StructRef::needExplicitAlias () const {
	    auto ref = Symbol {this-> _ref};
	    for (auto & it : ref.to <semantic::Struct> ().getGenerator ().to <generator::Struct> ().getFields ()) {

		auto mut = it.to <generator::VarDecl> ().isMutable ()
		    && it.to <generator::VarDecl> ().getVarType ().to <Type> ().needExplicitAlias ();
		
		if (mut)
		    return true;
	    }
	    
	    return false;
	}

	bool StructRef::containPointers () const {
	    auto ref = Symbol {this-> _ref};
	    for (auto & it : ref.to <semantic::Struct> ().getGenerator ().to <generator::Struct> ().getFields ()) {
		if (it.to <generator::VarDecl> ().getVarType ().to <Type> ().containPointers ())	       
		    return true;
	    }
	    
	    return false;	    
	}
	
	const Generator & StructRef::getExplicitAliasTypeLoc () const {
	    auto ref = Symbol {this-> _ref};
	    for (auto & it : ref.to <semantic::Struct> ().getGenerator ().to <generator::Struct> ().getFields ()) {		
		auto mut = it.to <generator::VarDecl> ().isMutable ()
		    && it.to <generator::VarDecl> ().getVarType ().to <Type> ().needExplicitAlias ();
		
		if (mut)
		    return it;
	    }
	    
	    return Generator::__empty__;
	}
	
	std::string StructRef::typeName () const {
	    auto ref = Symbol {this-> _ref};
	    return Ymir::format ("%", ref.getRealName ());
	}

	std::string StructRef::getMangledName () const {
	    auto ref = Symbol {this-> _ref};
	    return Ymir::format ("%", ref.getMangledName ());
	}
	
    }
}
