#include <ymir/semantic/generator/value/ConstructorProto.hh>
#include <ymir/semantic/generator/type/Void.hh>
#include <ymir/semantic/generator/value/ProtoVar.hh>
#include <ymir/semantic/generator/type/LambdaType.hh>
#include <ymir/semantic/symbol/Constructor.hh>
#include <ymir/utils/Path.hh>

namespace semantic {

    namespace generator {

	ConstructorProto::ConstructorProto () :
	    Value (),
	    _params ({}),
	    _type (Generator::empty ()),
	    _name ("")
	{}

	ConstructorProto::ConstructorProto (const lexing::Word & loc, const std::string & name, const Symbol & ref, const Generator & type, const std::vector<Generator> & params, const std::vector <Generator> &throwers) :
	    Value (loc, LambdaType::init (loc, type, getTypes (params))),
	    _params (params),
	    _type (type),
	    _name (name)
	{
	    auto aux = ref;
	    this-> _ref = aux.getPtr ();
	    
	    auto th = throwers;
	    for (auto &it : th) it = Generator::init (loc, it);
	    this-> setThrowers (th);	    
	}

	std::vector <Generator> ConstructorProto::getTypes (const std::vector <Generator> & params) {
	    std::vector <Generator> types;
	    for (auto & it : params) {
		types.push_back (it.to <ProtoVar> ().getType ());		    
	    }
	    return types;
	}
		   
	Generator ConstructorProto::init (const lexing::Word & loc, const std::string &name, const Symbol & ref, const Generator & type, const std::vector<Generator> & params, const std::vector <Generator> &throwers) {
	    return Generator {new (NO_GC) ConstructorProto (loc, name, ref, type, params, throwers)};
	}

	Generator ConstructorProto::init (const ConstructorProto & proto, const std::string & name) {
	    auto ret = proto.clone ();
	    ret.to <ConstructorProto>()._mangleName = name;
	    return ret;
	}
	
	Generator ConstructorProto::clone () const {
	    return Generator {new (NO_GC) ConstructorProto (*this)};
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
	    Ymir::Path p (this-> _name, "::");
	    if (p.fileName ().toString () != "self") {
		return Ymir::format ("self % (%)-> %", p.fileName ().toString (), content, this-> _type.prettyString ());
	    } else {
		return Ymir::format ("self (%)-> %", content, this-> _type.prettyString ());
	    }
	}
	

	const std::string & ConstructorProto::getMangledName () const {	    
	    return this-> _mangleName;
	}

	const std::string & ConstructorProto::getName () const {
	    return this-> _name;
	}

	Symbol ConstructorProto::getRef () const {
	    return Symbol {this-> _ref.lock ()};
	}
	
    }
    
}
