#pragma once

#include "Expression.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class IBinary : public IExpression {

	Expression left, right;
	Expression _autoCaster;
	
	bool isRight = false;

    public :

	IBinary (Word word, Expression left, Expression right, Expression ctype = NULL);

	Expression expression () override;

	Expression templateExpReplace (const std::map <std::string, Expression>&) override;
	
	Expression& getLeft ();

	Expression& getRight ();

	Expression& getAutoCast ();
	
	Ymir::Tree toGeneric () override;
	
	void print (int nb = 0) override;

	std::string prettyPrint () override;

	bool isLvalue () override;
	
	static const char * id () {
	    return TYPEID (IBinary);
	}
	
	std::vector <std::string> getIds () override;

	virtual ~IBinary ();
	
    private:

	Expression affect ();
	Expression reaff ();
	Expression bitwiseOr ();
	Expression normal (IBinary* = NULL);
	bool simpleVerif (IBinary*);
	bool canOverOpAssign (IBinary*);
	
	Expression findOpBinary (IBinary*);
	Expression findOpAssign (IBinary*, bool mandatory = true);
	Expression findOpTest (IBinary*);
	Expression findOpEqual (IBinary*);

	bool isTest (Word);
	std::string oppositeTest (Word);
	bool isEq (Word);
	
    };

    typedef IBinary* Binary;
    
}
