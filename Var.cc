#include <ymir/ast/Var.hh>
#include <ymir/syntax/Keys.hh>
#include <ymir/ast/TypedVar.hh>
#include <ymir/ast/Par.hh>
#include <ymir/ast/Dot.hh>
#include <ymir/semantic/types/_.hh>
#include <ymir/semantic/pack/Table.hh>

#include <ymir/semantic/pack/Symbol.hh>
#include <ymir/semantic/types/InfoType.hh>

namespace syntax {

    using namespace semantic;
    
    IVar::IVar (Word ident) : IExpression (ident) {}

    IVar::IVar (Word ident, Word deco) :
	IExpression (ident),
	deco (deco)
    {}

    IVar::IVar (Word ident, std::vector <Expression> tmps) :
	IExpression (ident),
	templates (tmps)
    {}

    const char* IVar::id () {
	return "IVar";
    }
	
    const char* IVar::getId () {
	return IVar::id ();
    }
	
    bool IVar::hasTemplate () {
	return this-> templates.size () != 0;
    }
    
    Expression IVar::expression () {
	if (this-> info && this-> info-> isImmutable ()) {
	    return this;	    
	} else if (!this-> isType ()) {
	    auto aux = new IVar (this-> token);
	    aux-> info = Table::instance ().get (this-> token.getStr ());
	    if (aux-> info == NULL) {
		Ymir::Error::assert ("TODO, gerer l'erreur");
	    }

	    if (this-> templates.size () != 0) {
		if (!this-> inside-> is<IPar> () && !this-> inside-> is<IDot> ()) {
		    Ymir::Error::assert ("TODO");
		} else if (auto dt = this-> inside-> to<IDot> ()) {
		    Ymir::Error::assert ("TODO");
		}
		
		std::vector <Expression> tmps;
		for (auto it : this-> templates) {
		    tmps.push_back (it-> expression ());
		}

		auto type = aux-> info-> type-> TempOp (tmps);
		if (type == NULL) {
		    Ymir::Error::assert ("TODO, gerer l'erreur");
		}
		aux-> templates = tmps;
		aux-> info = new ISymbol (aux-> info-> sym, type);
	    }
	    return aux;
	} else return asType ();	
    }

    Var IVar::var () {
	if (this-> info && this-> info-> isImmutable ()) {
	    return this;	    
	} else if (!this-> isType ()) {
	    auto aux = new IVar (this-> token);
	    aux-> info = Table::instance ().get (this-> token.getStr ());
	    if (aux-> info == NULL) {
		Ymir::Error::assert ("TODO, gerer l'erreur");
	    }

	    if (this-> templates.size () != 0) {
		std::vector <Expression> tmps;
		for (auto it : this-> templates) {
		    tmps.push_back (it-> expression ());
		}

		auto type = aux-> info-> type-> TempOp (tmps);
		if (type == NULL) {
		    Ymir::Error::assert ("TODO, gerer l'erreur");
		}
		aux-> templates = tmps;
		aux-> info = new ISymbol (aux-> info-> sym, type);
	    }
	    return aux;
	} else return asType ();
    }
    
    TypedVar IVar::setType (Symbol info) {
	if (this-> deco == Keys::REF) {
	    auto type = new IType (info-> sym, info-> type-> cloneOnExit ());
	    return new ITypedVar (this-> token, type);
	} else {
	    auto type = new IType (info-> sym, info-> type-> cloneOnExit ());
	    return new ITypedVar (this-> token, type, this-> deco);
	}
    }

    TypedVar IVar::setType (InfoType info) {
	if (this-> deco == Keys::REF) {
	    auto type = new IType (this-> token, info-> cloneOnExit ());
	    return new ITypedVar (this-> token, type);
	} else {
	    auto type = new IType (this-> token, info-> cloneOnExit ());
	    return new ITypedVar (this-> token, type, this-> deco);
	}
    }

