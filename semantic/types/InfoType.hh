#pragma once

#include <ymir/utils/memory.hh>
//#include <ymir/errors/Error.hh>
#include <ymir/semantic/utils/OperatorUtils.hh>
#include <ymir/semantic/types/Creators.hh>
#include <vector>
#include <ymir/syntax/Word.hh>
#include <ymir/semantic/pack/Frame.hh>
#include <ymir/semantic/pack/Module.hh>
#include <vector>

namespace syntax {
    class IExpression;
    typedef IExpression* Expression;

    class IVar;
    typedef IVar* Var;
}

namespace semantic {


    // typedef Tree (InstComp*)(Tree, Tree);
    // typedef Tree (InstCompMult*)(Tree, const std::vector<Tree> &);
    // typedef Tree (InstCompS*)(Tree);
    // typedef Tree (InstPreTreatment*)(InfoType, Expression, Expression);
        
    class IInfoType;
    typedef IInfoType* InfoType;
    
    class IApplicationScore  {
    public:

	IApplicationScore ();
	IApplicationScore (Word, bool isVariadic = false);

	long score;
	Word token;
	std::string name;
	bool dyn;
	InfoType left = NULL;
	InfoType ret = NULL;
	std::vector <InfoType> treat;
	std::map <std::string, syntax::Expression> tmps;
	bool isVariadic = false;
	bool isTemplate = false;
	Frame toValidate = NULL;
	FrameProto proto = NULL;
    };

    typedef IApplicationScore* ApplicationScore;
    
    class IInfoType  {
	
	typedef Ymir::Tree (*BinopLint) (Word, IInfoType*, syntax::Expression, syntax::Expression);
	typedef Ymir::Tree (*UnopLint) (Word, IInfoType*, syntax::Expression);

	typedef Ymir::Tree (*ApplyLint) (Word, std::vector <syntax::Var>&, syntax::Block, syntax::Expression);
	
	bool _isConst = false;
	bool _isLValue = false;
	bool _isStatic = false;
	ulong _toGet;
	bool _isText = false;
	Value _value = NULL;
	
	static std::map<std::string, InfoType> __alias__;
	static bool __need__Print_CONST__;
    protected:

	bool _isType = false;
	Module _onlyInMe = NULL;

    public:

	IInfoType (bool isConst);

	Ymir::Tree buildBinaryOp (Word word, InfoType, syntax::Expression left, syntax::Expression right);

	Ymir::Tree buildCastOp (Word word, InfoType, syntax::Expression left, syntax::Expression right);
	
	Ymir::Tree buildUnaryOp (Word word, InfoType, syntax::Expression elem);

	Ymir::Tree buildMultOp (Word word, InfoType, syntax::Expression elem, syntax::Expression rights);

	Ymir::Tree buildApplyOp (Word word, std::vector <syntax::Var> & vars, syntax::Block bl, syntax::Expression iter);
	
	static InfoType factory (Word word, const std::vector<syntax::Expression> & templates);
	
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

	virtual bool isConst ();

	bool & isText ();
	
	virtual void isConst (bool isConst);
       	
	bool& isStatic ();

	virtual bool isScopable ();
	
	virtual bool isType ();
	
	virtual void isType (bool);

	virtual InfoType getIntern ();

	virtual bool allowInternalUnref ();
	
	virtual Value& value ();

	static void printConst (bool needPrintConst = true);
	
	virtual std::string typeString ();
	
	virtual std::string innerTypeString () = 0;
	
	virtual std::string simpleTypeString ();

	virtual std::string innerSimpleTypeString () = 0;
	
	virtual bool isSame (InfoType) = 0;

	virtual InfoType BinaryOp (Word, syntax::Expression);
	
	InfoType BinaryOp (Word, InfoType);

	virtual InfoType BinaryOpRight (Word, syntax::Expression);

	InfoType BinaryOpRight (Word, InfoType);

	virtual ApplicationScore CallOp (Word, syntax::ParamList);

	virtual ApplicationScore CallOp (Word, const std::vector<IInfoType*> &);

	virtual InfoType ApplyOp (const std::vector<::syntax::Var> &);

	virtual InfoType UnaryOp (Word);

	virtual InfoType AccessOp (Word, syntax::ParamList, std::vector <IInfoType*> &);

	virtual InfoType CastOp (InfoType);

	virtual InfoType CompOp (InfoType);

	virtual InfoType ConstVerif (InfoType);
	
	virtual InfoType CastTo (InfoType);

	virtual InfoType DotOp (::syntax::Var);

	virtual InfoType DotExpOp (syntax::Expression);

	virtual InfoType DColonOp (::syntax::Var);

	InfoType DotOp (std::string);

	virtual InfoType TempOp (const std::vector<::syntax::Expression> &);

	InfoType clone ();

	virtual InfoType onClone () = 0;
	
	virtual InfoType StringOf ();

	InfoType cloneOnExit ();

	InfoType cloneOnExitWithInfo ();

	InfoType cloneConst ();

	virtual bool isImmutable ();
	
	virtual Ymir::Tree toGeneric ();

	virtual Ymir::Tree getInitFnPtr ();	    
	
	virtual InfoType getTemplate (ulong);

	virtual ulong nbTemplates ();
	
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

	InfoType onlyInMod (Module mod);
	
	template <typename T>
	bool is () {
	    return this-> to<T> () != NULL;
	}
	
	template <typename T>
	T* to () {
	    auto current = this;
	    while (true) {
		if (strcmp (current-> getId (), T::id ()) == 0) {
		    return (T*) current;
		}
		
		if (current == current-> getIntern () || !current-> allowInternalUnref ()) break;
		else current = current-> getIntern ();
	    }
	    return NULL;
	}
	
	virtual ~IInfoType () {}

	BinopLint binopFoo = NULL;
	UnopLint unopFoo = NULL;
	BinopLint multFoo = NULL;
	ApplyLint applyFoo = NULL;

	std::list <BinopLint> nextBinop;
	std::list <UnopLint> nextUnop;
	std::list <BinopLint> nextMult;
	
    };    

}
