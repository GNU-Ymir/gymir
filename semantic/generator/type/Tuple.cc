#include <ymir/semantic/generator/type/Tuple.hh>
#include <ymir/utils/OutBuffer.hh>

namespace semantic {
    namespace generator {
	
	Tuple::Tuple () :
	    Type ()
	{
	    this-> isComplex (true);
	}

	Tuple::Tuple (const lexing::Word & loc, const std::vector <Generator> & inner) :
	    Type (loc, loc.str)
	{
	    this-> isComplex (true);
	    this-> setInners (inner);
	}

	Generator Tuple::init (const lexing::Word & loc, const std::vector<Generator> & inner) {
	    return Generator {new (Z0) Tuple (loc, inner)};
	}

	Generator Tuple::clone () const {
	    return Generator {new (Z0) Tuple (*this)};
	}
		
	bool Tuple::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    Tuple thisTuple; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisTuple) [0] == vtable) return true;
	    return Type::isOf (type);	
	}

	bool Tuple::equals (const Generator & gen) const {
	    if (!gen.is<Tuple> ()) return false;
	    auto tu = gen.to <Tuple> ();
	    if (tu.getInners ().size () != this-> getInners ().size ()) return false;
	    for (auto it : Ymir::r (0, this-> getInners ().size ()))
		if (!this-> getInners () [it].equals (tu.getInners () [it])) return false;
		
	    return true;
	}
	
	std::string Tuple::typeName () const {
	    Ymir::OutBuffer buf;
	    buf.write ("(");
	    for (auto it  : Ymir::r (0, this-> getInners ().size ())) {
		if (it != 0) buf.write (", ");
		buf.write (this-> getInners () [it].to <Type> ().getTypeName (this-> isMutable ()));
	    }
	    buf.write (")");
	    return buf.str ();
	}
	
    }
}