    Type IVar::asType () {
	std::vector <Expression> tmps;
	for (auto it : this-> templates)
	    tmps.push_back (it-> expression ());
	
	if (!IInfoType::exists (this-> token.getStr ())) {	    
	    Ymir::Error::assert ("TODO");
	} else {
	    auto t_info = IInfoType::factory (this-> token, tmps);
	    if (this-> deco == Keys::REF)
		t_info = new IRefInfo (t_info);
	    else if (this-> deco == Keys::CONST) t_info-> isConst () = true;
	    else t_info-> isConst () = false;
	    return new IType (this-> token, t_info);
	}
    }

    bool IVar::isType () {
	if (IInfoType::exists (this-> token.getStr ())) return true;
	else {
	    auto info = Table::instance ().get (this-> token.getStr ());
	    if (info) Ymir::Error::assert ("TODO");
	}
	return false;	
    }

    std::vector <Expression>& IVar::getTemplates () {
	return this-> templates;
    }

    Word& IVar::getDeco () {
	return this-> deco;
    }

    void IVar::print (int nb) {
	printf ("\n%*c<Var> %s",
		nb, ' ',
		this-> token.toString ().c_str ()
	);
	    
	for(auto it : this-> templates) {
	    it-> print (nb + 4);
	}
	    
    }	

    IType::IType (Word token, InfoType type) :
	IVar (token),
	_type (type)
    {}    

    InfoType IType::type () {
	return this-> _type;
    }

    const char * IType::id () {
	return "IType";
    }
    
    const char * IType::getId () {
	return IType::id ();
    }
    
    void IType::print (int nb) {
	printf ("\n%*c<Type> %s",
		nb, ' ',
		this-> token.toString ().c_str ()
	);
    }


    ITypedVar::ITypedVar (Word ident, Var type) :
	IVar (ident),
	type (type)
    {}

    ITypedVar::ITypedVar (Word ident, Var type, Word deco) :
	IVar (ident),
	type (type)
    {
	this-> deco = deco;
    }

    ITypedVar::ITypedVar (Word ident, Expression type) :
	IVar (ident),
	expType (type)
    {}

    ITypedVar::ITypedVar (Word ident, Expression type, Word deco) :
	IVar (ident),
	expType (type)
    {
	this-> deco = deco;
    }
	
    const char * ITypedVar::id () {
	return "ITypedVar";
    }

    const char * ITypedVar::getId () {
	return ITypedVar::id ();
    }

    semantic::InfoType ITypedVar::getType () {
	if (this-> type) {
	    auto type = this-> type-> asType ();
	    if (this-> deco == Keys::REF && !type-> info-> type-> is <IRefInfo> ()) {
		if (type-> info-> type-> is<IEnumInfo> ()) 
		    Ymir::Error::assert ("TODO");
		return new IRefInfo (type-> info-> type);
	    } else if (this-> deco == Keys::CONST) {
		type-> info-> type-> isConst () = true;
	    } else type-> info-> type-> isConst () = false;
	    return type-> info-> type;	    
	} else {
	    this-> expType = this-> expType-> expression ();
	    auto type = this-> expType;
	    if (this-> deco == Keys::REF && !type-> info-> type-> is <IRefInfo> ()) {
		if (type-> info-> type-> is<IEnumInfo> ()) 
		    Ymir::Error::assert ("TODO");
		return new IRefInfo (type-> info-> type);
	    } else if (this-> deco == Keys::CONST) {
		type-> info-> type-> isConst () = true;
	    } else type-> info-> type-> isConst () = false;
	    return type-> info-> type;	    
	}
    }
	
    void ITypedVar::print (int nb) {
	printf ("\n%*c<TypedVar> %s%s",
		nb, ' ',
		this-> deco.isEof () ? "" : this-> deco.getStr ().c_str (),
		this-> token.toString ().c_str ()
	);
	if (this-> type) 
	    this-> type-> print (nb + 4);
	else this-> expType-> print (nb + 4);
    }

    
}
