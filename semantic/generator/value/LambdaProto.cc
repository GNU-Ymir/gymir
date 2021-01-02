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

	LambdaProto::LambdaProto (const lexing::Word & loc, const std::string & name, const Generator & type, const std::vector<Generator> & params, const syntax::Expression & content, bool isRefClosure, bool isMoveClosure, uint index) :
	    Value (loc, LambdaType::init (loc, type, getTypes (params))),
	    _params (params),
	    _type (type),
	    _name (name),
	    _content (content),
	    _isRefClosure (isRefClosure),
	    _isMoveClosure (isMoveClosure),
	    _index (index)
	{
	    // Logically, a lambda proto can't throw anything
	    // I suppose its safer to prohibit any throw inside lambdas,
	    // I can't see how we can check the throwing system when using lambdas
	}

	std::vector <Generator> LambdaProto::getTypes (const std::vector <Generator> & params) {
	    std::vector <Generator> types;
	    for (auto & it : params) {
		types.push_back (it.to <ProtoVar> ().getType ());		    
	    }
	    return types;
	}
	
	Generator LambdaProto::init (const lexing::Word & loc, const std::string & name, const Generator & type, const std::vector<Generator> & params, const syntax::Expression & content, bool isRefClosure, bool isMoveClosure, uint index) {
	    return Generator {new (NO_GC) LambdaProto (loc, name, type, params, content, isRefClosure, isMoveClosure, index)};
	}

	Generator LambdaProto::init (const LambdaProto & other, const std::string & name, Frame::ManglingStyle style) {
	    auto ret = other.clone ();
	    ret.to <LambdaProto> ()._mangleName = name;
	    ret.to <LambdaProto> ()._style = style;
	    return ret;
	}
    
	Generator LambdaProto::clone () const {
	    return Generator {new (NO_GC) LambdaProto (*this)};
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
	

	Frame::ManglingStyle LambdaProto::getManglingStyle () const {
	    return this-> _style;
	}

	const std::string & LambdaProto::getMangledName () const {	    
	    return this-> _mangleName;
	}

	const syntax::Expression & LambdaProto::getContent () const {
	    return this-> _content;
	}

	bool LambdaProto::isRefClosure () const {
	    return this-> _isRefClosure;
	}

	bool LambdaProto::isMoveClosure () const {
	    return this-> _isMoveClosure;	    
	}

	uint LambdaProto::getClosureIndex () const {
	    return this-> _index;
	}
	
    }
    
}
