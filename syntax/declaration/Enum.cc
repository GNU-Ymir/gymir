#include <ymir/syntax/declaration/Enum.hh>

namespace syntax {

    Enum::Enum () :
	IDeclaration (lexing::Word::eof (), ""),
	_type (Expression::empty ())
    {}

    Enum::Enum (const lexing::Word & ident, const std::string & comment, const Expression& type,  const std::vector <Expression> & values, const std::vector <std::string> & fieldComments) :
	IDeclaration (ident, comment),
	_values (values),
	_type (type),
	_field_comments (fieldComments)
    {}
    
    Declaration Enum::init (const lexing::Word & ident, const std::string & comment, const Expression& type,  const std::vector <Expression> & values, const std::vector <std::string> & fieldComments) {
	return Declaration {new (NO_GC) Enum (ident, comment, type, values, fieldComments)};
    }
    
    const Expression & Enum::getType () const {
	return this-> _type;
    }

    const std::vector <Expression> & Enum::getValues () const {
	return this-> _values;
    }
    
    const std::vector <std::string> & Enum::getFieldComments () const {
	return this-> _field_comments;
    }

    void Enum::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*", i, '\t');
	stream.writeln ("<Enum> : ", this-> getLocation (), ":");
	this-> _type.treePrint (stream, i + 1);
	
	for (auto & it : this-> _values) {
	    stream.writef ("%*", i + 1, '\t');
	    stream.writeln ("<EnumValue> : ");
	    it.treePrint (stream, i + 2);
	}
    }

    const std::set <std::string> & Enum::computeSubVarNames () {
	auto tSet = this-> _type.getSubVarNames ();
	for (auto & it : this-> _values) {
	    auto & iSet = it.getSubVarNames ();
	    tSet.insert (iSet.begin (), iSet.end ());
	}
	this-> setSubVarNames (tSet);
	return this-> getSubVarNames ();
    }
    
}
