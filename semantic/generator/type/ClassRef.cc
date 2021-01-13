#include <ymir/semantic/generator/type/ClassRef.hh>
#include <ymir/semantic/generator/type/NoneType.hh>
#include <ymir/utils/OutBuffer.hh>
#include <ymir/semantic/symbol/Class.hh>
#include <ymir/semantic/generator/value/Class.hh>

namespace semantic {
    namespace generator {

	const std::string ClassRef::INIT_NAME = "new";
	const std::string ClassRef::TUPLEOF   = "tupleof";
	
	ClassRef::ClassRef () :
	    Type (),
	    _parent (Generator::empty ())
	{}

	ClassRef::ClassRef (const lexing::Word & loc, const Generator & parent, const Symbol & ref) :
	    Type (loc, loc.getStr ()),
	    _parent (parent)
	{
	    auto aux = ref;
	    this-> _ref = aux.getPtr ();
	    // if (!this-> _ref.lock ()) { 
	    // 	this-> isComplex (
	    // 	    true
	    // 	);
	    // }
	}

	Generator ClassRef::init (const lexing::Word&  loc, const Generator & parent, const Symbol & ref) {
	    return Generator {new (NO_GC) ClassRef (loc, parent, ref)};
	}
	
	Generator ClassRef::clone () const {
	    return Generator {new (NO_GC) ClassRef (*this)};
	}

	bool ClassRef::equals (const Generator & gen) const {
	    if (!gen.is<ClassRef> ()) return false;
	    auto str = gen.to <ClassRef> ();
	    return (Symbol {this-> _ref}).equals (Symbol {str._ref});
	}

	bool ClassRef::isRefOf (const Symbol & sym) const {
	    return (Symbol {this-> _ref}).isSameRef (sym);
	}

	Symbol ClassRef::getRef () const {
	    return Symbol {this-> _ref};
	}
	
	std::string ClassRef::typeName () const {
	    return (Symbol {this-> _ref}).getRealName ();
	}
	
	std::string ClassRef::getMangledName () const {
	    return Ymir::format ("%", (Symbol {this-> _ref}).getMangledName ());
	}

	const Generator & ClassRef::getAncestor () const {
	    return this-> _parent;
	}
	
    }
}
