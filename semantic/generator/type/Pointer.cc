#include <ymir/semantic/generator/type/Pointer.hh>
#include <ymir/utils/OutBuffer.hh>
#include <ymir/semantic/generator/type/Array.hh>
#include <ymir/semantic/generator/type/ClassRef.hh>
#include <ymir/semantic/generator/type/Void.hh>
#include <ymir/errors/Error.hh>

namespace semantic {
    namespace generator {

	std::string Pointer::INNER_NAME = "inner";

	std::string Pointer::INIT_NAME = "init";

	uint64_t Pointer::INIT = 0;
	
	Pointer::Pointer () :
	    Type ()
	{
	    this-> isComplex (true);
	}

	Pointer::Pointer (const lexing::Word & loc, const Generator & inner) :
	    Type (loc, loc.getStr ())
	{
	    this-> isComplex (true);
	    this-> setInners ({Type::init (inner.to <Type> (), inner.to <Type> ().isMutable (), false)});
	    if (inner.is <ClassRef> ())
		Ymir::Error::halt ("", "");
	}

	Generator Pointer::init (const lexing::Word & loc, const Generator & inner) {
	    return Generator {new (NO_GC) Pointer (loc, inner)};
	}

	Generator Pointer::clone () const {
	    return Generator {new (NO_GC) Pointer (*this)};
	}
		
	bool Pointer::equals (const Generator & gen) const {
	    if (!gen.is<Pointer> ()) return false;
	    auto array = gen.to <Pointer> ();
	    return this-> getInners () [0].equals (array.getInners () [0]);
	}

	bool Pointer::needExplicitAlias () const {
	    return true;
	}
	
	bool Pointer::containPointers () const {
	    return true;
	}

	bool Pointer::isCompatible (const Generator & gen) const {
	    if (this-> equals (gen)) return true;
	    if (!gen.is <Pointer> ()) return false;
	    auto array = gen.to <Pointer> ();
		
	    return this-> getInners () [0].to<Type> ().isCompatible (array.getInners () [0]);
	}	
	
	std::string Pointer::typeName () const {
	    Ymir::OutBuffer buf;
	    buf.write ("&(", this-> getInners () [0].to<Type> ().computeTypeName (this-> isMutable (), true, true), ")");
	    return buf.str ();
	}
		
    }
}
