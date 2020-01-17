#include <ymir/semantic/generator/type/ClassRef.hh>
#include <ymir/utils/OutBuffer.hh>
#include <ymir/semantic/symbol/Class.hh>
#include <ymir/semantic/generator/value/Class.hh>

namespace semantic {
    namespace generator {

	std::string ClassRef::INIT_NAME = "new";
	
	ClassRef::ClassRef () :
	    Type (),
	    _ref (Symbol::__empty__),
	    _parent (Generator::empty ())
	{}

	ClassRef::ClassRef (const lexing::Word & loc, const Generator & parent, const Symbol & ref) :
	    Type (loc, loc.str),
	    _ref (ref),
	    _parent (parent)
	{
	    if (!this-> _ref.isEmpty ()) { // A structure is complex iif one of its field is complex
		this-> isComplex (
		    true
		);
	    }
	}

	Generator ClassRef::init (const lexing::Word&  loc, const Generator & parent, const Symbol & ref) {
	    return Generator {new (Z0) ClassRef (loc, parent, ref)};
	}

	Generator ClassRef::clone () const {
	    return Generator {new (Z0) ClassRef (*this)};
	}

	bool ClassRef::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    ClassRef thisClass; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisClass) [0] == vtable) return true;
	    return Type::isOf (type);	
	}

	bool ClassRef::equals (const Generator & gen) const {
	    if (!gen.is<ClassRef> ()) return false;
	    auto str = gen.to <ClassRef> ();
	    return this-> _ref.equals (str._ref);
	}

	bool ClassRef::isRefOf (const Symbol & sym) const {
	    return this-> _ref.isSameRef (sym);
	}

	const Symbol & ClassRef::getRef () const {
	    return this-> _ref;
	}

	bool ClassRef::needExplicitAlias () const {
	    return true;
	}
	
	std::string ClassRef::typeName () const {
	    return Ymir::format ("%", this-> _ref.getRealName ());
	}

	std::string ClassRef::getMangledName () const {
	    return Ymir::format ("%", this-> _ref.getMangledName ());
	}

	const Generator & ClassRef::getAncestor () const {
	    return this-> _parent;
	}
	
    }
}
