#include <ymir/semantic/generator/value/MethodProto.hh>
#include <ymir/semantic/generator/type/Void.hh>
#include <ymir/semantic/generator/value/ProtoVar.hh>
#include <ymir/semantic/generator/type/LambdaType.hh>

namespace semantic {

    namespace generator {

	MethodProto::MethodProto () :
	    FrameProto (),
	    _isMut (false),
	    _classType (Generator::empty ())	    
	{}

	MethodProto::MethodProto (const lexing::Word & loc, const std::string & name, const Generator & type, const std::vector<Generator> & params, bool isCVariadic, const Generator& classType, bool isMutable, bool isEmptyFrame) :
	    FrameProto (loc, name, type, params, isCVariadic),
	    _isMut (isMutable),
	    _classType (classType),
	    _isEmptyFrame (isEmptyFrame)
	{}

	Generator MethodProto::init (const lexing::Word & loc, const std::string & name, const Generator & type, const std::vector<Generator> & params, bool isCVariadic, const Generator& classType, bool isMutable, bool isEmptyFrame) {
	    return Generator {new MethodProto (loc, name, type, params, isCVariadic, classType, isMutable, isEmptyFrame)};
	}
	
	Generator MethodProto::clone () const {
	    return Generator {new (Z0) MethodProto (*this)};
	}

	bool MethodProto::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    MethodProto thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return FrameProto::isOf (type);	
	}

	bool MethodProto::equals (const Generator & gen) const {
	    if (!gen.is <MethodProto> ()) return false;
	    auto fr = gen.to<MethodProto> ();
	    if (fr.getMangledName () != this-> getMangledName ()) return false;
	    if (fr.getParameters ().size () != this-> _params.size ()) return false;
	    if (!fr.getReturnType ().equals (this-> _type)) return false;
	    for (auto it : Ymir::r (0, this-> _params.size ())) {
		if (!fr.getParameters () [it].equals (this-> _params [it])) return false;
	    }
	    return true;
	}

	std::string MethodProto::prettyString () const {
	    auto buf = "(" + this-> _classType.prettyString () + ") => " + FrameProto::prettyString ();
	    if (this-> _isMut) buf = "mut " + buf;
	    return buf;
	}

	const Generator & MethodProto::getClassType () const {
	    return this-> _classType;
	}

	bool MethodProto::isMutable () const {
	    return this-> _isMut;
	}

	bool MethodProto::isEmptyFrame () const {
	    return this-> _isEmptyFrame;
	}
	
    }
    
}
