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
     * Just some basic definition of the operator used : 
     - := is for definition 
     - (x)?  means x can appear or not
     - (x)* means x can appear a arbitrary number of time from 0 to infinity
     - (x)+ means x can appear a arbitrary number of time, but at least one time
     - x | y means we can get x or y
     - x y means x and then y
     - 'token' means the token 'token' written out
     -  (x, y)! means x and y must appear, but the order is not important
     -  x |& y means x or y, or both of them, in the respected order if both appears
     -  x |! y means x or y, or both of them, in any order

     * Consuming function and not consuming function are represented in this class: 

     * A not consuming function will restore the lexer cursor position to entry point value, 
     * if the syntax does not match with expectation

     * In opposition, consuming function throw syntax errors
     */
    class Visitor {

	lexing::Lexer _lex;

	std::vector <std::string> _forbiddenKeys;	
	
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
	 * \param isPrivate the returned block will be private ?
	 * \verbatim
	 protection := ('private' | 'public') declaration_block
	 declaration_block := declaration | '{' declaration* '}'
	 \endverbatim
	 */
	Declaration visitProtectionBlock (bool isPrivate);

	/**
	 * \brief Visit a version block
	 * \param global This boolean is set to true if we are in a block where we can define new version words 
	 * \verbatim
	 version_glob ? (global) := ('=' Identifier ';') | version_glob ? (!global)
	 version_glob ? (!global) := Identifier declaration_block ('else' declaration_block)?
	 \endverbatim
	 */
	Declaration visitVersionGlob (bool global);

	/**
	 * \brief Used to factorize function visitVersionGlob (bool)
	 * \brief Called when !global, or equivalent
	 * \verbatim
	 version_block := version_glob ? (!global)
	 \endverbatim
	 */
	Declaration visitVersionGlobBlock ();
	
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
			use      |
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
	 */
	Expression visitSingleVarDeclaration ();	

	/**
	 * \brief Visit an single expression 
	 */
	Expression visitExpression ();
	       	
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
	
    };
    
}
