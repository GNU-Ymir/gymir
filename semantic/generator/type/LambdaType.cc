#include <ymir/semantic/generator/type/LambdaType.hh>
#include <ymir/utils/OutBuffer.hh>

namespace semantic {
    namespace generator {
	
	LambdaType::LambdaType () :
	    Type ()
	{
	    this-> isComplex (true);
	}

	LambdaType::LambdaType (const lexing::Word & loc, const Generator & retType, const std::vector <Generator> & inner) :
	    Type (loc, loc.getStr ())
	{
	    this-> isComplex (true);
	    std::vector <Generator> types;
	    types.push_back (retType);
	    types.insert (types.end (), inner.begin (), inner.end ());
	    this-> setInners (types);
	}

	Generator LambdaType::init (const lexing::Word & loc, const Generator & retType, const std::vector<Generator> & inner) {
	    return Generator {new (NO_GC) LambdaType (loc, retType, inner)};
	}

	Generator LambdaType::clone () const {
	    return Generator {new (NO_GC) LambdaType (*this)};
	}
		
	bool LambdaType::equals (const Generator & gen) const {
	    if (!gen.is<LambdaType> ()) return false;
	    auto tu = gen.to <LambdaType> ();
	    if (tu.getInners ().size () != this-> getInners ().size ()) return false;
	    for (auto it : Ymir::r (0, this-> getInners ().size ()))
		if (!this-> getInners () [it].equals (tu.getInners () [it])) return false;
		
	    return true;
	}
	
	Generator LambdaType::createMutable (bool is) const {
	    auto ret = this-> clone ();
	    ret.to <LambdaType> ().setMutable (is);
	    return ret;
	}
	
	std::string LambdaType::typeName () const {
	    Ymir::OutBuffer buf;
	    buf.write ("fn (");
	    for (auto it  : Ymir::r (1, this-> getInners ().size ())) {
		if (it != 1) buf.write (", ");
		if (this-> getInners ()[it].isEmpty ())
		    buf.write ("any");
		else
		    buf.write (this-> getInners () [it].to <Type> ().getTypeName (false));
	    }
	    buf.write (")-> ");
	    if (this-> getInners ()[0].isEmpty ()) buf.write ("any");
	    else buf.write (this-> getInners ()[0].to <Type> ().getTypeName (false));
	    return buf.str ();
	}
	
    }
}
