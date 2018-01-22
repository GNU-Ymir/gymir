#pragma once

#include "Expression.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class IBinary : public IExpression {

	Expression left, right;
	
	bool isRight = false;

    public :

	IBinary (Word word, Expression left, Expression right);

	Expression expression () override;

	Expression templateExpReplace (std::map <std::string, Expression>) override;
	
	Ymir::Tree toGeneric () override;
	
	void print (int nb = 0) override;

	std::string prettyPrint () override;
	
	static const char * id () {
	    return TYPEID (IBinary);
	}
	
	std::vector <std::string> getIds () override;
	
    private:

	Expression affect ();
	Expression reaff ();
	Expression normal ();
	bool simpleVerif (IBinary*);

	Expression findOpBinary (IBinary*);
	Expression findOpAssign (IBinary*);
	Expression findOpTest (IBinary*);
	Expression findOpEqual (IBinary*);

	bool isTest (Word);
	bool isEq (Word);
	
    };

    typedef IBinary* Binary;
    
}
