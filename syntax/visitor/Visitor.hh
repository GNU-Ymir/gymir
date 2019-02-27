#pragma once

#include <ymir/syntax/Declaration.hh>
#include <ymir/syntax/Expression.hh>
#include <ymir/syntax/declaration/Function.hh>

#include <ymir/lexing/Lexer.hh>
#include <ymir/lexing/Word.hh>

namespace syntax {

    /**
     * This class is used to parse a lexer and create a abstract syntax tree of a given source code
     * In the following documentation, is simple syntax is used to represent the syntax recognize by each function
     * A more accomplished work is required to use this element as the effective grammar of the language
     * But this simple version gives us a overview of the grammar
     * A rule beginning with a upper case character, is a final one, it depends on no other rules
     * Just some basic definition of the operator used : 
     - \c := is for definition 
     - \c (x)?  means x can appear or not
     - \c (x)* means x can appear a arbitrary number of time from 0 to infinity
     - \c (x)+ means x can appear a arbitrary number of time, but at least one time
     - <tt>x | y</tt> means we can get x or y
     - <tt>x y</tt> means x and then y
     - <tt>'token'</tt> means the token 'token' written out
     -  <tt>(x, y)!</tt> means x and y must appear, but the order is not important
     -  <tt>x |& y</tt> means x or y, or both of them, in the respected order if both appears
     -  <tt>x |! y</tt> means x or y, or both of them, in any order
     -  <tt>[a-Z]</tt> means any alphabetic char, can be used with anything between the <tt>[]</tt>

     * Consuming function and not consuming function are represented in this class: 

     * A not consuming function will restore the lexer cursor position to entry point value, 
     * if the syntax does not match with expectation

     * In opposition, consuming function throw syntax errors
     */
    class Visitor {

	/** The lexer used to perform the lexical analyses */
	lexing::Lexer _lex;

	/** All the keys that cannot be indentifier */
	std::vector <std::string> _forbiddenKeys;	

	/** the binary operators sorted by priority (0 to 10) */
	std::vector <std::vector <std::string> > _operators;

	/** the special binary operators (!of, !is ...) sorted by priority (0 to 10) */
	std::vector <std::vector <std::string> > _specialOperators;

	/** The unary operators that can be applied directly to operand */
	std::vector <std::string> _operand_op;

	/** The suffix int */
	std::vector <std::string> _fixedSuffixes;

	/** The suffix float */
	std::vector <std::string> _floatSuffix;

	/** The suffix char */
	std::vector <std::string> _charSuffix;

	/** The list of instricts */
	std::vector <std::string> _intrisics;

	/** The list of declarable things 
	 * (for convinience, just adding something in this list won't result something usefull) 
	*/
	std::vector <std::string> _declarations;

	/**
	 * The list of declarable things inside a block
	 * It is a subset of _declarations
	 */
	std::vector <std::string> _declarationsBlock;
	
    private :

	Visitor ();

    public :

	/**
	 * \brief Create a new visitor from a lexer
	 * \param path the path to the filename
	 * \param file the file already opened
	 */
	static Visitor init (const std::string & path, FILE * file);
	
	/**
	 * \brief Create a new visitor from a lexer
	 * \param lexer the lexical analyser 
	 */
	static Visitor init (const lexing::Lexer & lexer);

	/** 
	 * \brief Visit a global module (entire file)
	 * \verbatim
	 global_module := (mod_space)? (version | private | public | declaration)*	
	 mod_space := 'mod' namespace ';'
	 \endverbatim
	 */
	Declaration visitModGlobal ();      	

	/**
	 * \brief Visit a private or public block
	 * \verbatim
	 protection := ('private' | 'public') declaration_block
	 declaration_block := declaration | '{' declaration* '}'
	 \endverbatim
	 * \param isPrivate the returned block will be private ?
	 */
	Declaration visitProtectionBlock (bool isPrivate);

	/**
	 * \brief Visit a version block
	 * \verbatim
	 version_glob:(global) := ('=' Identifier ';') | version_glob ? (!global)
	 version_glob:(!global) := Identifier declaration_block ('else' declaration_block)?
	 \endverbatim
	 * \param global This boolean is set to true if we are in a block where we can define new version words 
	 */
	Declaration visitVersionGlob (bool global);

