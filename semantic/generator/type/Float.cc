#include <ymir/semantic/generator/type/Float.hh>
#include <ymir/utils/OutBuffer.hh>

namespace semantic {
    namespace generator {

	std::vector <std::string> Float::NAMES = {"f32", "f64", "fsize"};

	std::string Float::NOT_A_NUMBER = "NAN";
	
	std::string Float::INIT = Float::NOT_A_NUMBER;

	std::string Float::INF_NAME = "inf";

	std::string Float::INF = "INF";

	std::string Float::MIN_NAME = "min";

	std::string Float::MAX_NAME = "max";

	std::string Float::NAN_NAME = "nan";

	std::string Float::DIG_NAME = "dig";

	std::string Float::MANT_DIG_NAME = "mant_dig";

	std::string Float::EPSILON_NAME = "epsilon";

	std::string Float::MAX_10_EXP_NAME = "max_10_exp";

	std::string Float::MAX_EXP_NAME = "max_exp";

	std::string Float::MIN_10_EXP_NAME = "min_10_exp";

	std::string Float::MIN_EXP_NAME = "min_exp";
	
	Float::Float () :
	    Type ()		  
	{}

	Float::Float (const lexing::Word & loc, int size) :
	    Type (loc, loc.getStr ()),
	    _size (size)
	{}

	Generator Float::init (const lexing::Word & loc, int size) {
	    return Generator {new (NO_GC) Float (loc, size)};
	}

	Generator Float::clone () const {
	    return Generator {new (NO_GC) Float (*this)};
	}
		
	bool Float::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    Float thisFloat; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisFloat) [0] == vtable) return true;
	    return Type::isOf (type);	
	}

	bool Float::equals (const Generator & gen) const {
	    if (!gen.is <Float> ()) return false;
	    auto integer = gen.to<Float> ();
	    return this-> _size == integer._size;
	}

	std::string Float::typeName () const {
	    if (this-> _size > 0)
		return Ymir::OutBuffer ("f", this-> _size).str ();
	    return "fsize";
	}	
    
	int Float::getSize () const {
	    return this-> _size;
	}
	
    }
}
