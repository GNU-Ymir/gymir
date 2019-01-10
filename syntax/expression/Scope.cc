#include <ymir/syntax/expression/Scope.hh>

namespace syntax {

    Scope::Scope () : _content (Expression::empty ()) {}

    Expression Scope::init (const Scope & scope) {
	auto ret = new (Z0) Scope ();
	ret-> _location = scope._location;
	ret-> _content = scope._content;
	return Expression {ret};    
    }

    Expression Scope::init (const lexing::Word & location, const Expression & content) {
	auto ret = new (Z0) Scope ();
	ret-> _location = location;
	ret-> _content = content;
	return Expression {ret};
    }

    Expression Scope::clone () const {
	return Scope::init (*this);
    }

    bool Scope::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Scope thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }

    ScopeFailure::ScopeFailure () {}

    Expression ScopeFailure::init (const ScopeFailure & other) {
	auto ret = new (Z0) ScopeFailure ();
	ret-> _location = other._location;
	ret-> _content = other._content;
	ret-> _names = other._names;
	ret-> _types = other._types;
	ret-> _contents = other._contents;
	return Expression {ret};
    }

    Expression ScopeFailure::init (const lexing::Word & location, const std::vector <lexing::Word> & names, const std::vector <Expression> &types, const std::vector <Expression> & contents) {
	auto ret = new (Z0) ScopeFailure ();
	ret-> _location = location;
	ret-> _content = Expression::empty ();
	ret-> _names = names;
	ret-> _types = types;
	ret-> _contents = contents;
	return Expression {ret};
    }

    Expression ScopeFailure::clone () const {
	return ScopeFailure::init (*this);
    }

    bool ScopeFailure::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	ScopeFailure thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return Scope::isOf (type);
    }
    

}
