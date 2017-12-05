#pragma once

#include <gc/gc_cpp.h>
//#include <ymir/errors/Error.hh>
#include <ymir/semantic/types/Creators.hh>
#include <vector>
#include <ymir/syntax/Word.hh>
#include <ymir/semantic/pack/Frame.hh>
#include <vector>

namespace syntax {
    class IExpression;
    typedef IExpression* Expression;

    class IVar;
    typedef IVar* Var;
}

namespace semantic {


    // typedef Tree (InstComp*)(Tree, Tree);
    // typedef Tree (InstCompMult*)(Tree, std::vector <Tree>);
    // typedef Tree (InstCompS*)(Tree);
    // typedef Tree (InstPreTreatment*)(InfoType, Expression, Expression);
        
    class IInfoType;
    typedef IInfoType* InfoType;
    
    class IApplicationScore : public gc {
    public:

	IApplicationScore ();
	IApplicationScore (Word, bool isVariadic = false);

	long score;
	Word token;
	std::string name;
	bool dyn;
	InfoType left;
	InfoType ret;
	std::vector <InfoType> treat;
	std::map <std::string, syntax::Expression> tmps;
	bool isVariadic;
	bool isTemplate;
	Frame toValidate;
    };

    typedef IApplicationScore* ApplicationScore;
    
    class IInfoType : public gc {
	
	typedef Ymir::Tree (*BinopLint) (Word, syntax::Expression, syntax::Expression);
	typedef Ymir::Tree (*UnopLint) (Word, syntax::Expression);

	
	bool _isConst = false;
	bool _isStatic = false;
	ulong _toGet;

	//Value _value;
	
	static std::map<std::string, InfoType> __alias__;
	
    protected:

	bool _isType = false;
	

    public:

	IInfoType (bool isConst);

	Ymir::Tree buildBinaryOp (Word word, syntax::Expression left, syntax::Expression right);
	
	Ymir::Tree buildUnaryOp (Word word, syntax::Expression elem);

	Ymir::Tree buildMultOp (Word word, syntax::Expression elem, syntax::Expression rights);
	
	static InfoType factory (Word word, std::vector <syntax::Expression> templates);
	
	static const char* id () {
	    return "IInfoType";
	}

	virtual const char* getId () = 0;
	
	// static void addCreator (std::string name) {
	//     Creators::instance ().add (name) = StructCstInfo::create;
	// }

	static void addAlias (std::string name, InfoType alias) {
	    __alias__ [name] = alias;
	}

	static void removeAlias (std::string name) {
	    __alias__.erase (name);
	}

	static bool exists (std::string name) {
	    return (Creators::instance ().find (name) != NULL) ||
		(__alias__.find (name) != __alias__.end ());
	}

	ulong& toGet ();

	bool& isConst ();

	bool& isStatic ();

	virtual bool isScopable ();
	
	virtual bool isType ();
	
	virtual void isType (bool);

	//Value& value ();

	std::string typeString ();

	virtual std::string innerTypeString () = 0;
	
	virtual std::string simpleTypeString () = 0;

	virtual bool isSame (InfoType) = 0;

	virtual InfoType BinaryOp (Word, syntax::Expression);
	
	InfoType BinaryOp (Word, InfoType);

	virtual InfoType BinaryOpRight (Word, syntax::Expression);

	virtual ApplicationScore CallOp (Word, syntax::ParamList);

	virtual ApplicationScore CallOp (Word, std::vector <IInfoType*>);

	virtual InfoType ApplyOp (std::vector<::syntax::Var>);

	virtual InfoType UnaryOp (Word);

	virtual InfoType AccessOp (Word, syntax::ParamList);

	virtual InfoType CastOp (InfoType);

	virtual InfoType CompOp (InfoType);

	virtual InfoType ConstVerif (InfoType);

	virtual InfoType CastTo (InfoType);

	virtual InfoType DotOp (::syntax::Var);

	virtual InfoType DotExpOp (syntax::Expression);

	virtual InfoType DColonOp (::syntax::Var);

	InfoType DotOp (std::string);

	virtual InfoType TempOp (std::vector <::syntax::Expression>);

	virtual InfoType clone () = 0;

	InfoType cloneOnExit ();

	InfoType cloneConst ();

	virtual Ymir::Tree toGeneric ();
	
	virtual InfoType getTemplate (ulong);

	/**
	   Retourne les paramètre templates sur un range (pour les variadics templates).
	   Params:
	   begin = le nombre de templates avant
	   end = le nombre de templates après.
	   Example:
	   --------
	   def foo (T ...) (a : t!(int, T, int)) {
	   }

	   foo ((1, "r", 't', 7.4, 2));
	   // getTemplate (1, 1) -> [string, char, float];
	   --------
	   Returns: this.templates [begin .. end]
	*/	
	virtual std::vector <InfoType> getTemplate (ulong bef, ulong af);

	template <typename T>
	bool is () {
	    return strcmp (this-> getId (), T::id ()) == 0;
	}
	
	template <typename T>
	T* to () {
	    if (strcmp (this-> getId (), T::id ()) == 0) {
		return (T*) this;
	    } else return NULL;
	}

	BinopLint binopFoo;
	UnopLint unopFoo;
	BinopLint multFoo;
	
    };    

}
