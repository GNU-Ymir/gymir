#pragma once

#include "Frame.hh"
#include "FrameProto.hh"
#include <ymir/ast/Function.hh>
#include <ymir/ast/ParamList.hh>
#include <ymir/ast/OfVar.hh>
#include <ymir/ast/ArrayAlloc.hh>
#include <ymir/ast/FuncPtr.hh>
#include <map>

namespace semantic {

    struct TemplateSolution {
	long score;
	bool valid;
	InfoType type;
	bool isVariadic = false;
	std::map <std::string, syntax::Expression> elements;
	std::vector <InfoType> varTypes;

	TemplateSolution (long score, bool valid) :
	    score (score), valid (valid), type (NULL)
	{}

	TemplateSolution (long score, bool valid, InfoType type) :
	    score (score), valid (valid), type (type)
	{}

	TemplateSolution (long score, bool valid, InfoType type, std::map <std::string, syntax::Expression>& elemts) :
	    score (score), valid (valid), type (type)
	{
	    this-> elements.swap (elemts);
	}

	std::string toString ();
	
    };

    class TemplateSolver {

	ulong __VAR__ = 1;
	ulong __ARRAY__ = 2;
	ulong __FUN__ = 1;
	static TemplateSolver __instance__;
	
    public:

	static TemplateSolver& instance ();
	
	bool merge (long& score, std::map <std::string, syntax::Expression>& left, TemplateSolution& right);

	bool merge (long& score, std::map <std::string, syntax::Expression>& left, std::map <std::string, syntax::Expression>& right);

	/**
	   Résolution du type d'un paramètre, 
	   Example:
	   -----------
	   def foo (T of [U], U) (a : T) {
	   }
	   foo ([2, 3]); // solve ([T, U], a, [int]);
	   -----------
	   Params:
	   tmps = la liste des paramètre templates de l'élément
	   param = un paramètre dont il font résoudre le type
	   type = le type du paramètre passé 
	   Returns: le tableau associatif des nouvelles expression
	*/
	TemplateSolution solve (const std::vector <syntax::Expression> &tmps, syntax::Var param, InfoType type);

	/**
	   Résolution du type d'un paramètre, 
	   Params:
	   tmps = la liste des paramètre templates de l'élément
	   param = un paramètre dont il font résoudre le type
	   type = le type du paramètre passé 
	   Returns: le tableau associatif des nouvelles expression
	*/
	TemplateSolution solveInside (const std::vector <syntax::Expression> &tmps, syntax::Var param, InfoType type);

	
	/**
	   Résoud un paramètre template de type variadic
	   Params:
	   tmps = la liste des templates de l'élement que l'on est en train de résoudre
	   param = l'expression type de l'attribut
	   type = les types interne du paramètre.
	   Returns: une solution template
	*/
	TemplateSolution solveInside (const std::vector <syntax::Expression> &tmps, syntax::Var var, const std::vector <InfoType> &type);

	/**
	   Résoud un paramètre template de type tableau
	   Example:
	   --------
	   def foo (T) (a : [T]) {
	   }
	   
	   foo ([10]); // solve ([T], [T], [int]);
	   --------
	   Params:
	   tmps = les paramètres templates de la fonction
	   param = l'expression type de l'attribut
	   type = Le type du paramètre
	   Returns: une solution template
	*/
	TemplateSolution solve (const std::vector <syntax::Expression> &tmps, syntax::ArrayVar param, InfoType type, bool isConst = false);

	
	TemplateSolution solve (const std::vector <syntax::Expression> &tmps, syntax::ArrayAlloc param, InfoType type, bool isConst = false);

	/**
	   Résoud un paramètre template de type fonction
	   Example:
	   -------
	   def foo (T) (a : fn (T) -> T) {         
	   }

	   foo ((a : int) => 10); // solve ([T], fn (T) -> T, funcPtr(int)->int);
	   -------
	*/
	TemplateSolution solve (const std::vector <syntax::Expression> &tmps, syntax::Expression param, InfoType type);	
	/**
	   Résoud un paramètre template de type fonction
	   Example:
	   -------
	   def foo (T) (a : fn (T) -> T) {}
	   
	   foo ((a : int) => 10); // solve ([T], fn (T) -> T, funcPtr (int) -> int);
	   -------
	*/
	TemplateSolution solveInside (const std::vector <syntax::Expression> &tmps, syntax::FuncPtr ptr, InfoType type);
	
	/**
	   résoud un paramètre template
	   Params;
	   elem = le paramètre templates
	   param = le paramètre passé à la fonction
	   type = le type du paramètre passé 
	   Returns: le tableau associatif des nouvelles expressions
	*/
	TemplateSolution solve (syntax::Var elem, syntax::Var param, InfoType type, bool isConst = false);

	/**
	   Résoud un paramètre template
	   Params;
	   elem = le paramètre templates
	   param = le paramètre passé à la fonction
	   type = le type du paramètre passé 
	   Returns: le tableau associatif des nouvelles expressions
	*/
	TemplateSolution solve (syntax::Var elem, syntax::TypedVar param, InfoType type, bool isConst = false);

	/**
	   Résoud un paramètre templates de type of
	   Example:
	   -------
	   def foo (A of int) (a : A) {
	   }
	   foo (10); // solve (A of int, a : A, int);
	   -------
	   Params:
	   elem = le paramètre template
	   param = l'attribut de la fonction
	   type = le type du paramètre passé à la fonction
	   Returns: une solution template
	*/
	TemplateSolution solve (const std::vector <syntax::Expression> &tmps, syntax::OfVar elem, syntax::Var param, InfoType type, bool isConst = false);

