#include <ymir/semantic/generator/value/ConstructorProto.hh>
#include <ymir/semantic/generator/type/Void.hh>
#include <ymir/semantic/generator/value/ProtoVar.hh>
#include <ymir/semantic/generator/type/LambdaType.hh>

namespace semantic {

    namespace generator {

	ConstructorProto::ConstructorProto () :
	    Value (),
	    _params ({}),
	    _type (Generator::empty ()),
	    _name (""),
	    _ref (Symbol::empty ())
	{}

	ConstructorProto::ConstructorProto (const lexing::Word & loc, const std::string & name, const Symbol & ref, const Generator & type, const std::vector<Generator> & params) :
	    Value (loc, LambdaType::init (loc, type, getTypes (params))),
	    _params (params),
	    _type (type),
	    _name (name),
	    _ref (ref)
	{}

	std::vector <Generator> ConstructorProto::getTypes (const std::vector <Generator> & params) {
	    std::vector <Generator> types;
	    for (auto & it : params) {
		types.push_back (it.to <ProtoVar> ().getType ());		    
	    }
	    return types;
	}
		   
	Generator ConstructorProto::init (const lexing::Word & loc, const std::string &name, const Symbol & ref, const Generator & type, const std::vector<Generator> & params) {
	    return Generator {new ConstructorProto (loc, name, ref, type, params)};
	}
    
	Generator ConstructorProto::clone () const {
	    return Generator {new (Z0) ConstructorProto (*this)};
	}

	bool ConstructorProto::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    ConstructorProto thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return Value::isOf (type);	
	}

	bool ConstructorProto::equals (const Generator & gen) const {
	    if (!gen.is <ConstructorProto> ()) return false;
	    auto fr = gen.to<ConstructorProto> ();
	    if (fr.getMangledName () != this-> getMangledName ()) return false;
	    if (fr.getParameters ().size () != this-> _params.size ()) return false;
	    if (!fr.getReturnType ().equals (this-> _type)) return false;
	    for (auto it : Ymir::r (0, this-> _params.size ())) {
		if (!fr.getParameters () [it].equals (this-> _params [it])) return false;
	    }
	    return true;
	}

	const std::vector <Generator> & ConstructorProto::getParameters () const {
	    return this-> _params;
	}

	const Generator & ConstructorProto::getReturnType () const {
	    return this-> _type;
	}

	std::string ConstructorProto::prettyString () const {
	    std::vector <std::string> content;
	    for (auto & it : this-> _params) {
		content.push_back (Ymir::entab (it.prettyString ()));		
		if (content.back ().size () != 0 && content.back ().back () == '\n')
		    content.back () = content.back ().substr (0, content.back ().size () - 1);
	    }
	    return Ymir::format ("self (%)-> %", content, this-> _type.prettyString ());
	}
	

	void ConstructorProto::setMangledName (const std::string & name) {
	    this-> _mangleName = name;
	}

	const std::string & ConstructorProto::getMangledName () const {	    
	    return this-> _mangleName;
	}

	const std::string & ConstructorProto::getName () const {
	    return this-> _name;
	}

	Symbol ConstructorProto::getRef () const {
	    return this-> _ref;
	}
	
    }
    
}
