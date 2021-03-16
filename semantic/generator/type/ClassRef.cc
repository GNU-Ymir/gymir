#include <ymir/semantic/generator/type/ClassRef.hh>
#include <ymir/semantic/generator/type/NoneType.hh>
#include <ymir/utils/OutBuffer.hh>
#include <ymir/semantic/symbol/Class.hh>
#include <ymir/semantic/generator/value/Class.hh>

namespace semantic {
    namespace generator {

	const std::string	ClassRef::INIT_NAME	 = "new";
	const std::string	ClassRef::SUPER		 = "super";
	
	ClassRef::ClassRef () :
	    Type (),
	    _parent (Generator::empty ()),
	    _isFast (false)
	{}

	ClassRef::ClassRef (const lexing::Word & loc, const Generator & parent, const Symbol & ref, bool isFast) :
	    Type (loc, loc.getStr ()),
	    _parent (parent),
	    _isFast (isFast)
	{
	    auto aux = ref;
	    this-> _ref = aux.getPtr ();
	    // if (!this-> _ref.lock ()) { 
	    // 	this-> isComplex (
	    // 	    true
	    // 	);
	    // }
	}

	Generator ClassRef::init (const lexing::Word&  loc, const Generator & parent, const Symbol & ref, bool isFast) {
	    return Generator {new (NO_GC) ClassRef (loc, parent, ref, isFast)};
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
	    if (this-> _isFast) {
		return "fast " + (Symbol {this-> _ref}).getRealName ();
	    }
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
