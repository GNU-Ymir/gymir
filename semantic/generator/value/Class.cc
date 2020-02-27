#include <ymir/semantic/generator/value/Class.hh>
#include <ymir/semantic/generator/type/NoneType.hh>
#include <ymir/semantic/generator/value/VarDecl.hh>
#include <ymir/semantic/symbol/Class.hh>
#include <ymir/syntax/expression/VarDecl.hh>
#include <ymir/utils/OutBuffer.hh>

namespace semantic {
    namespace generator {
	
	Class::Class () :
	    Value (),
	    _classRef (Generator::empty ())
	{
	}

	Class::Class (const lexing::Word & loc, const Symbol & ref, const Generator & clRef) :
	    Value (loc, loc.str, NoneType::init (loc, "class " + ref.getRealName ())),
	    _classRef (clRef)
	{
	    auto aux = ref;
	    this-> _ref = aux.getPtr ();
	}

	Generator Class::init (const lexing::Word & loc, const Symbol & ref, const Generator & clRef) {
	    return Generator {new (Z0) Class (loc, ref, clRef)};
	}

	Generator Class::initFields (const Class & other, const std::vector <generator::Generator> & fields) {
	    auto ret = other.clone ();
	    ret.to <Class> ()._fields = fields;
	    return ret;
	}

	Generator Class::initVtable (const Class & other, const std::vector <generator::Generator> & vtable, const std::vector <MethodProtection> & prots) {
	    auto ret = other.clone ();
	    ret.to <Class> ()._vtable = vtable;
	    ret.to <Class> ()._prots = prots;
	    return ret;
	}
	
	Generator Class::clone () const {
	    return Generator {new (Z0) Class (*this)};
	}
		
	bool Class::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    Class thisClass; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisClass) [0] == vtable) return true;
	    return Value::isOf (type);	
	}

	bool Class::equals (const Generator & gen) const {
	    if (!gen.is<Class> ()) return false;
	    auto str = gen.to <Class> ();
	    return (Symbol {this-> _ref}).isSameRef (Symbol {str._ref});
	}

	const std::vector <generator::Generator> & Class::getFields () const {
	    return this-> _fields;
	}


	Generator Class::getFieldType (const std::string & name) const {
	    Generator type (Generator::empty ());

	    // Those fields contains all the fields of the class including ancestor
	    for (auto & it: this-> _fields) { 
		if (it.to <generator::VarDecl> ().getName () == name) {
		    type = it.to <generator::VarDecl> ().getVarType ();
		    break;
		}
	    }

	    if (type.isEmpty ()) return type;
	    
	    // so we need to check that it belong to this class and not to an ancestor	    
	    for (auto & it : (Symbol {this-> _ref}).to <semantic::Class> ().getFields ()) {
		if (it.to <syntax::VarDecl> ().getName ().str == name) return type;
	    }
	    return Generator::empty ();
	}

	Generator Class::getFieldTypeProtected (const std::string & name) const {
	    Generator type (Generator::empty ());
	    // Those fields contains all the fields of the class including ancestor
	    for (auto & it: this-> _fields) {
		if (it.to <generator::VarDecl> ().getName () == name) {
		    type = it.to <generator::VarDecl> ().getVarType ();
		    break;
		}
	    }

	    auto ref = Symbol {this-> _ref};
	    if (type.isEmpty ()) return type;
	    // so we need to check that it belong to this class and not to an ancestor	    
	    for (auto & it : ref.to <semantic::Class> ().getFields ()) {
		if (it.to <syntax::VarDecl> ().getName ().str == name) {
		    if (ref.to <semantic::Class> ().isMarkedPrivate (name))
			return Generator::empty ();
		    return type;
		}
	    }
	    return Generator::empty ();	    
	}

	Generator Class::getFieldTypePublic (const std::string & name) const {
	    Generator type (Generator::empty ());
	    // Those fields contains all the fields of the class including ancestor
	    for (auto & it: this-> _fields) {
		if (it.to <generator::VarDecl> ().getName () == name) {
		    type = it.to <generator::VarDecl> ().getVarType ();
		    break;
		}
	    }

	    auto ref = Symbol {this-> _ref};
	    if (type.isEmpty ()) return type;
	    // so we need to check that it belong to this class and not to an ancestor	    
	    for (auto & it : ref.to <semantic::Class> ().getFields ()) {
		if (it.to <syntax::VarDecl> ().getName ().str == name) {
		    if (ref.to <semantic::Class> ().isMarkedPrivate (name) ||
			ref.to <semantic::Class> ().isMarkedProtected (name))
			return Generator::empty ();
		    return type;
		}
	    }
	    return Generator::empty ();	    
	}

	
	const std::vector <generator::Generator> & Class::getVtable () const {
	    return this-> _vtable;
	}

	const std::vector <generator::Class::MethodProtection> & Class::getProtectionVtable () const {
	    return this-> _prots;
	}
	
	std::string Class::getName () const {
	    return (Symbol {this-> _ref}).getRealName ();
	}
	
	std::string Class::prettyString () const {
	    return Ymir::format ("%", (Symbol {this-> _ref}).getRealName ());
	}

	Symbol Class::getRef () const {
	    return Symbol {this-> _ref};
	}

	const Generator & Class::getClassRef () const {
	    return this-> _classRef;
	}
	
    }
}
