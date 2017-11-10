#pragma once

#include "Expression.hh"
#include "ParamList.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class IAccess : public IExpression {
	
	Word end;

	ParamList params;

	Expression left;

    public:

	IAccess (Word word, Word end, Expression left, ParamList params) :
	    IExpression (word),
	    end (end),
	    params (params),
	    left (left)	    
	{
	    this-> left-> inside = this;
	    this-> params-> inside = this;
	}
	
	IAccess (Word word, Word end) :
	    IExpression (word),
	    end (end)
	{
	}

	// Expression expression () override {
	//     auto aux = new IAccess (this-> token, this-> end);
	//     aux-> params = (ParamList) this-> params-> expression ();
	//     aux-> left = this-> left-> expression ();
	//     if (is<Type> (aux-> left))
	// 	Ymir::Error::undefVar (aux-> left-> token,
	// 			       TABLE.getAlike (aux-> left-> token-> getStr ())
	// 	);

	//     else if (is <UndefInfo> (aux-> left-> info-> type))
	// 	Ymir::Error::uninitVar (aux-> left-> token);
	//     else if (aux-> left-> info-> isType ())
	// 	Ymir::Error::useAsVar (aux-> left-> token, aux-> left-> info);

	//     auto type = aux-> left-> info-> type-> AccessOp (aux-> left-> token,
	// 						     aux-> params);
	//     if (type == NULL) {
	// 	delete aux;
	// 	auto call = findOpAccess ();
	// 	if (call == NULL) {
	// 	    Ymir::Error::undefinedOp (this-> token, this-> end,
	// 				      aux-> left-> info, aux-> params);
	// 	} else {
	// 	    return call;
	// 	}
	//     }
	//     aux-> info = new ISymbol (this-> token, type);
	//     return aux;
	// }


	// Expression templateExpReplace (std::map<std::string, Expression> values)
	//     override {
	//     auto params = this-> params.templateExpReplace (values);
	//     auto left = this-> left-> templateExpReplace (values);
	//     return new IAccess (this-> token, this-> end, left, params);
	// }
	    
	Expression getLeft () {
	    return this-> left;
	}

	std::vector <Expression> getParams () {
	    return this-> params-> getParams ();
	}
	    
	void print (int nb = 0) override {
	    printf ("\n%*c<Access> %s",
		    nb, ' ',
		    this-> token.toString ().c_str ()
	    );
	    this-> left-> print (nb + 4);
	    this-> params-> print (nb + 4);
	}
	    
    protected:
	
	// Expression onClone () override {
	//     return new Access (this-> token,
	// 		       this-> end,
	// 		       this-> left-> clone (),
	// 		       (ParamList) this-> params-> clone ()
	//     );
	// }	
	
    private:
	
	// Expression findOpAccess () {
	//     try {
	// 	auto word = Word (this-> token.locus, Keys::OPACCESS.descr, true);
	// 	auto var = new IVar (word);
	// 	std::vector <Expression> params = {this-> left};
	// 	params.insert (params.begin (), this-> params-> params.begin (),
	// 		       this-> params-> params.end ());
	// 	auto params = new IParamList (this-> token, params);
	// 	auto call = IPar (this-> token, this-> token, var, params, true);
	// 	return call.expression ();
	//     } catch (YmirException tmp) {
	// 	return NULL;
	//     }
	// }
	
    };

    typedef IAccess* Access;

}
