#include <ymir/semantic/generator/Frame.hh>

namespace semantic {
    namespace generator {

	Frame::Frame () :
	    IGenerator (lexing::Word::eof (), ""),
	    _params (),
	    _type (Generator::empty ()),
	    _content (Generator::empty ())
	{}
	
	Frame::Frame (const lexing::Word & location, const std::string & name, const std::vector <Generator> & params, const Generator & type, const Generator & content, bool needFinalReturn) :
	    IGenerator (location, name),
	    _params (params),
	    _type (type),
	    _content (content),
	    _needFinalReturn (needFinalReturn)
	{}

	Generator Frame::init (const lexing::Word & location, const std::string & name, const std::vector <Generator> & params, const Generator & type, const Generator & content, bool needFinalReturn) {
	    return Generator {new (NO_GC) Frame (location, name, params, type, content, needFinalReturn)};
	}

	Generator Frame::clone () const {
	    return Generator {new (NO_GC) Frame (*this)};
	}

	const Generator & Frame::getType () const {
	    return this-> _type;
	}

	const std::vector <Generator> & Frame::getParams () const {
	    return this-> _params;
	}
	
	const Generator & Frame::getContent () const {
	    return this-> _content;
	}
       
	bool Frame::equals (const Generator & other) const {
	    if (!other.is <Frame> ()) return false;
	    else 
		return other.getName () == this-> getName ();
	}

	bool Frame::needFinalReturn () const {
	    return this-> _needFinalReturn;
	}

	void Frame::setManglingStyle (Frame::ManglingStyle style) {
	    this-> _style = style;
	}

	Frame::ManglingStyle Frame::getManglingStyle () const {
	    return this-> _style;
	}

	void Frame::isWeak (bool is) {
	    this-> _isWeak = is;
	}

	bool Frame::isWeak () const {
	    return this-> _isWeak;
	}

	void Frame::setMangledName (const std::string & mangleName) {
	    this-> _mangleName = mangleName;
	}

	const std::string& Frame::getMangledName () const {
	    return this-> _mangleName;
	}
	
    }
}
