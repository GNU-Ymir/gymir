#include <ymir/semantic/generator/value/VtablePointer.hh>

namespace semantic {

    namespace generator {

	VtablePointer::VtablePointer () :
	    _str (Generator::empty ())
	{}

	VtablePointer::VtablePointer (const lexing::Word & loc, const Generator & type, const Generator & cl) :
	    Value (loc, type),
	    _str (cl)
	{
	    this-> setThrowers (this-> _str.getThrowers ());
	}

	Generator VtablePointer::init (const lexing::Word & loc, const Generator & type, const Generator & cl) {
	    return Generator {new (NO_GC) VtablePointer (loc, type, cl)};
	}
	
	Generator VtablePointer::clone () const {
	    return Generator {new (NO_GC) VtablePointer (*this)};
	}
	
	bool VtablePointer::equals (const Generator & gen) const {
	    if (!gen.is <VtablePointer> ()) return false;
	    auto bin = gen.to<VtablePointer> ();	    
	    return bin._str.equals (this-> _str);
	}

	const Generator & VtablePointer::getClass () const {
	    return this-> _str;
	}

	std::string VtablePointer::prettyString () const {
	    return Ymir::format ("#_vtable (%)", this-> _str.prettyString ());
	}

    }    

}