	/**
	 * \brief Used to factorize function visitVersionGlob (bool)
	 * \brief Called when !global, or equivalent
	 * \verbatim
	 version_block := version_glob:(!global)
	 \endverbatim
	 */
	Declaration visitVersionGlobBlock (bool global);
	
	/**
	 * \brief Visit an extern element
	 * \verbatim
	 extern := 'extern' ('(' C | C++ | D | Y (',' space)? ')' )? declaration_block
	 \endverbatim
	 */
	Declaration visitExtern ();

	/**
	 * \brief Visit a single declaration
	 * \verbatim
	 declaration := alias    |
	                class    | 
	                enum     |
                        function |
                        global   |
                        import   |
                        macro    |
                        struct   |
                        trait    |
                        use     
	 \endverbatim
	 */
	Declaration visitDeclaration ();

	/**
	 * \brief Visit an alias
	 alias := 'alias' Identifier (template)? '=' expression ';'
	 */
	Declaration visitAlias ();


	/**
	 * \brief Visit a class definition
	 * \verbatim
	 class := 'type' Identifier (template)? ('over' expression)? '{' class_content* '}'
	 \endverbatim
	 */
	Declaration visitClass ();	

	/**
	 * \brief Return a class content
	 * \verbatim
	 class_block := class_content*
	 \endverbatim
	 */
	std::vector<Declaration> visitClassBlock ();
	
	/**
	 * \brief Return a class content
	 * \verbatim
	 class_content := constructor       |
	                  destructor        |
			  function          |
			  mixin             |
			  var_decl          |
			  protect_content   
	 \endverbatim
	 */
	Declaration visitClassContent ();


	/**
	 * \brief Visit a class Constructor 
	 * \verbatim
	 class_constructor := 'self' function_proto function_body
	 \endverbatim
	 */
	Declaration visitClassConstructor ();
	
	/**
	 * \brief Visit a class destructor
	 * \verbatim
	 class_destructor := '~' 'self' '(' ')' expression
	 \endverbatim
	 */
	Declaration visitClassDestructor ();

	/**
	 * \brief Inner class version control
	 */
	Declaration visitVersionClass ();
	
	/**
	 * \brief Return a class protected content (DeclBlock)
	 * \verbatim
	 protect_content := ('private' | 'protected') (('{' class_content* '}') | class_content)
	 \endverbatim
	 */
	Declaration visitProtectionClassBlock (bool isPrivate);


	/**
	 * \brief Visit a mixin usage
	 * \verbatim
	 mixin := 'mixin' operand:(0) (';')?
	 \endverbatim
	 */
	Declaration visitClassMixin ();

	/**
	 * \brief Visit a enumeration declaration
	 * \verbatim
	 enum := 'enum' (parameters)+ '->' Identifier (templates)?
	 parameters := '|' Identifier '=' expression
	 \endverbatim	 
	 */
	Declaration visitEnum ();
	
	/**
	 * \brief Visit a function declaration 
	 * \verbatim
	 function := 'def' ('if' expression)? (attributes)? Identifier (templates)? function_proto function_body
	 \endverbatim
	 */
	Declaration visitFunction ();
	
	/**
	 * \brief Visit a function prototype
	 * \verbatim
	 function_proto := '(' var_decl* ')' ('->' expression)?
	 \endverbatim
	 */
	Function::Prototype visitFunctionPrototype ();

	/**
	 * \brief Visit a function body
	 * \verbatim
	 function_body := ('in' expression)? ('out' '(' Identifier ')' expression)? expression
	 \endverbatim
	 */
	Function::Body visitFunctionBody ();
	
	/**
	 * \brief Visit a set of template parameter, used in declaration
	 * \brief This function is not a cosuming one
	 * \verbatim
	 params := '(' template_element+ ')'
	 template_element := Identifier                 |
	                     Identifier ':' expression  |
	                     Identifier 'of' expression | 
			     Identifier (':')? '...'    |
			     expression
	 \endverbatim
	 */
	std::vector <Expression> visitTemplateParameters ();


