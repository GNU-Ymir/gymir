#include <ymir/semantic/generator/value/LambdaProto.hh>
#include <ymir/semantic/generator/type/Void.hh>
#include <ymir/semantic/generator/value/ProtoVar.hh>
#include <ymir/semantic/generator/type/LambdaType.hh>

namespace semantic {

    namespace generator {

	LambdaProto::LambdaProto () :
	    Value (),
	    _params ({}),
	    _type (Generator::empty ()),
	    _name (""),
	    _content (syntax::Expression::empty ())
	{
	}

	LambdaProto::LambdaProto (const lexing::Word & loc, const std::string & name, const Generator & type, const std::vector<Generator> & params, const syntax::Expression & content) :
	    Value (loc, LambdaType::init (loc, type, getTypes (params))),
	    _params (params),
	    _type (type),
	    _name (name),
	    _content (content)
	{}

	std::vector <Generator> LambdaProto::getTypes (const std::vector <Generator> & params) {
	    std::vector <Generator> types;
	    for (auto & it : params) {
		types.push_back (it.to <ProtoVar> ().getType ());		    
	    }
	    return types;
	}
	
	Generator LambdaProto::init (const lexing::Word & loc, const std::string & name, const Generator & type, const std::vector<Generator> & params, const syntax::Expression & content) {
	    return Generator {new LambdaProto (loc, name, type, params, content)};
	}
    
	Generator LambdaProto::clone () const {
	    return Generator {new (Z0) LambdaProto (*this)};
	}

	bool LambdaProto::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    LambdaProto thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return Value::isOf (type);	
	}

	bool LambdaProto::equals (const Generator & gen) const {
	    if (!gen.is <LambdaProto> ()) return false;
	    auto fr = gen.to<LambdaProto> ();
	    if (fr.getMangledName () != this-> getMangledName ()) return false;
	    if (fr.getParameters ().size () != this-> _params.size ()) return false;
	    if (!fr.getReturnType ().equals (this-> _type)) return false;
	    for (auto it : Ymir::r (0, this-> _params.size ())) {
		if (!fr.getParameters () [it].equals (this-> _params [it])) return false;
	    }	    
	    return true;
	}

	const std::vector <Generator> & LambdaProto::getParameters () const {
	    return this-> _params;
	}

	const Generator & LambdaProto::getReturnType () const {
	    return this-> _type;
	}

	const std::string & LambdaProto::getName () const {
	    return this-> _name;
	}

	std::string LambdaProto::prettyString () const {
	    std::vector <std::string> content;
	    for (auto & it : this-> _params) {
		if (it.to <ProtoVar> ().getType ().isEmpty ())
		    content.push_back (Ymir::entab ("_"));
		else
		    content.push_back (Ymir::entab (it.prettyString ()));
		
		if (content.back ().size () != 0 && content.back ().back () == '\n')
		    content.back () = content.back ().substr (0, content.back ().size () - 1);
	    }
	    if (this-> _type.isEmpty ())
		return Ymir::format ("% (%)-> _", this-> _name, content);
	    else
		return Ymir::format ("% (%)-> %", this-> _name, content, this-> _type.prettyString ());
	}
	
	void LambdaProto::setManglingStyle (Frame::ManglingStyle style) {
	    this-> _style = style;
	}

	Frame::ManglingStyle LambdaProto::getManglingStyle () const {
	    return this-> _style;
	}

	void LambdaProto::setMangledName (const std::string & name) {
	    this-> _mangleName = name;
	}

	const std::string & LambdaProto::getMangledName () const {	    
	    return this-> _mangleName;
	}

	const syntax::Expression & LambdaProto::getContent () const {
	    return this-> _content;
	}
	
    }
    
}
