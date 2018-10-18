#pragma once

#include <ymir/utils/memory.hh>
#include "../../syntax/Word.hh"
#include "Namespace.hh"
#include <ymir/semantic/tree/Tree.hh>
#include <ymir/semantic/pack/Namespace.hh>

namespace syntax {

    class IExpression;
    typedef IExpression* Expression;
    
}

namespace semantic {
    
    class IInfoType;
    typedef IInfoType* InfoType;

    class IValue;
    typedef IValue* Value;

    struct DeclSymbol {
	ulong lifetime = 0;
	ulong closureLifeTime = 0;
	
	static DeclSymbol init ();

    };
    
    class ISymbol  {

	bool scoped;
	bool _public;
	bool _static;
	static ulong __nbTmp__;
	Ymir::Tree decl;
	Namespace _space;
	bool _isClosured = false;
	bool _isInline = false;
	DeclSymbol _decl;
	
	InfoType _type;
	
    public:

	Word sym;
	syntax::Expression _from = NULL;
	
	ISymbol (Word word, DeclSymbol sym, syntax::Expression from, InfoType);

	ISymbol (Word word, syntax::Expression from, InfoType);
	
	bool isConst ();
	
	void isConst (bool is);

	bool& isPublic ();

	bool& isStatic ();

	bool& isClosured ();

	ulong lifeTime ();

	ulong& closureLifeTime ();

	DeclSymbol getDeclSym ();
	
	bool & isInline ();
	
	Value & value ();

	bool isLvalue ();
	
	bool isType ();

	bool isScoped ();

	bool isImmutable ();

	void quit (Namespace space);

	Ymir::Tree treeDecl ();

	Namespace & space ();
	
	InfoType type ();

	void type (InfoType type);

	void treeDecl (Ymir::Tree tree);
	
	std::string typeString ();

	std::string simpleTypeString ();

	static ulong getLastTmp ();

	static void resetNbTmp ();

	virtual ~ISymbol ();
	
    };
    
    typedef ISymbol* Symbol;
    
}