	/**
	   résolution des paramètre templates.
	   Example:
	   --------
	   def foo (T of [U], U) () {
	   }
     
	   foo!([int]) (); // solve ([T, U], [[int]]);
	   --------
	   Params:
	   tmps = la liste des paramètre templates de l'élément
	   params = la liste des paramètre de opTemp
	   Returns: le tableau associatif des nouvelles expressions
	*/    
	TemplateSolution solve (const std::vector <syntax::Expression> &tmps, const std::vector <syntax::Expression> &params);

	/**
	   Résoud un paramètre template
	   Example:
	   ------
	   def foo (T) () {}
	   foo!(int) ();

	   def foo (a : string) () {
	   }
	   foo!"salut" ();
	   ------

	   Params:
	   tmps = les paramètre templates de l'élément
	   left = le paramètre template courant
	   right = le paramètre template passé à la fonction
	   Returns: une solution de résolution template
	*/
	TemplateSolution solveInside (const std::vector <syntax::Expression> &tmps, syntax::Var left, syntax::Expression right);


	/**
	   Résoud un paramètre template
	   Example:
	   -------     
	   def foo (T) () {}
	   foo!int (); //solveInside (T, int);
	   -------
	*/;
	TemplateSolution solveInside (syntax::Var left, syntax::Type right) ;

	/**
	 * Résoud  un paramètre template
	 * Example:
	 * ------------------
	 * struct Test { a : int }
	 * 
	 * def foo (T) () {
	 *    // ...
	 * }
	 *
	 * foo!(Test); // solveInside (T, Test);
	 * ------------------
	 */
	TemplateSolution solveInside (syntax::Var left, syntax::Var right);

	/**
	 * Résoud  un paramètre template
	 * Example:
	 * ------------------
	 * 
	 * def foo (T) () {
	 *    // ...
	 * }
	 *
	 * foo!(fn (int)-> int); // solveInside (T, Test);
	 * ------------------
	 */
	TemplateSolution solveInside (syntax::Var left, syntax::FuncPtr right);

	/**
	   Résoud un paramètre template
	   Example:
	   --------
	   def foo (T of int) () {
	   }
	   foo!int (); //solveInside (T of int, int);
	   --------
	*/
	TemplateSolution solveInside (const std::vector <syntax::Expression> &tmps, syntax::OfVar left, syntax::Expression right) ;
	    

	// /**
	//    Résoud un paramètre template.
	//    Example:
	//    -----------
	//    def foo (T...) (a : t!(T)) {
	//    }
	//    foo ((1, 2, 3)); // solveInside (T..., tuple(int, int, int));
	//    -----------     
	// */
	TemplateSolution solveInside (const std::vector<syntax::Expression>  & tmps, syntax::VariadicVar var, syntax::Expression right);

	/**
	   Résoud un paramètre template
	   Example:
	   -------     
	   def foo (a : string) () {
	   }
	   foo!"salut" ();     
	   -------
	*/
	TemplateSolution solveInside (const std::vector <syntax::Expression> &tmps, syntax::TypedVar left, syntax::Expression right);

	/**
	   Résoud un paramètre template
	   Example:
	   --------
	   def foo ("salut") () {
	   }

	   foo!("salut") ();
	   --------
	*/
	TemplateSolution solveInside (const std::vector <syntax::Expression> &tmps, syntax::Expression left, syntax::Expression right);

	/**
	   Résolution des paramètre templates.
	   Example:
	   --------
	   struct (T of [U], U) 
	   | a : T
	   -> Test;
     
	   let a = Test!([int])(); // solve ([T, U], [[int]]);
	   --------
	   Params:
	   tmps = la liste des paramètre templates de l'élément
	   params = la liste des paramètre de opTemp
	   Returns: le tableau associatif des nouvelles expressions
	*/    
	TemplateSolution solve (const std::vector <syntax::Var> &tmps, const std::vector <syntax::Expression> &params) ;

	TemplateSolution solveVariadic (const std::vector <syntax::Expression> tmps, syntax::Expression last, std::vector <syntax::Expression> params);

	TemplateSolution solveVariadic (const std::vector <syntax::Expression> tmps, syntax::Expression last, const std::vector <InfoType>& params);

	TemplateSolution solveVariadic (const std::vector <syntax::Expression> & tmps, const std::vector <syntax::Expression> & params);
	
	/**
	   Vérifie que les paramètres templates sont valide
	   Params:
	   args = les paramètre templates
	*/
	bool isValid (const std::vector <syntax::Expression> &args);

	/**
	   Params:
	   args = les paramètres templates de l'élément 
	   soluce = la résolution
	   Returns: Les paramètres templates ont été correctement typé, et le résultat est une solution valide ?
	*/
	bool isSolved (const std::vector <syntax::Expression> &args, TemplateSolution soluce);

	/**
	   Params:
	   args = les paramètres templates de l'élément 
	   types = la résolution
	   Returns: Les paramètres templates ont été correctement typé, et le résultat est une solution valide ?
	*/
	bool isSolved (const std::vector <syntax::Expression> &args, std::map <std::string, syntax::Expression> &types);

	
	/**
	   Example:
	   -------
	   def foo (A, B) () {
	   }
	   foo!int; // unSolved () == [B]
	   -------
	   Params:
	   args = une liste de paramètre template
	   soluce = une solution de résolution templates
	   Returns: la liste des paramètre qui n'apparaissent pas résolue dans la solution
	*/
	std::vector <syntax::Expression> unSolved (const std::vector <syntax::Expression> &args, TemplateSolution soluce);

		
    private:

	TemplateSolver ();
	TemplateSolver (const TemplateSolver& other);
	TemplateSolver operator= (const TemplateSolver& other);
	
    };
    
}
