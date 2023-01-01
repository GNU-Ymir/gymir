#include <ymir/semantic/generator/type/Closure.hh>
#include <ymir/utils/OutBuffer.hh>

namespace semantic {
    namespace generator {

	std::string Closure::ARITY_NAME = "arity";

	std::string Closure::INIT_NAME = "init";
	
	Closure::Closure () :
	    Type ()
	{
	    this-> isComplex (true);
	}

	Closure::Closure (const lexing::Word & loc, const std::vector <Generator> & inner, const std::vector<std::string> & names, uint32_t index) :
	    Type (loc, loc.getStr ()),
	    _names (names),
	    _index (index)
	{
	    this-> isComplex (true);
	    this-> setInners (inner);
	}

	Generator Closure::init (const lexing::Word & loc, const std::vector<Generator> & inner, const std::vector <std::string> & names, uint32_t index) {
	    return Generator {new (NO_GC) Closure (loc, inner, names, index)};
	}

	Generator Closure::clone () const {
	    return Generator {new (NO_GC) Closure (*this)};
	}

	Generator Closure::createMutable (bool is) const {
	    auto ret = this-> clone ();
	    ret.to <Closure> ().setMutable (is);
	    return ret;
	}

	Generator Closure::toMutable () const {
	    return this-> createMutable (true);
	}

	Generator Closure::toDeeplyMutable () const {
	    return this-> createMutable (true);
	}
		
	bool Closure::equals (const Generator & gen) const {
	    if (!gen.is<Closure> ()) return false;
	    auto tu = gen.to <Closure> ();
	    if (tu.getInners ().size () != this-> getInners ().size ()) return false;
	    for (auto it : Ymir::r (0, this-> getInners ().size ()))
		if (!this-> getInners () [it].equals (tu.getInners () [it])) return false;
		
	    return true;
	}

	const Generator & Closure::getField (const std::string & name) const {
	    for (auto it : Ymir::r (0, this-> _names.size ())) {
		if (this-> _names [it] == name)
		    return this-> getInners () [it];
	    }
	    return Generator::__empty__;
	}
	
	const std::vector <std::string> & Closure::getNames () const {
	    return this-> _names;
	}	

	uint32_t Closure::getIndex () const {
	    return this-> _index;
	}
	
	std::string Closure::typeName () const {
	    Ymir::OutBuffer buf;
	    buf.write ("(");
	    for (auto it  : Ymir::r (0, this-> getInners ().size ())) {
		if (it != 0) buf.write (", ");
		buf.write (this-> getInners () [it].to <Type> ().computeTypeName (this-> isMutable (), true, true));
	    }
	    buf.write (")");
	    return buf.str ();
	}
	
    }
}
