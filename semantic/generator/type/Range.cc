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
	    Type (loc, loc.getStr ())
	{
	    this-> isComplex (true);	    
	    this-> setInners ({Type::init (inner.to <Type> (), inner.to <Type> ().isMutable (), false)});
	}

	Generator Range::init (const lexing::Word & loc, const Generator & inner) {
	    return Generator {new (NO_GC) Range (loc, inner)};
	}

	Generator Range::clone () const {
	    return Generator {new (NO_GC) Range (*this)};
	}
		
	bool Range::directEquals (const Generator & gen) const {
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
