#include <ymir/semantic/generator/type/TupleClosure.hh>
#include <ymir/utils/OutBuffer.hh>

namespace semantic {
    namespace generator {
	
	TupleClosure::TupleClosure () :
	    Type ()
	{
	    this-> isComplex (true);
	}

	TupleClosure::TupleClosure (const lexing::Word & loc, const std::vector <Generator> & inner) :
	    Type (loc, loc.str)
	{
	    this-> isComplex (true);
	    auto aux = inner;
	    this-> setInners (aux);
	}

	Generator TupleClosure::init (const lexing::Word & loc, const std::vector<Generator> & inner) {
	    return Generator {new (Z0) TupleClosure (loc, inner)};
	}

	Generator TupleClosure::clone () const {
	    return Generator {new (Z0) TupleClosure (*this)};
	}
		
	bool TupleClosure::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    TupleClosure thisTupleClosure; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisTupleClosure) [0] == vtable) return true;
	    return Type::isOf (type);	
	}

	bool TupleClosure::equals (const Generator & gen) const {
	    if (!gen.is<TupleClosure> ()) return false;
	    auto tu = gen.to <TupleClosure> ();
	    if (tu.getInners ().size () != this-> getInners ().size ()) return false;
	    for (auto it : Ymir::r (0, this-> getInners ().size ()))
		if (!this-> getInners () [it].equals (tu.getInners () [it])) return false;
		
	    return true;
	}

	bool TupleClosure::needExplicitAlias () const {
	    for (auto it : Ymir::r (0, this-> getInners ().size ())) {
		if (this-> getInners () [it].to <Type> ().needExplicitAlias ()
		    && this-> getInners ()[it].to <Type> ().isMutable ()) return true;
	    }
	    return false;
	}
	
	std::string TupleClosure::typeName () const {
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