	/**
	 * \brief Visit a global var declaration 
	 * \verbatim
	 global := 'static' var_decl (';')?
	 \endverbatim
	 */
	Declaration visitGlobal ();


	/**
	 * \brief Visit a importation declaration
	 * \verbatim
	 import := 'import' import_name (',' import_name)* (';')?
	 import_name := space ('as' Identifier)?
	 \endverbatim
	 */
	Declaration visitImport ();


	/**
	 * \brief Visit a local module (inside another module file)
	 * \verbatim
	 module := 'mod' Identifier (templates)? '{' (version | private | public | declaration)* '}'
	 \endverbatim
	 */
	Declaration visitLocalMod ();


	/**
	 * \brief Visit a struct declaration 
	 * \verbatim
	 struct := 'struct' ('|' var_decl)* '->' name (templates)?
	 \endverbatim
	 */
	Declaration visitStruct ();


	/**
	 * \brief Visit a trait declaration 
	 * \verbatim
	 trait := ('mixin' | 'trait') Identifier (templates)? '{' class_content '}'
	 \endverbatim
	 */
	Declaration visitTrait ();


	/**
	 * \brief Visit a use declaration
	 * \verbatim
	 use := 'use' expression (';')?
	 \endverbatim
	 */
	Declaration visitUse ();
	
	/**
	 * \brief Return a set of expression used inside a param list
	 * \verbatim
	 param_list := (param (',' param)*)?
	 param := expression:(0) | '?' Identifier '=' expression:(0)
	 \endverbatim
	 * \param withNamed set to true if we can have named expression 
	 */
	std::vector <Expression> visitParamList (bool withNamed = false);	
	
	/**
	 * \brief Visit the Custom attributes associated to declarations
	 * This function is not a consuming one
	 * \verbatim
	 attributes := '@' (('{' Identifier (',' Identifier)* '}') | Identifier)
	 \endverbatim
	 */
	std::vector <lexing::Word> visitAttributes ();
	
	/**
	 * Visit a var declaration, 
	 * if multiple variable are declared in the same line, 
	 * return a ExpressionList of VarDecl, and just a single VarDecl otherwise
	 * \verbatim
	 var_decl_set := 'let' var_decl (',' var_decl)* ';'
	 \endverbatim
	 */
	Expression visitVarDeclaration ();

	/**
	 * \brief Visit a single var declaration
	 * \verbatim
	 var_decl := (('ref')?, ('const')?, ('cte')?, ('static')?, ('mut')?)! Identifier (':' expression)? ('=' expression)?
	 \endverbatim
	 * \param withValue is true if we can have an affectation ('=' expression)?
	 */
	Expression visitSingleVarDeclaration (bool mandType = false, bool withValue = true);	


	/**
	 * \return true iif, a var declaration is following
	 * \param mandatoryType is the type mandatory in this declaration ?
	 * \param withValue is the value present in this declaration ?
	 */
	bool canVisitSingleVarDeclaration (bool mandatoryType, bool withValue);
	
	/**
	 * \brief Visit an single expression 
	 * \verbatim
	 expression:(priority) := (expression:(priority + 1) operator:(priority) expression:(priority + 1)) (operator:(priority) expression:(priority))?
	 expression:(10) := operand:(0)
	 
	 operator:(0)  := '=' | '-=' | '/=' | '*=' | '+='
	 operator:(1)  := '||'
	 operator:(2)  := '&&'
	 operator:(3)  := '<' | '>' | '<=' | '>=' | '!=' | '==' | (('!')? ('of' | 'is' | 'in'))
	 operator:(4)  := '...' | '..' 
	 operator:(5)  := '<<' | '>>'
	 operator:(6)  := '|' | '^' | '&'
	 operator:(7)  := '+' | '~' | '-'
	 operator:(8)  := '*' | '%' | '/'
	 operator:(9)  := '^^'
	 \endverbatim
	 * \param priority, the priority of the expression (used to get the set of operators usable at this instant)	 
	 */
	Expression visitExpression (uint priority = 0);


