#include <ymir/semantic/generator/type/EnumRef.hh>
#include <ymir/utils/OutBuffer.hh>
#include <ymir/semantic/symbol/Enum.hh>
#include <ymir/semantic/generator/value/Enum.hh>

namespace semantic {
    namespace generator {

	const std::string EnumRef::MEMBERS = "members";
	
	EnumRef::EnumRef () :
	    Type ()
	{}

	EnumRef::EnumRef (const lexing::Word & loc, const Symbol & ref) :
	    Type (loc, loc.getStr ())
	{
	    auto aux = ref;
	    this-> _ref = aux.getPtr ();
	}

	Generator EnumRef::init (const lexing::Word&  loc, const Symbol & ref) {
	    return Generator {new (NO_GC) EnumRef (loc, ref)};
	}

	Generator EnumRef::clone () const {
	    return Generator {new (NO_GC) EnumRef (*this)};
	}
	
	bool EnumRef::directEquals (const Generator & gen) const {
	    if (!gen.is<EnumRef> ()) return false;
	    auto str = gen.to <EnumRef> ();
	    return (Symbol {this-> _ref}).equals (Symbol {str._ref});
	}
	
	bool EnumRef::isRefOf (const Symbol & sym) const {
	    return (Symbol {this-> _ref}).isSameRef (sym);
	}
	
	Symbol EnumRef::getRef () const {
	    return Symbol {this-> _ref};
	}
	
	std::string EnumRef::typeName () const {
	    return Ymir::format ("%", (Symbol {this-> _ref}).getRealName ());
	}

	std::string EnumRef::getMangledName () const {	    
	    return Ymir::format ("%", (Symbol {this-> _ref}).getMangledName ());
	}
	
    }
}
