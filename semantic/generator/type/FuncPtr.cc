#include <ymir/semantic/generator/type/FuncPtr.hh>
#include <ymir/utils/OutBuffer.hh>

namespace semantic {
    namespace generator {

	std::string FuncPtr::RET_NAME = "ret";

	std::string FuncPtr::PARAMS_NAME = "params";
	
	FuncPtr::FuncPtr () :
	    Type ()
	{
	    this-> isComplex (true);
	}

	    FuncPtr::FuncPtr (const lexing::Word & loc, const Generator & retType, const std::vector <Generator> & typeParams) :
	    Type (loc, loc.str)
	{
	    this-> isComplex (true);
	    std::vector <Generator> gens = typeParams;
	    gens.insert (gens.begin (), retType);
	    this-> setInners (gens);
	}

	Generator FuncPtr::init (const lexing::Word & loc, const Generator & retType, const std::vector <Generator> & typeParams) {
	    return Generator {new (Z0) FuncPtr (loc, retType, typeParams)};
	}

	Generator FuncPtr::clone () const {
	    return Generator {new (Z0) FuncPtr (*this)};
	}
		
	bool FuncPtr::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    FuncPtr thisFuncPtr; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisFuncPtr) [0] == vtable) return true;
	    return Type::isOf (type);	
	}

	bool FuncPtr::equals (const Generator & gen) const {
	    if (!gen.is<FuncPtr> ()) return false;
	    auto array = gen.to <FuncPtr> ();
	    if (this-> getInners ().size () != array.getInners ().size ()) return false;
	    for (auto it : Ymir::r (0, this-> getInners ().size ()))
		if (!this-> getInners () [it].equals (array.getInners ()[it]))
		    return false;
	    return true;
	}

	std::string FuncPtr::typeName () const {
	    Ymir::OutBuffer buf;
	    buf.write ("fn(");
	    for (auto it : Ymir::r (1, this-> getInners ().size ())) {
		if (it != 1) buf.write (", "); 
		buf.write (this-> getInners ()[it].to <Type> ().getTypeName ());
	    }
	    buf.write (Ymir::format (")-> %", this-> getInners ()[0].to <Type> ().getTypeName ()));
	    return buf.str ();
	}	

	const Generator & FuncPtr::getReturnType () const {
	    return this-> getInners ()[0];
	}

	std::vector <Generator> FuncPtr::getParamTypes () const {
	    std::vector <Generator> ret (this-> getInners ().begin () + 1, this-> getInners ().end ());
	    return ret;
	}
	
    }
}