	/**
	 * \brief Used to simplificate the writting of visitExpression 
	 * \param left the left operand (already read)
	 * \param the priority of the operation
	 */
	Expression visitExpression (const Expression & left, uint priority = 0);
	
	/**
	 * \brief Visit an operand (which has higher priority than every expression of visitExpression)
	 * \brief Some of the operand, may sound weird (like let, or break), if they are syntaxically correct, at semantic time use them as value will create a failure
	 * \brief I think at the moment when i write this documentation, (no type system implemented yet) that it will be easier this way (after a previous version of the compiler, where there was a syntaxic dinstinction between instruction and expression)
	 * To better understand what it implies, the following code is syntaxically correct but does not mean anything :
	 * \verbatim
	 let a = 12 + let b = 89
	 \endverbatim
	 * \verbatim
	 operand:(0) := (operand_op_bef)? operand:(1)
	 operand:(1) := operand:(2) (mult_next)?
	 operand_op_bef := '&' | '*' | '-'
	 mult_op :=  '(' expression:(0) (',' expression:(0))* ')' | 
	             '[' expression:(0) (',' expression:(0))* ']' 

	 operand:(2) := literal      | 
	                array        |
	                tuple        |
			lambda       |
			block        |
			if           |
			while        |
			for          |
			match        |
			var_decl_set |
			break        |
			assert       |
			throw        |
			pragma       |
			scope        |
			version      | 
			return 			
	 
	 \endverbatim
	 */
	Expression visitOperand0 ();

	/**
	 * \brief Used to factorize visitOperand
	 */
	Expression visitOperand1 ();	

	/**
	 * \brief Used to factorize visitOperand
	 */
	Expression visitOperand2 ();

	/**
	 * \brief Used to factorize visitOperand
	 */
	Expression visitOperand2 (const Expression & value);
	
	/**
	 * \brief Used to factorize visitOperand
	 */
	Expression visitOperand3 ();
	
	/**
	 * \brief Visit a literal expression 
	 * \verbatim
	 literal := Int | float | String | Char | Bool | '_'
	 Int := [0-9]([0-9] | '_')*
	 float := Int '.' (Int)? | (Int)? '.' Int
	 String := '"' [.]* '"'
	 Char := '\'' [.] '\''
	 Bool := 'true' | 'false'
	 \endverbatim
	 */
	Expression visitLiteral ();


	/**
	 * \brief Visit a numeric literal
	 */
	Expression visitNumeric ();

	/**
	 * \brief Verif that content is well formed for a numeric value
	 * \brief Throw syntax error on loc if not 
	 * \param loc the location of the literal
	 * \param content the content to check
	 * \return the numeric is in a hexadecimal form
	 */
	bool verifNumeric (const lexing::Word & loc, const std::string& content);

	/**
	 * \brief Visit a float literal
	 * \brief Read only the decimal part
	 * \param begin is the beginning part of the float
	 */
	Expression visitFloat (const lexing::Word & begin);

	/**
	 * \brief Visit a char literal 
	 * \brief Does not verify the integrity of the content, it will be checked at semantic time
	 */
	Expression visitChar ();
	
	/**
	 * \brief Visit an array literal, it can be either a real literal or an array allocator
	 * \verbatim
	 array := allocator | array_literal 
	 allocator := '[' expression:(0) ';' expression:(0) ']'
	 array_literal := '[' (expression:(0) (',' expression:(0))*)? ']'
	 \endverbatim
	 */
	Expression visitArray ();

	/**
	 * \brief Visit a tuple literal, or an simple expression surrounded with '(' ')'
	 * \verbatim
	 tuple := '(' (expression:(0) ((',' expression:(0))* | ','))? ')'
	 \endverbatim
	 */
	Expression visitTuple ();

	/**
	 * \brief Visit a lambda function definition 
	 * \verbatim
	 lambda := ('ref')? function_proto '=>' expression 
	 \endverbatim
	 */
	Expression visitLambda ();
	
	/**
	 * \brief Visit a if expression
	 * \verbatim
	 if := 'if' expression:(0) expression:(0) ('else' if)?
	 \endverbatim
	 */
	Expression visitIf ();


