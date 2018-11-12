#pragma once


#include "Var.hh"
#include "Declaration.hh"
#include <ymir/utils/Array.hh>
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    /**
     * \struct IProto
     * The syntaxic node representation of a proto
     * A proto is a function without a body (cf. Function)
     */
    class IProto : public IDeclaration {

	/** The return type of the prototype */
	Expression _type;

	/** The return type decoration (const, ref, mut) */
	Word _retDeco;

	/** The list of parameters */
	std::vector <Var> _params;

	/** The space of the proto (in case of external language) */
	std::string _space;

	/** Is this function a variadic one ? */
	bool _isVariadic;

	/** The associated frame */
	semantic::Frame _frame;

	/** The identifier of the prototype */
	Word _ident;

	/** The language from which the linkage will be done */
	std::string _from;

    public:

	/**
	 * \param ident the location and identifier of the proto
	 * \param docs the related comments
	 * \param params the parameters of the proto
	 * \param isVariadic is this prototype variadic ?
	 */
	IProto (Word ident, const std::string & docs, const std::vector <Var>& params, bool isVariadic);

	/**
	 * \param ident the location and identifier of the proto
	 * \param docs the related comments
	 * \param type the return type of the proto
	 * \param retDeco the return type decoration
	 * \param params the parameters of the proto
	 * \param space the external space of the real function declaration
	 * \param isVariadic is this prototype variadic ?
	 */
	IProto (Word ident, const std::string & docs, Expression type, Word retDeco, const std::vector <Var>& params, std::string space, bool isVariadic);

	Ymir::json generateDocs () override;
	
	void declare () override;
	
	void declare (semantic::Module) override;
	
	void declareAsExtern (semantic::Module) override;
	
	Declaration templateDeclReplace (const std::map <std::string, Expression>&) override;

	/**
	 * \return the identifier and location of the proto
	 */
	const Word & getIdent ();
	
	/**
	 * \return the name of this prototype
	 */
	const std::string & getName ();

	/**
	 * \return get or set the type of the proto
	 */
	Expression& type ();

	/**
	 * \return get or set the external language from which the real declaration is from
	 */
	std::string & externLang ();

	/**
	 * \return get or set the external language space from which the real declaration is from
	 */
	std::string & externLangSpace ();

	/**
	 * \return get or set the return type decoration
	 */
	Word& retDeco ();

	/**
	 * \return get or set is this prototype variadic
	 */
	bool& isVariadic ();

	/**
	 * \return get or set the list of parameters of the proto
	 */
	std::vector <Var>& params ();
	
	void print (int nb = 0) override;

	virtual ~IProto ();
	
    };

    typedef IProto* Proto;
}
