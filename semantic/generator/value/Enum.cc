#include <ymir/semantic/generator/value/Enum.hh>
#include <ymir/semantic/generator/type/Void.hh>
#include <ymir/semantic/generator/value/VarDecl.hh>
#include <ymir/utils/OutBuffer.hh>

namespace semantic {
    namespace generator {
	
	Enum::Enum () :
	    Value (),
	    _ref (Symbol::__empty__),
	    _type (Generator::empty ())
	{
	}

	Enum::Enum (const lexing::Word & loc, const Symbol & ref) :
	    Value (loc, loc.str, Void::init (loc)),
	    _ref (ref),
	    _type (Generator::empty ())
	{}

	Generator Enum::init (const lexing::Word & loc, const Symbol & ref) {
	    return Generator {new (Z0) Enum (loc, ref)};
	}

	Generator Enum::clone () const {
	    return Generator {new (Z0) Enum (*this)};
	}
		
	bool Enum::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    Enum thisEnum; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisEnum) [0] == vtable) return true;
	    return Value::isOf (type);	
	}

	bool Enum::equals (const Generator & gen) const {
	    if (!gen.is<Enum> ()) return false;
	    auto str = gen.to <Enum> ();
	    return this-> _ref.isSameRef (str._ref);
	}

	Generator Enum::getType () const {
	    return this-> _type;
	}

	void Enum::setType (const Generator & type) {
	    this-> _type = type;
	}
	
	const std::vector <generator::Generator> & Enum::getFields () const {
	    return this-> _fields;
	}

	const generator::Generator  Enum::getFieldValue (const std::string & name) const {
	    for (auto & it : this-> _fields){
		if (it.to <VarDecl> ().getName () == name)
		    return it.to <VarDecl> ().getVarValue ();
	    }
	    return Generator::empty ();
	}	  
	
	void Enum::setFields (const std::vector <generator::Generator> & fields) {
	    this-> _fields = fields;
	}

	std::string Enum::getName () const {
	    return this-> _ref.getRealName ();
	}
		
	std::string Enum::prettyString () const {
	    std::vector <std::string> content;
	    for (auto & it : this-> _fields) {
		content.push_back (Ymir::entab (it.prettyString ()));
		if (content.back ().size () != 0 && content.back ().back () == '\n')
		    content.back () = content.back ().substr (0, content.back ().size () - 1);
	    }
	    return Ymir::format ("% (%)", this-> _ref.getRealName (), content);
	}

	const Symbol & Enum::getRef () const {
	    return this-> _ref;
	}
	
    }
}
