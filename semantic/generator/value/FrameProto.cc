#include <ymir/semantic/generator/value/FrameProto.hh>
#include <ymir/semantic/generator/type/Void.hh>

namespace semantic {

    namespace generator {

	FrameProto::FrameProto () :
	    Value (),
	    _params ({}),
	    _type (Generator::empty ())
	{}

	FrameProto::FrameProto (const lexing::Word & loc, const std::string & name, const Generator & type, const std::vector<Generator> & params) :
	    Value (loc, Void::init (loc)),
	    _params (params),
	    _type (type),
	    _name (name)
	{}
	
	Generator FrameProto::init (const lexing::Word & loc, const std::string & name, const Generator & type, const std::vector<Generator> & params) {
	    return Generator {new FrameProto (loc, name, type, params)};
	}
    
	Generator FrameProto::clone () const {
	    return Generator {new (Z0) FrameProto (*this)};
	}

	bool FrameProto::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    FrameProto thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return Value::isOf (type);	
	}

	bool FrameProto::equals (const Generator & gen) const {
	    if (!gen.is <FrameProto> ()) return false;
	    auto fr = gen.to<FrameProto> ();
	    if (fr.getMangledName () != this-> getMangledName ()) return false;
	    if (fr.getParameters ().size () != this-> _params.size ()) return false;
	    if (!fr.getReturnType ().equals (this-> _type)) return false;
	    for (auto it : Ymir::r (0, this-> _params.size ())) {
		if (!fr.getParameters () [it].equals (this-> _params [it])) return false;
	    }
	    return true;
	}

	const std::vector <Generator> & FrameProto::getParameters () const {
	    return this-> _params;
	}

	const Generator & FrameProto::getReturnType () const {
	    return this-> _type;
	}

	const std::string & FrameProto::getName () const {
	    return this-> _name;
	}

	std::string FrameProto::prettyString () const {
	    std::vector <std::string> content;
	    for (auto & it : this-> _params) {
		content.push_back (Ymir::entab (it.prettyString ()));		
		if (content.back ().size () != 0 && content.back ().back () == '\n')
		    content.back () = content.back ().substr (0, content.back ().size () - 1);
	    }
	    return Ymir::format ("% (%)-> %", this-> _name, content, this-> _type.prettyString ());
	}
	
	void FrameProto::setManglingStyle (Frame::ManglingStyle style) {
	    this-> _style = style;
	}

	Frame::ManglingStyle FrameProto::getManglingStyle () const {
	    return this-> _style;
	}

	void FrameProto::setMangledName (const std::string & name) {
	    this-> _mangleName = name;
	}

	const std::string & FrameProto::getMangledName () const {	    
	    return this-> _mangleName;
	}
    }
    
}
