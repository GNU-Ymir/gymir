// #include <ymir/syntax/expression/MacroCall.hh>

// namespace syntax {

//     MacroCall::MacroCall () :
// 	IExpression (lexing::Word::eof ()),
// 	_left (Expression::empty ())
//     {}
    
//     MacroCall::MacroCall (const lexing::Word & loc) :
// 	IExpression (loc),
// 	_left (Expression::empty ())
//     {}

//     Expression MacroCall::init (const lexing::Word & location, const Expression & left, const std::vector <lexing::Word> & content) {
// 	auto ret = new (Z0) MacroCall (location);
// 	ret-> _left = left;
// 	ret-> _content = content;
// 	return Expression {ret};
//     }

//     Expression MacroCall::clone () const {
// 	return Expression {new (Z0) MacroCall (*this)};
//     }

//     bool MacroCall::isOf (const IExpression * type) const {
// 	auto vtable = reinterpret_cast <const void* const *> (type) [0];
// 	MacroCall thisType; // That's why we cannot implement it for all class
// 	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
// 	return IExpression::isOf (type);
//     }
    
// }
