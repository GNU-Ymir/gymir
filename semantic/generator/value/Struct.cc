#include <ymir/semantic/generator/value/Struct.hh>
#include <ymir/semantic/generator/type/NoneType.hh>
#include <ymir/semantic/generator/value/VarDecl.hh>
#include <ymir/utils/OutBuffer.hh>

namespace semantic {
    namespace generator {
	
	Struct::Struct () :
	    Value ()
	{
	}

	Struct::Struct (const lexing::Word & loc, const Symbol & ref) :
	    Value (loc, loc.getStr (), NoneType::init (loc, "struct " + ref.getRealName ()))
	{
	    auto aux = ref;
	    this-> _ref = aux.getPtr ();
	}

	Generator Struct::init (const lexing::Word & loc, const Symbol & ref) {
	    return Generator {new (NO_GC) Struct (loc, ref)};
	}

	Generator Struct::init (const Struct & other, const std::vector <Generator> & fields) {
	    auto ret = other.clone ();
	    ret.to <Struct> ()._fields = fields;
	    return ret;
	}

	Generator Struct::clone () const {
	    return Generator {new (NO_GC) Struct (*this)};
	}
		
	bool Struct::equals (const Generator & gen) const {
	    if (!gen.is<Struct> ()) return false;
	    auto str = gen.to <Struct> ();
	    return (Symbol {this-> _ref}).isSameRef (str._ref);
	}

	const std::vector <generator::Generator> & Struct::getFields () const {
	    return this-> _fields;
	}

	std::string Struct::getName () const {
	    return (Symbol {this-> _ref}).getRealName ();
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
	    return Ymir::format ("% (%)", (Symbol {this-> _ref}).getRealName (), content);
	}

	Symbol Struct::getRef () const {
	    return Symbol {this-> _ref};
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
