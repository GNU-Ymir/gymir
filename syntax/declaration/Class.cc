#include <ymir/syntax/declaration/Class.hh>

namespace syntax {

    Class::Class () :
	IDeclaration (lexing::Word::eof (), ""), 
	_over (Expression::empty ())
    {}
    
    Class::Class (const lexing::Word & name, const std::string & comment, const Expression & over, const std::vector <Declaration> & decls, const std::vector <lexing::Word> & attribts) :
	IDeclaration (name, comment),
	_over (over),
	_innerDeclaration (decls),
	_attributes (attribts)
    {}
    
    Declaration Class::init (const Class & cl) {
	return Declaration {new (NO_GC) Class (cl)};
    }

    Declaration Class::init (const lexing::Word & name, const std::string & comment, const Expression & over, const std::vector <Declaration> & decls, const std::vector <lexing::Word> & attribs) {
	return Declaration {new (NO_GC) Class (name, comment, over, decls, attribs)};
    }
   
    const Expression & Class::getAncestor () const {
	return this-> _over;
    }

    const std::vector <Declaration> & Class::getDeclarations () const {
	return this-> _innerDeclaration;
    }

    const std::vector <lexing::Word> & Class::getAttributes () const {
	return this-> _attributes;
    }

    bool Class::isAbstract () const {
	for (auto & it : this-> _attributes) {
	    if (it == Keys::ABSTRACT) return true;
	}
	return false;
    }

    const std::set <std::string> & Class::computeSubVarNames () {
	auto oSet = this-> _over.getSubVarNames ();
	for (auto & it : this-> _innerDeclaration) {
	    auto & iSet = it.getSubVarNames ();
	    oSet.insert (iSet.begin (), iSet.end ());
	}
	
	this-> setSubVarNames (oSet);	
	return this-> getSubVarNames ();
    }
    
}
