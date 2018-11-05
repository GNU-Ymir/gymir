#pragma once

#include "Declaration.hh"
#include "Var.hh"
#include "Expression.hh"
#include "Block.hh"
#include <vector>
#include "../syntax/Word.hh"
#include <ymir/utils/Array.hh>
#include <ymir/semantic/pack/Frame.hh>
#include <ymir/semantic/pack/Namespace.hh>

namespace syntax {

    class IProto;
    typedef IProto* Proto;
    
    /**
     * \struct IFunction
     * The syntaxic node representation of a function 
     * \verbatim 
     function := 'def' ('@' attrs)? Identifier (if_expr)? ('(' templates ')')? '(' (params)? ')' ('->' type)? (block | contract) 
     attrs := ('{' Identifier (',' Identifier)* '}') | Identifier 
     if_expr := 'if' expression 
     templates := temp_elem (',' temp_elem)*
     temp_elem := constante | Identifier 'of' expression | Identifier ':' expression | Identifier 
     params := var_decl (',' var_decl)*
     var_decl := Identifier (':' expression)?
     contract := '{' 'pre' block 'body' block 'post' '(' Identifier ')' block '}'
     \endverbatim 
     */
    class IFunction : public IDeclaration {
    protected :

	/** The function identifier */
	Word _ident;

	/** The return type of the function, may be NULL */
	Expression _type;

	/** The decoration of the return type (ref, mut, const) */
	Word _retDeco;

	/** The params of the function */
	std::vector <Var> _params;

	/** The templates of the function */
	std::vector <Expression> _tmps;

	/** The attributs of the function */
	std::vector <Word> _attrs;

	/** The body of the function */
	Block _block;

	/** The test of the if, may be NULL */
	Expression _test;

	/** The pre block, may be NULL*/
	Block _pre = NULL;

	/** The post block, may be NULL */
	Block _post = NULL;

	/** The var assigned to return value at post block time, may be NULL */
	Var _postVar = NULL;

	/** External language function in case of external declaration (could be Keys::CLANG, Keys::CPPLANG or Keys::DLANG) */
	std::string _externLang = "";

	/** The namespace of the external function declaration */
	std::string _externLangSpace = "";

	/** The associated frame (semantic time) may be NULL*/
	semantic::Frame _frame;
	
    public:
	
	/**
	 * \param ident the identifier and the location of the definition
	 * \param docs the comments related to the function
	 * \param attrs the attriutes of the function
	 * \param params the parameters
	 * \param tmps the templates
	 * \params test the test
	 * \params block the body
	 */
	IFunction (Word ident, const std::string& docs, const std::vector <Word>& attrs, const std::vector<Var> & params, const std::vector <Expression>& tmps, Expression test, Block block);

	/**
	 * \param ident the identifier and the location of the definition
	 * \param docs the comments related to the function
	 * \param attrs the attriutes of the function
	 * \param type the type of the function
	 * \param retDeco the decoration of the return type
	 * \param params the parameters
	 * \param tmps the templates
	 * \params test the test
	 * \params block the body
	 */
	IFunction (Word ident, const std::string & docs, const std::vector <Word>& attrs, Expression type, Word retDeco, const std::vector<Var> & params, const std::vector <Expression>& tmps, Expression test, Block block);

	/**
	 * \return the identifier of the function
	 */
	Word getIdent ();	

	/**
	 * \return the name of the function
	 */
	const std::string& getName ();

	/**
	 * \brief set the name of the function
	 */
	void setName (std::string &);

	/**
	 * \brief set the name of the function
	 */
	void setName (const char*);

	Ymir::json generateDocs () override;

	/**
	 * \return get or set the parameters of the function
	 */
	std::vector <Var>& params ();
	
	/**
	 * \return the parameters of the function
	 */
	const std::vector <Var>& getParams ();

	/**
	 * \return get or set the templates of the function
	 */
	std::vector <Expression>& templates ();
	
	/**
	 * \return the templates of the function
	 */
	const std::vector <Expression>& getTemplates ();

	/**
	 * \return all the variable declaration done inside the function
	 */
	std::vector <semantic::Symbol> allInnerDecls ();

	/**
	 * \return the attributes of the function
	 */
	const std::vector <Word> & getAttributes ();
	
	virtual Declaration templateDeclReplace (const std::map <std::string, Expression>& tmps);
	
	virtual IFunction* templateReplace (const std::map <std::string, Expression>& tmps);

	/**
	 * \return the type of the function
	 */
	Expression getType ();

	/**
	 * \return the decoration of the return type
	 */
	Word getRetDeco ();

	/**
	 * \return The external linkage of the function 
	 */
	std::string& externLang ();

	/**
	 * \return The external namespace linkage of the function 
	 */
	std::string& externLangSpace ();

	/**
	 * \return the test of the function
	 */
	Expression getTest ();

	/**
	 * \return the body of the function
	 */
	Block getBlock ();
	
	void declare () override;
	
	void declare (semantic::Module)	override;

	void declareAsExtern (semantic::Module) override;
	
	void print (int nb = 0) override;

	/**
	 * \return get or set the pre block
	 */
	Block& pre ();

	/**
	 * \return get or set the post block
	 */
	Block& post ();

	/**
	 * \return get or set the post var
	 */
	Var& postVar ();

	/**
	 * \return true if the function possess the attribute <em>str</em>
	 */
	bool has (const std::string & str);
	
	static const char* id ();
	
	std::vector <std::string> getIds () override;

	virtual ~IFunction ();
	
    protected:

	/**
	 * \brief verify the purety of the function 
	 * \return a frame corresponding to the information given
	 */
	semantic::Frame verifyPure (semantic::Namespace);

	/**
	 * \brief verify the purety of the function 
	 * \return a frame corresponding to the information given (as external)
	 */
	semantic::Frame verifyPureExtern (semantic::Namespace);

	/**
	 * \return if the udas are valid return true, return false and throw errors otherwise
	 */
	virtual bool verifUdas ();

	/**
	 * \brief Transform the function to a extern prototype 
	 */
	Proto toProto ();

	/**
	 * \brief Verify that the templates are valid
	 */
	bool verifyTemplates ();
	
    };

    typedef IFunction* Function;
    
}
