#include <ymir/semantic/generator/type/Option.hh>
#include <ymir/utils/OutBuffer.hh>
#include <ymir/errors/Error.hh>

namespace semantic {
    namespace generator {

	const std::string Option::INIT_NAME = "err";
	const std::string Option::TYPE_FIELD = "type";
	const std::string Option::VALUE_FIELD = "val";
	const std::string Option::ERROR_FIELD = "err";
	
	Option::Option () :
	    Type ()
	{
	    this-> isComplex (true);
	}

	Option::Option (const lexing::Word & loc, const Generator & inner, const Generator & errorType) :
	    Type (loc, loc.getStr ())
	{
	    this-> isComplex (true);
	    this-> setInners ({
		    Type::init (inner.to <Type> (), inner.to <Type> ().isMutable (), false),
		    Type::init (errorType.to <Type> (), false, false)
		});
	}

	Generator Option::init (const lexing::Word & loc, const Generator & inner, const Generator & errorType) {
	    return Generator {new (NO_GC) Option (loc, inner, errorType)};
	}

	Generator Option::clone () const {
	    return Generator {new (NO_GC) Option (*this)};
	}
		
	bool Option::equals (const Generator & gen) const {
	    if (!gen.is<Option> ()) return false;
	    auto array = gen.to <Option> ();
	    return this-> getInners () [0].equals (array.getInners () [0]);
	}

	bool Option::needExplicitAlias () const {
	    for (auto it : Ymir::r (0, 1)) {
		if (this-> getInners () [it].to <Type> ().needExplicitAlias ()
		    && this-> getInners ()[it].to <Type> ().isMutable ()) return true;
	    }
	    return false;
	}
	
	bool Option::containPointers () const {
	    return this-> getInners ()[0].to <Type> ().containPointers ();
	}

	bool Option::isCompatible (const Generator & gen) const {
	    if (this-> equals (gen)) return true;
	    return false;
	}	
	
	std::string Option::typeName () const {
	    Ymir::OutBuffer buf;
	    buf.write ("(", this-> getInners () [0].to<Type> ().getTypeName (this-> isMutable ()), ")?");
	    return buf.str ();
	}
		
    }
}
