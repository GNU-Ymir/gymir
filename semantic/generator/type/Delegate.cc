#include <ymir/semantic/generator/type/Delegate.hh>
#include <ymir/utils/OutBuffer.hh>
#include <ymir/semantic/generator/type/FuncPtr.hh>
#include <ymir/semantic/generator/type/Void.hh>
#include <ymir/errors/Error.hh>

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
	    Type (loc, loc.getStr ())
	{
	    this-> isComplex (true);
	    this-> setInners ({funcPtr}); 
	}

	Generator Delegate::init (const lexing::Word & loc, const Generator & funcPtr) {
	    return Generator {new (NO_GC) Delegate (loc, funcPtr)};
	}

	Generator Delegate::clone () const {
	    return Generator {new (NO_GC) Delegate (*this)};
	}
		
	bool Delegate::equals (const Generator & gen) const {
	    if (!gen.is<Delegate> ()) return false;
	    auto array = gen.to <Delegate> ();
	    if (this-> getInners ().size () != array.getInners ().size ()) return false;
	    for (auto it : Ymir::r (0, this-> getInners ().size ())) {
		if (!this-> getInners () [it].equals (array.getInners ()[it])) {
		    return false;
		}
	    }
	    return true;
	}

	Generator Delegate::createMutable (bool is) const {
	    auto ret = this-> clone ();
	    ret.to <Delegate> ().setMutable (is);
	    return ret;
	}

	Generator Delegate::toMutable () const {
	    return this-> createMutable (true);
	}

	Generator Delegate::toDeeplyMutable () const {
	    return this-> createMutable (true);
	}
	
	std::string Delegate::typeName () const {
	    Ymir::OutBuffer buf;
	    if (this-> getInners ()[0].is<Type> ()) {
		buf.write ("dg(");
		for (auto it : Ymir::r (1, this-> getInners ()[0].to<Type> ().getInners ().size ())) {
		    if (it != 1) buf.write (", "); 
		    buf.write (this-> getInners ()[0].to <Type> ().getInners ()[it].to <Type> ().getTypeName ());
		}
		buf.write (Ymir::format (")-> %", this-> getInners ()[0].to <Type> ().getInners ()[0].to <Type> ().getTypeName ()));
	    } else {
		buf.writef ("dg(%)", this-> getInners ()[0].prettyString ());
	    }
	    return buf.str ();
	}	
	
    }
}