	/**
	 * \brief Visit a while loop expression 
	 * \verbatim
	 while := 'while' expression:(0) expression:(0)
	 \endverbatim
	 */
	Expression visitWhile ();

	/**
	 * \brief Visit a do while loop expression
	 * \verbatim
	 do_while := 'do' expression:(0) 'while' expression:(0)
	 */
	Expression visitDoWhile ();
	
	/**
	 * \brief Visit a for loop
	 * \verbatim
	 for := 'for' var_decl (',' var_decl)* 'in' expression:(10) expression:(0)
	 \endverbatim
	 */
	Expression visitFor ();

	/**
	 * \brief Visit a match expression 
	 * \verbatim
	 match := 'match' expression:(0) '{' (match_pattern '=>' expression:(0))+ '}'
	 match_pattern := match_pattern_content ('if' expression:(0))?
	 match_pattern_content := expression:(0)                                |
	                          var_decl                                      |
				  expression:(10) '{' match_pattern_content* '}' |
				  '(' match_pattern_content* ')'                |
	 \endverbatim
	 */
	Expression visitMatch ();


	/**
	 * \brief Visit a break
	 * \verbatim
	 break := 'break' (expression:(10))?
	 \endverbatim
	 */
	Expression visitBreak ();

	/**
	 * \brief Visit an assertion 
	 * \verbatim
	 assert := ('cte')? 'assert' expression:(0) ('=>' expression:(10))
	 \endverbatim
	 */
	Expression visitAssert ();

	
	/**
	 * \brief Visit a throw
	 * \verbatim
	 throw := 'throw' expression:(10)
	 \endverbatim
	 */
	Expression visitThrow ();

	/**
	 * \brief Visit a pragma
	 * \verbatim
	 pargma := '__pragma' '(' param_set ')'
	 \endverbatim
	 */
	Expression visitPragma ();

	/**
	 * \brief Visit a scope guard
	 * \verbatim
	 scope := 'on' ('exit' | 'failure' | 'success') '=>' expression:(0) |
	          'on' 'failure' '=>' '{' (var_decl '=>' expression:(0))+ '}'	
	 \endverbatim
	 */
	Expression visitScope ();

	/**
	 * \brief Visit a version manager
	 * \verbatim
	 version := 'version' Identifier expression:(0) ('else' expression:(0))?
	 \endverbatim
	 */
	Expression visitVersion ();

	/**
	 * \brief Visit a return expression
	 * \verbatim
	 return := 'return' expression:(0) 
	 \endverbatim
	 */
	Expression visitReturn ();
	
	/**
	 * \brief Visit a block of expression 
	 * \verbatim
	 block := '{' (expression:(0) | ';')* '}'
	 \endverbatim
	 */
	Expression visitBlock ();


	/**
	 * \brief Visit a var declaration
	 * \verbatim
	 var := Identifier (('!' expression:(0)) | '!' '(' expression:(0) (',' expression:(0))* ')')?
	 \endverbatim
	 */
	Expression visitVar ();

	/**
	 * \brief Visit a decorated expression 
	 * \verbatim 
	 deco_expr := ('ref' | 'const' | 'copy' | 'cte' | 'mut')+ expression:(9)
	 */
	Expression visitDecoratedExpression ();
	
	
	/**
	 * \brief Visit a namespace set 
	 * \verbatim
	 space := '_' | (Identifier ('::' Identifier)* ('::' '_')?)
	 \endverbatim
	 */
	lexing::Word visitNamespace ();


	/**
	 * \brief Visit a indentifier
	 * \verbatim
	 Identifier := ('_')* ([a-z]|[A-Z]) ([a-z]|[A-Z]|'_'|[0-9])*
	 \endverbatim
	 */
	lexing::Word visitIdentifier ();


	/**
	 * \return true if the next token is an identifier 
	 */
	bool canVisitIdentifier ();

	/**
	 * \brief Return true if function does not throw anything if calling
	 * \brief Reset the cursor location of the file to entry point in all cases
	 */
	bool can (Expression (Visitor::*func)());


	/**
	 * \brief Ignore a block of content (skip all token between '{' and '}')	 
	 */
	void ignoreBlock ();
	
    };


    
}
