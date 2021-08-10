#include <ymir/semantic/generator/type/Slice.hh>
#include <ymir/utils/OutBuffer.hh>
#include <ymir/semantic/generator/type/Array.hh>
#include <ymir/semantic/validator/Visitor.hh>

namespace semantic {
    namespace generator {

	std::string Slice::LEN_NAME = "len";
	std::string Slice::PTR_NAME = "ptr";
	std::string Slice::INNER_NAME = "inner";
	std::string Slice::INIT = "init";
	
	Slice::Slice () :
	    Type ()
	{
	    this-> isComplex (true);
	}

	Slice::Slice (const lexing::Word & loc, const Generator & inner) :
	    Type (loc, loc.getStr ())
	{
	    this-> isComplex (true);	    
	    this-> setInners ({Type::init (inner.to <Type> (), inner.to <Type> ().isMutable (), false)});
	}

	Generator Slice::init (const lexing::Word & loc, const Generator & inner) {
	    return Generator {new (NO_GC) Slice (loc, inner)};
	}

	Generator Slice::clone () const {
	    return Generator {new (NO_GC) Slice (*this)};
	}
		
	bool Slice::equals (const Generator & gen) const {
	    if (!gen.is<Slice> ()) return false;
	    auto array = gen.to <Slice> ();
	    return this-> getInners () [0].equals (array.getInners () [0]);
	}

	bool Slice::isCompatible (const Generator & gen) const {
	    if (this-> equals (gen)) return true;
	    if (gen.is <Array> ()) {
		auto array = gen.to <Array> ();		
		if (this-> getInners () [0].to<Type> ().isCompatible (array.getInners () [0])) {
		    return true;
		} else if (array.getInners ()[0].is <Void> ()) {
		    return true;
		}
	    } else if (gen.is <Slice> ()) {
		auto array = gen.to <Slice> ();
		if (this-> getInners () [0].to<Type> ().isCompatible (array.getInners () [0])) {
		    return true;
		} else if (array.getInners ()[0].is <Void> ()) {
		    return true;
		}
	    }
	    return false;
	}

	bool Slice::needExplicitAlias () const {
	    return true;
	}

	bool Slice::containPointers () const {
	    return true;
	}
	
	std::string Slice::typeName () const {
	    Ymir::OutBuffer buf;
	    buf.write ("[", this-> getInners () [0].to<Type> ().computeTypeName (this-> isMutable (), true, true), "]");
	    return buf.str ();
	}
		
    }
}
