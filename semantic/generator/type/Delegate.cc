#include <ymir/semantic/generator/type/Delegate.hh>
#include <ymir/utils/OutBuffer.hh>

namespace semantic {
    namespace generator {

	std::string Delegate::RET_NAME = "ret";
	std::string Delegate::PARAMS_NAME = "params";
	
	Delegate::Delegate () :
	    Type ()
	{
	    this-> isComplex (true);
	}

	Delegate::Delegate (const lexing::Word & loc, const Generator & funcPtr) :
	    Type (loc, loc.str)
	{
	    this-> isComplex (true);
	    this-> setInners ({funcPtr}); 
	}

	Generator Delegate::init (const lexing::Word & loc, const Generator & funcPtr) {
	    return Generator {new (Z0) Delegate (loc, funcPtr)};
	}

	Generator Delegate::clone () const {
	    return Generator {new (Z0) Delegate (*this)};
	}
		
	bool Delegate::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    Delegate thisDelegate; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisDelegate) [0] == vtable) return true;
	    return Type::isOf (type);	
	}

	bool Delegate::equals (const Generator & gen) const {
	    if (!gen.is<Delegate> ()) return false;
	    auto array = gen.to <Delegate> ();
	    if (this-> getInners ().size () != array.getInners ().size ()) return false;
	    for (auto it : Ymir::r (0, this-> getInners ().size ()))
		if (!this-> getInners () [it].equals (array.getInners ()[it]))
		    return false;
	    return true;
	}

	std::string Delegate::typeName () const {
	    Ymir::OutBuffer buf;
	    buf.write ("dg(");
	    for (auto it : Ymir::r (1, this-> getInners ()[0].to<Type> ().getInners ().size ())) {
		if (it != 1) buf.write (", "); 
		buf.write (this-> getInners ()[0].to <Type> ().getInners ()[it].to <Type> ().getTypeName ());
	    }
	    buf.write (Ymir::format (")-> %", this-> getInners ()[0].to <Type> ().getInners ()[0].to <Type> ().getTypeName ()));
	    return buf.str ();
	}	
	
    }
}
