#include <ymir/semantic/generator/value/Struct.hh>
#include <ymir/semantic/generator/type/Void.hh>
#include <ymir/semantic/generator/value/VarDecl.hh>
#include <ymir/utils/OutBuffer.hh>

namespace semantic {
    namespace generator {
	
	Struct::Struct () :
	    Value (),
	    _ref (Symbol::__empty__)
	{
	}

	Struct::Struct (const lexing::Word & loc, const Symbol & ref) :
	    Value (loc, loc.str, Void::init (loc)),
	    _ref (ref)
	{}

	Generator Struct::init (const lexing::Word & loc, const Symbol & ref) {
	    return Generator {new (Z0) Struct (loc, ref)};
	}

	Generator Struct::clone () const {
	    return Generator {new (Z0) Struct (*this)};
	}
		
	bool Struct::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    Struct thisStruct; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisStruct) [0] == vtable) return true;
	    return Value::isOf (type);	
	}

	bool Struct::equals (const Generator & gen) const {
	    if (!gen.is<Struct> ()) return false;
	    auto str = gen.to <Struct> ();
	    return this-> _ref.isSameRef (str._ref);
	}

	const std::vector <generator::Generator> & Struct::getFields () const {
	    return this-> _fields;
	}

	void Struct::setFields (const std::vector <generator::Generator> & fields) {
	    this-> _fields = fields;
	}

	std::string Struct::getName () const {
	    return this-> _ref.getRealName ();
	}
	
	Generator Struct::getFieldType (const std::string & name) const {
	    for (auto & it : this-> _fields) {
		if (it.to <generator::VarDecl> ().getName () == name)
		    return it.to <generator::VarDecl> ().getVarType ();
	    }
	    return Generator::empty ();
	}
	
	std::string Struct::prettyString () const {
	    std::vector <std::string> content;
	    for (auto & it : this-> _fields) {
		content.push_back (Ymir::entab (it.prettyString ()));
		if (content.back ().size () != 0 && content.back ().back () == '\n')
		    content.back () = content.back ().substr (0, content.back ().size () - 1);
	    }
	    return Ymir::format ("% (%)", this-> _ref.getRealName (), content);
	}

	const Symbol & Struct::getRef () const {
	    return this-> _ref;
	}

	bool Struct::hasComplexField () const {
	    for (auto & it : this-> _fields) {
		auto & type = it.to <generator::VarDecl> ().getVarType ().to<Type> ();		
		if (type.isComplex () && type.mutabilityLevel () != 0)
		    return true;
	    }
	    return false;
	}
	
    }
}
