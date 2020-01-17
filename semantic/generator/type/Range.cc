#include <ymir/semantic/generator/type/Range.hh>
#include <ymir/semantic/generator/type/Integer.hh>
#include <ymir/semantic/generator/type/Char.hh>
#include <ymir/utils/OutBuffer.hh>
#include <ymir/utils/Match.hh>

namespace semantic {
    namespace generator {
	std::string Range::NAME       = "r";
	std::string Range::INNER_NAME = "inner";
	std::string Range::FST_NAME   = "fst";
	std::string Range::SCD_NAME   = "scd";
	std::string Range::STEP_NAME  = "step";
	std::string Range::FULL_NAME  = "contain";
	
	Range::Range () :
	    Type ()
	{
	    this-> isComplex (true);
	}

	Range::Range (const lexing::Word & loc, const Generator & inner) :
	    Type (loc, loc.str)
	{
	    this-> isComplex (true);
	    auto aux = inner;
	    aux.to <Type> ().isRef (false);
	    
	    this-> setInners ({aux});
	}

	Generator Range::init (const lexing::Word & loc, const Generator & inner) {
	    return Generator {new (Z0) Range (loc, inner)};
	}

	Generator Range::clone () const {
	    return Generator {new (Z0) Range (*this)};
	}
		
	bool Range::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    Range thisRange; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisRange) [0] == vtable) return true;
	    return Type::isOf (type);	
	}

	bool Range::equals (const Generator & gen) const {
	    if (!gen.is<Range> ()) return false;
	    auto array = gen.to <Range> ();
	    return this-> getInners () [0].equals (array.getInners () [0]);
	}

	Generator Range::getStepType () const {
	    match (this-> getInners ()[0]) {
		of (Integer, in, {
			return Integer::init (in.getLocation (), in.getSize (), true);
		    }
		) else of (Char, ch, {
			return Integer::init (ch.getLocation (), ch.getSize (), true);		    
		    }
		);
	    }
	    return this-> getInners ()[0];
	}
	
	std::string Range::typeName () const {
	    Ymir::OutBuffer buf;
	    buf.write ("r!(", this-> getInners () [0].to<Type> ().getTypeName (true), ")");
	    return buf.str ();
	}	
	
    }
}
