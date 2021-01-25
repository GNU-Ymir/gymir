#include <ymir/semantic/generator/type/Tuple.hh>
#include <ymir/utils/OutBuffer.hh>

namespace semantic {
    namespace generator {

	const std::string Tuple::ARITY_NAME = "arity";

	const std::string Tuple::INIT_NAME = "init";
	
	Tuple::Tuple () :
	    Type ()
	{
	    this-> isComplex (true);
	}

	Tuple::Tuple (const lexing::Word & loc, const std::vector <Generator> & inner) :
	    Type (loc, loc.getStr ())
	{
	    this-> isComplex (true);
	    
	    std::vector <Generator> aux;
	    aux.reserve (inner.size ());
	    
	    for (auto & it : inner) {
		aux.push_back (Type::init (it.to <Type> (), it.to <Type> ().isMutable (), false));
	    }
	    
	    this-> setInners (aux);
	}

	Generator Tuple::init (const lexing::Word & loc, const std::vector<Generator> & inner) {
	    return Generator {new (NO_GC) Tuple (loc, inner)};
	}

	Generator Tuple::clone () const {
	    return Generator {new (NO_GC) Tuple (*this)};
	}
		
	bool Tuple::equals (const Generator & gen) const {
	    if (!gen.is<Tuple> ()) return false;
	    auto tu = gen.to <Tuple> ();
	    if (tu.getInners ().size () != this-> getInners ().size ()) return false;
	    for (auto it : Ymir::r (0, this-> getInners ().size ()))
		if (!this-> getInners () [it].equals (tu.getInners () [it])) return false;
		
	    return true;
	}

	bool Tuple::needExplicitAlias () const {
	    for (auto it : Ymir::r (0, this-> getInners ().size ())) {
		if (this-> getInners () [it].to <Type> ().needExplicitAlias ()
		    && this-> getInners ()[it].to <Type> ().isMutable ()) return true;
	    }
	    return false;
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
