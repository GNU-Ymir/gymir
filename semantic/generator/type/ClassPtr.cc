#include <ymir/semantic/generator/type/ClassPtr.hh>
#include <ymir/utils/OutBuffer.hh>
#include <ymir/errors/Error.hh>

namespace semantic {
    namespace generator {
	
	ClassPtr::ClassPtr () :
	    Type (),
	    _ref (Generator::empty ())
	{
	    this-> isComplex (true);
	}

	ClassPtr::ClassPtr (const lexing::Word & loc, const Generator & inner) :
	    Type (loc, loc.str),
	    _ref (inner)
	{	    
	    this-> isComplex (true);
	    /// We still put it as inner, because that convinient
	    this-> setInners ({Type::init (inner.to <Type> (), inner.to <Type> ().isMutable (), false)});
	    if (!inner.is <ClassRef> ()) {
		Ymir::Error::halt ("", "");
	    }
	}

	Generator ClassPtr::init (const lexing::Word & loc, const Generator & inner) {
	    return Generator {new (NO_GC) ClassPtr (loc, inner)};
	}

	Generator ClassPtr::clone () const {
	    return Generator {new (NO_GC) ClassPtr (*this)};
	}
		
	bool ClassPtr::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    ClassPtr thisClassPtr; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisClassPtr) [0] == vtable) return true;
	    return Type::isOf (type);	
	}

	bool ClassPtr::equals (const Generator & gen) const {
	    if (!gen.is<ClassPtr> ()) return false;
	    auto array = gen.to <ClassPtr> ();
	    return this-> _ref.equals (array._ref);
	}

	bool ClassPtr::needExplicitAlias () const {
	    return true;
	}
	
	bool ClassPtr::isCompatible (const Generator & gen) const {
	    if (this-> equals (gen)) return true;
	    return false;
	}	

	const ClassRef & ClassPtr::getClassRef () const {
	    return this-> _ref.to <ClassRef> ();
	}
	
	std::string ClassPtr::typeName () const {
	    Ymir::OutBuffer buf;
	    buf.write ("&(", this-> _ref.to <ClassRef> ().getTypeName (this-> isMutable ()), ")");
	    return buf.str ();
	}
		
    }
}
