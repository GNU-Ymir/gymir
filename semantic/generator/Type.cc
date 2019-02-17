#include <ymir/semantic/generator/Type.hh>

namespace semantic {

    namespace generator {

	Type::Type () :
	    IGenerator (lexing::Word::eof (), "")
	{}

	Type::Type (const lexing::Word & loc, const std::string & name) :
	    IGenerator (loc, name)
	{}
	
	Generator Type::clone () const {
	    return Generator{new (Z0) Type ()};
	} 
	
	bool Type::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    Type thisType; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	    return IGenerator::isOf (type);	
	}

	bool Type::equals (const Generator &) const {
	    return false;
	}

	bool Type::isCompatible (const Generator & gen) const {
	    return this-> equals (gen);
	}
	
	std::string Type::getTypeName (bool isParentMutable) const {
	    auto inner = this-> typeName ();
	    if (this-> _isMutable && isParentMutable)
		return "mut " + inner;
	    return inner;
	}

	std::string Type::typeName () const {
	    return "undef";
	}	

	bool Type::isRef () const {
	    return this-> _isRef;
	}

	bool Type::isMutable () const {
	    return this-> _isMutable;
	}

	bool Type::isComplex () const {
	    return this-> _isComplex;
	}
	
	void Type::isRef (bool is) {
	    this-> _isRef = is;
	}
	
	void Type::isMutable (bool is) {	    
	    this-> _isMutable = is;
	}

	void Type::isComplex (bool is) {
	    this-> _isComplex = is;
	}

	bool Type::isLocal () const {
	    if (!this-> _isComplex)
		return false;

	    for (auto & it : this-> getInners ()) {
		if (it.to<Type> ().isLocal ()) return false;
	    }
	    
	    return this-> _isLocal;
	}

	void Type::isLocal (bool is) {
	    this-> _isLocal = is;
	}
	
	const std::vector<Generator> & Type::getInners () const {
	    if (!this-> _isComplex)
		Ymir::Error::halt ("%(r) - Getting inner data of a simple type !", "Critical");
	    return this-> _inners;
	}


	void Type::setInners (const std::vector <Generator> & inner) {
	    if (this-> _isComplex) {
		this-> _inners = inner ;
	    } else
		Ymir::Error::halt ("%(r) - Getting inner data of a simple type !", "Critical");
	}
	
	Generator Type::toMutable () const {
	    if (!this-> isComplex ()) {
		Generator ret = Generator (this-> clone ());
		ret.to<Type> ().isMutable (true);
		return ret;
	    } else {
		std::vector <Generator> inners;
		for (auto & it : this-> getInners ())
		    inners.push_back (it.to <Type> ().toLevelMinusOne ());
		
		Generator ret = Generator {this-> clone ()};
		ret.to<Type> ().isMutable (true);
		ret.to <Type> ().setInners (inners);
		return ret;
	    }
	}	

	Generator Type::toLevelMinusOne () const {
	    if (this-> isMutable ()) return Generator {this-> clone ()};
	    if (!this-> isComplex ()) return this-> toMutable ();
	    else {
		std::vector<Generator> inners = this-> getInners ();
		for (auto & it : inners) 
		    it.to <Type> ().isMutable (false);
		
		Generator ret = Generator {this-> clone ()};
		ret.to <Type> ().isMutable (true);
		ret.to <Type> ().setInners (inners);
		return ret;
	    }
	}

	int Type::mutabilityLevel (int level) const {	    
	    if (this-> isMutable ()) {
		if (this-> isComplex ()) {
		    auto max = level;
		    for (auto & it : this-> getInners ()) {
			auto mut = it.to <Type> ().mutabilityLevel (level + 1);
			if (mut > max) max = mut;
		    }
		    
		    return max;
		}
		else return level + 1;
	    }

	    return level;	    
	}
	
    }
    
}
