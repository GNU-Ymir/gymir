#include <ymir/semantic/generator/value/FrameProto.hh>
#include <ymir/semantic/generator/type/Void.hh>
#include <ymir/semantic/generator/value/ProtoVar.hh>
#include <ymir/semantic/generator/type/LambdaType.hh>

namespace semantic {

    namespace generator {

	FrameProto::FrameProto () :
	    Value (),
	    _params ({}),
	    _type (Generator::empty ())	    
	{}

	FrameProto::FrameProto (const lexing::Word & loc, const std::string & name, const Generator & type, const std::vector<Generator> & params, bool isCVariadic, bool isSafe, const std::vector <Generator> & throwers) :
	    Value (loc, LambdaType::init (loc, type, getTypes (params))),
	    _params (params),
	    _type (type),
	    _name (name),
	    _isCVariadic (isCVariadic),
	    _isSafe (isSafe)
	{
	    auto th = throwers;
	    for (auto &it : th) it = Generator::init (loc, it);
	    this-> setThrowers (th);		
	}

	std::vector <Generator> FrameProto::getTypes (const std::vector <Generator> & params) {
	    std::vector <Generator> types;
	    for (auto & it : params) {
		types.push_back (it.to <ProtoVar> ().getType ());		    
	    }
	    return types;
	}
		   
	Generator FrameProto::init (const lexing::Word & loc, const std::string & name, const Generator & type, const std::vector<Generator> & params, bool isCVariadic, bool isSafe, const std::vector <Generator> & throwers) {
	    return Generator {new (Z0) FrameProto (loc, name, type, params, isCVariadic, isSafe, throwers)};
	}

	Generator FrameProto::init (const FrameProto & proto, const std::string & name, Frame::ManglingStyle style) {
	    auto ret = proto.clone ();
	    ret.to <FrameProto> ()._mangleName = name;
	    ret.to <FrameProto> ()._style = style;
	    return ret;
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

	Frame::ManglingStyle FrameProto::getManglingStyle () const {
	    return this-> _style;
	}

	const std::string & FrameProto::getMangledName () const {	    
	    return this-> _mangleName;
	}

	bool FrameProto::isCVariadic () const {
	    return this-> _isCVariadic;
	}
    }
    
}
