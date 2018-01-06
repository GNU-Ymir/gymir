#include "errors/Error.hh"
#include "errors/Languages.hh"
#include <ymir/utils/OutBuffer.hh>
#include <ymir/ast/_.hh>
#include <ymir/semantic/types/InfoType.hh>
#include <ymir/semantic/pack/Symbol.hh>
#include <sstream>
#include <iostream>
#include <string>

namespace Ymir {    	
    using namespace Private;
    
    const char* Error::RESET = "\u001B[0m";
    const char* Error::PURPLE = "\u001B[1;35m";
    const char* Error::BLUE = "\u001B[1;36m";
    const char* Error::YELLOW = "\u001B[1;33m";
    const char* Error::RED = "\u001B[1;31m";
    const char* Error::GREEN = "\u001B[1;32m";
    const char* Error::BOLD = "\u001B[1;50m";
    
    unsigned long Error::nb_errors (0);
    
    std::vector <ErrorMsg> Error::__caught__;
    bool Error::__isEnable__ = true;
    Error Error::__instance__;

    std::string substr (std::string& x, ulong beg, ulong end) {
	if (end - beg > x.length ()) return "";
	return x.substr (beg, end - beg);
    }
    
    std::string readln (FILE * i) {
	unsigned long max = 255;
	std::string final = "";
	while (1) {
	    char * buf = new char[max];
	    char * aux = fgets(buf, max, i);
	    if (aux == NULL) return "";
	    std::string ret = std::string (buf);
	    final += ret;
	    free (buf);
	    if (ret.size () != max - 1) return final;
	    else max *= 2;      
	}
    }  
    
    std::string getLine (location_t locus) {
	auto file = fopen (LOCATION_FILE (locus), "r");
	std::string cline;
	for (auto it = 0 ; it < LOCATION_LINE (locus) ; it ++) {
	    cline = readln (file);
	}
	return cline;
    }
	
    ulong computeMid (std::string& mid, std::string word, std::string line, ulong begin, ulong end) {
	auto end2 = begin;
	for (auto it = 0 ; it < (int) std::min (word.size (), end - begin); it ++) {
	    if (word [it] != line [begin + it]) break;
	    else end2 ++;
	}
	mid = substr (line, begin, end2);
	return end2;
    }	
    
    std::string center (std::string toCenter, ulong nb, char concat) {
	std::ostringstream ss;
	nb = nb - toCenter.length ();
	for (int i = 0 ; i < (int) nb / 2 ; i++) {
	    ss << (char) concat;
	}
	ss << toCenter;
	for (int i = 0 ; i < (int) (nb - nb / 2) ; i++)
	    ss << (char) concat;
	return ss.str ();
    }

    std::string rightJustify (std::string toJustify, ulong nb, char concat) {
	nb = nb - toJustify.length ();
	std::ostringstream ss;
	for (int i = 0 ; i < (int) nb; i++) ss << concat;
	ss << toJustify;
	return ss.str ();
    }
    
    bool isVerbose () {
	return true;
    }

    void addLine (std::ostringstream &ss, Word word) {
	auto locus = word.getLocus ();
	auto line = getLine (locus);
	if (line.length () > 0) {
	    std::string leftLine = center (format ("%", LOCATION_LINE (locus)), 3, ' ');
	    auto padd = center ("", leftLine.length (), ' ');
	    ss << format ("\n% --> %:(%,%)%\n%% | %\n", 
			  Error::BOLD,
			  LOCATION_FILE (locus),
			  LOCATION_LINE (locus),
			  LOCATION_COLUMN (locus),
			  Error::RESET,
			  Error::BOLD,
			  padd.c_str (),
			  Error::RESET
	    );

	    auto column = LOCATION_COLUMN (locus);
	    ss << format ("%% | %%%%%%",
			  Error::BOLD,
			  leftLine.c_str (),
			  Error::RESET,
			  line.substr (0, column - 1).c_str (),
			  Error::YELLOW,
			  substr (line, column - 1, column + word.getStr ().length () - 1).c_str (),
			  Error::RESET,
			  substr (line, column + word.getStr ().length () - 1, line.length ()).c_str ()
	    );

	    if (line [line.length () - 1] != '\n') ss << "\n";	    	    
	    ss << format ("%% | %",
			  Error::BOLD,
			  padd.c_str (),
			  Error::RESET
	    );
	    for (auto it = 0 ; it < column - 1 ; it ++) {
		if (line [it] == '\t') ss << "\t";
		else ss << ' ';
	    }
	    
	    ss << rightJustify ("", word.getStr ().length (), '^');
	    ss << "\n";
	} else {
	    ss << format ("\n%Error% : %\n",
			  Error::RED,
			  Error::RESET,
			  getString (EndOfFile)
	    );
	}
    }

    void addLine (std::ostringstream &ss, Word word, Word word2) {
	auto locus = word.getLocus (), locus2 = word2.getLocus ();
	auto line = getLine (locus);
	if (line.size () > 0) {
	    auto leftLine = center (format ("%", LOCATION_LINE (locus)), 3, ' ');
	    auto padd = center ("", leftLine.size (), ' ');
	    ss << format ("\n% --> %:(%,%)%\n%% | %\n",
			  Error::BOLD, LOCATION_FILE (locus),  LOCATION_LINE (locus), LOCATION_COLUMN (locus),
			  Error::RESET, Error::BOLD,
			  padd.c_str (),
			  Error::RESET
	    );

	    std::string mid, mid2;
	    auto locCol1 = LOCATION_COLUMN (locus), locCol2 = LOCATION_COLUMN (locus2);
	    auto end1 = computeMid (mid, word.getStr (), line, locCol1 - 1, locCol1 + word.getStr ().size () - 1);
	    auto end2 = computeMid (mid2, word2.getStr (), line, locCol2 - 1, locCol2 + word2.getStr ().size () - 1);

	    ss << format ("%% | %%%%%%%%%%",
			  Error::BOLD,
			  leftLine.c_str (),
			  Error::RESET,
			  substr (line, 0, locCol1 - 1).c_str (),
			  Error::YELLOW,
			  mid.c_str (),
			  Error::RESET,
			  substr (line, end1, locCol2 - 1).c_str (),
			  Error::YELLOW,
			  mid2.c_str (),
			  Error::RESET,
			  substr (line, end2, line.size ()).c_str ()
	    );

	    if (line [line.size () - 1] != '\n') ss << '\n';
	    ss << format ("%% | %",
			  Error::BOLD,
			  padd.c_str (),
			  Error::RESET
	    );

	    for (auto it : Ymir::r (0, locCol1 - 1)) {
		if (line [it] == '\t') ss << "\t";
		else ss << ' ';
	    }

	    ss << rightJustify ("", end1 - locCol1 + 1, '^');
	    for (auto it : Ymir::r (end1, locCol2 - 1)) {
		if (line [it] == '\t') ss << "\t";
		else ss << ' ';
	    }
	    ss << rightJustify ("", end2 - locCol2 + 1, '^') << "\n";
	} else {
	    ss << format ("\n%Error% : %\n",
			  Error::RED,
			  Error::RESET,
			  getString (EndOfFile)
	    );
	}
    }

    void addLine (std::ostream &, Word, ulong, ulong) {}
    
    std::string addLine (std::string in, Word word) {
	std::ostringstream ss;
	ss << in;
	addLine (ss, word);
	return ss.str ();
    }    

    std::string addLine (std::string in, Word word, Word word2) {
	std::ostringstream ss;
	ss << in;
	addLine (ss, word, word2);
	return ss.str ();
    }    

    

    namespace Private {
	const char * getString (ErrorType type, Language ln) {
	    auto strings = Languages::getLanguage (ln);
	    return strings [type];	
	}
    }
    

    void Error::activeError (bool isActive) {
	__isEnable__ = isActive;
	if (isActive) {
	    __caught__.clear ();
	}
    }

    void Error::notATemplate (Word word) {
	auto str = getString (NotATemplate);
	std::string msg = format (str, YELLOW, word.getStr ().c_str (), RESET);
	msg = std::string (RED) + "Error" + std::string (RESET) + " : " + msg;
	msg = addLine (msg, word);
	ErrorMsg errorMsg = {msg, false, false};
	if (__isEnable__) {
	    Error::instance ().nb_errors ++;
	    printf ("%s", errorMsg.msg.c_str ());
	} else __caught__.push_back (errorMsg);
    }

    void Error::noValueNonVoidFunction (Word word) {
	auto str = getString (NoValueNonVoid);
	std::string msg = format (str, YELLOW, word.getStr ().c_str (), RESET);
	msg = std::string (RED) + "Error" + std::string (RESET) + " : " + msg;
	msg = addLine (msg, word);
	ErrorMsg errorMsg = {msg, false, false};
	if (__isEnable__) {
	    Error::instance ().nb_errors ++;
	    printf ("%s", errorMsg.msg.c_str ());
	} else __caught__.push_back (errorMsg);
    }
    
    void Error::takeATypeAsTemplate (Word word) {
	std::string msg = getString (TakeAType);
	msg = std::string (RED) + "Error" + std::string (RESET) + " : " + msg;
	msg = addLine (msg, word);
	ErrorMsg errorMsg = {msg, false, false};
	if (__isEnable__) {
	    Error::instance ().nb_errors ++;
	    printf ("%s", errorMsg.msg.c_str ());
	} else __caught__.push_back (errorMsg);
    }

    void Error::unknownType (Word word) {
	std::string msg = format (getString (UnknownType), YELLOW, word.getStr ().c_str (), RESET);
	msg = std::string (RED) + "Error" + std::string (RESET) + " : " + msg;
	msg = addLine (msg, word);
	ErrorMsg errorMsg = {msg, false, false};
	if (__isEnable__) {
	    Error::instance ().nb_errors ++;
	    printf ("%s", errorMsg.msg.c_str ());
	} else __caught__.push_back (errorMsg);
    }

    void Error::recursiveExpansion (Word word) {
	std::string msg = getString (RecursiveExpansion);
	msg = std::string (RED) + "Error" + std::string (RESET) + " : " + msg;
	msg = addLine (msg, word);
	ErrorMsg errorMsg = {msg, false, false};
	if (__isEnable__) {
	    Error::instance ().nb_errors ++;
	    printf ("%s", errorMsg.msg.c_str ());
	} else __caught__.push_back (errorMsg);
    }

    void Error::shadowingVar (Word word, Word word2) {
	std::string msg = format (getString (ShadowingVar), YELLOW, word.getStr ().c_str (), RESET);
	msg = std::string (RED) + "Error" + std::string (RESET) + " : " + msg;
	msg = addLine (msg, word);

	
	auto msg2 = std::string (getString (Here));
	auto aux = std::string (BLUE) + "Note" + std::string (RESET) + " : " + msg2;
	aux = addLine (aux, word2);
	
	ErrorMsg errorMsg = {msg + aux, false, false};
	if (__isEnable__) {
	    Error::instance ().nb_errors ++;
	    printf ("%s", errorMsg.msg.c_str ());
	} else __caught__.push_back (errorMsg);
    }
    
    void Error::multipleLoopName (Word word, Word word2) {
	std::string msg = format (getString (MultipleLoopName), YELLOW, word.getStr ().c_str (), RESET);
	msg = std::string (RED) + "Error" + std::string (RESET) + " : " + msg;
	msg = addLine (msg, word);

	
	auto msg2 = std::string (getString (Here));
	auto aux = std::string (BLUE) + "Note" + std::string (RESET) + " : " + msg2;
	aux = addLine (aux, word2);
	
	ErrorMsg errorMsg = {msg + aux, false, false};
	if (__isEnable__) {
	    Error::instance ().nb_errors ++;
	    printf ("%s", errorMsg.msg.c_str ());
	} else __caught__.push_back (errorMsg);
    }

    
    
    void Error::syntaxError (Word word) {
	auto str = getString (SyntaxError2);
	auto msg = format (str, YELLOW, word.getStr ().c_str (), RESET);
	msg = std::string (RED) + "Error" + std::string (RESET) + " : " + std::string (msg);
	msg = addLine (msg, word);
	ErrorMsg errorMsg = {msg, false, false};
	if (__isEnable__) {
	    Error::instance ().nb_errors ++;
	    fail ("%s", errorMsg.msg.c_str ());	    
	} else __caught__.push_back (errorMsg);
    }

    void Error::syntaxError (Word word, const char * expected) {
	auto str = getString (SyntaxError);
	auto msg = format (str, expected, YELLOW, word.getStr ().c_str (), RESET);

	msg = addLine (msg, word);
	ErrorMsg errorMsg = {msg, true, false};
	if (__isEnable__) {	    
	    Error::instance ().nb_errors ++;
	    fail("%s", errorMsg.msg.c_str ());
	} else __caught__.push_back (errorMsg);
    }

    void Error::syntaxError (Word word, Word word2) {
	auto str = getString (SyntaxError);
	auto msg = format (str, word2.getStr().c_str (),
			   YELLOW, word.getStr ().c_str (), RESET);
	
	msg = addLine (msg, word);
	ErrorMsg errorMsg = {msg, false, false};
	if (__isEnable__) {
	    Error::instance ().nb_errors ++;
	    fail ("%s", errorMsg.msg.c_str ());
	} else __caught__.push_back (errorMsg);
    }


    void Error::syntaxErrorFor (Word word, Word word2) {
	auto str = getString (SyntaxErrorFor);
	auto msg = format (str,
			   YELLOW, word.getStr ().c_str (), RESET,
			   YELLOW, word2.getStr ().c_str (), RESET
	);
	
	msg = addLine (msg, word);
	ErrorMsg errorMsg = {msg, false, false};
	if (__isEnable__) {
	    Error::instance ().nb_errors ++;
	    fail ("%s", errorMsg.msg.c_str ());
	} else __caught__.push_back (errorMsg);
    }
    
    void Error::escapeError (Word word) {
	auto msg = std::string (getString (EscapeChar));	
	msg = addLine (msg, word);
	ErrorMsg errorMsg = {msg, false, false};
	if (__isEnable__) {
	    Error::instance ().nb_errors ++;
	    printf ("%s", errorMsg.msg.c_str ());
	} else __caught__.push_back (errorMsg);
    }
    
    void Error::endOfFile () {
	auto msg = std::string (getString (EndOfFile));	
	msg = std::string (RED) + "Error" + std::string (RESET) + " : " + std::string (msg);
	ErrorMsg errorMsg = {msg, false, false};
	if (__isEnable__) {
	    Error::instance ().nb_errors ++;
	    printf ("%s", errorMsg.msg.c_str ());
	} else __caught__.push_back (errorMsg);
    }

    void Error::templateCreation (Word word) {
	auto str = getString (TemplateCreation);
	auto msg = format (str, YELLOW, word.getStr ().c_str (), RESET);
	msg = std::string (BLUE) + "Note" + std::string (RESET) + " : " + std::string (msg);
	msg = addLine (msg, word);
	ErrorMsg errorMsg = {msg, false, false};
	if (__isEnable__) {
	    Error::instance ().nb_errors ++;
	    printf ("%s", errorMsg.msg.c_str ());
	} else __caught__.push_back (errorMsg);
    }
    
    void Error::unterminated (Word word) {
	auto str = getString (Unterminated);
	auto msg = format (str, YELLOW, word.getStr ().c_str (), RESET);	
	msg = std::string (RED) + "Error" + std::string (RESET) + " : " + std::string (msg);
	msg = addLine (msg, word);
	ErrorMsg errorMsg = {msg, false, false};
	if (__isEnable__) {
	    Error::instance ().nb_errors ++;
	    printf ("%s", errorMsg.msg.c_str ());
	} else __caught__.push_back (errorMsg);
    }

    void Error::uninitVar (Word word) {
	auto str = getString (UninitVar);
	auto msg = format (str, YELLOW, word.getStr ().c_str (), RESET);	
	msg = std::string (RED) + "Error" + std::string (RESET) + " : " + std::string (msg);
	msg = addLine (msg, word);
	ErrorMsg errorMsg = {msg, false, false};
	if (__isEnable__) {
	    Error::instance ().nb_errors ++;
	    printf ("%s", errorMsg.msg.c_str ());
	} else __caught__.push_back (errorMsg);
    }

    void Error::useAsVar (Word word, semantic::Symbol) {
	auto str = getString (UseAsVar);
	auto msg = format (str, YELLOW, word.getStr ().c_str (), RESET);	
	msg = std::string (RED) + "Error" + std::string (RESET) + " : " + std::string (msg);
	msg = addLine (msg, word);
	ErrorMsg errorMsg = {msg, false, false};
	if (__isEnable__) {
	    Error::instance ().nb_errors ++;
	    printf ("%s", errorMsg.msg.c_str ());
	} else __caught__.push_back (errorMsg);
    }

    void Error::breakRefUndef (Word word) {
	auto str = getString (BreakRefUndef);
	auto msg = format (str, YELLOW, word.getStr ().c_str (), RESET);	
	msg = std::string (RED) + "Error" + std::string (RESET) + " : " + std::string (msg);
	msg = addLine (msg, word);
	ErrorMsg errorMsg = {msg, false, false};
	if (__isEnable__) {
	    Error::instance ().nb_errors ++;
	    printf ("%s", errorMsg.msg.c_str ());
	} else __caught__.push_back (errorMsg);
    }

    void Error::constNoInit (Word word) {
	auto str = getString (ConstNoInit);
	auto msg = format (str, YELLOW, word.getStr ().c_str (), RESET);
	msg = std::string (RED) + "Error" + std::string (RESET) + " : " + std::string (msg);
	msg = addLine (msg, word);
	ErrorMsg erroMsg = {msg, false, false};
	if (__isEnable__) {
	    Error::instance ().nb_errors ++;
	    printf ("%s", erroMsg.msg.c_str ());
	} else __caught__.push_back (erroMsg);
    }
    
    void Error::notLValue (Word word) {
	auto str = getString (NotLValue);
	auto msg = format (str, YELLOW, word.getStr ().c_str (), RESET);
	msg = std::string (RED) + "Error" + std::string (RESET) + " : " + std::string (msg);
	msg = addLine (msg, word);
	ErrorMsg erroMsg = {msg, false, false};
	if (__isEnable__) {
	    Error::instance ().nb_errors ++;
	    printf ("%s", erroMsg.msg.c_str ());
	} else __caught__.push_back (erroMsg);
    }
    
    void Error::breakOutSide (Word word) {
	auto msg = std::string (getString (BreakOutSide));
	msg = std::string (RED) + "Error" + std::string (RESET) + " : " + std::string (msg);
	msg = addLine (msg, word);
	ErrorMsg errorMsg = {msg, false, false};
	if (__isEnable__) {
	    Error::instance ().nb_errors ++;
	    printf ("%s", errorMsg.msg.c_str ());
	} else __caught__.push_back (errorMsg);
    }

    void Error::unreachableStmt (Word word) {
	auto msg = std::string (getString (UnreachableStmt));
	msg = std::string (RED) + "Error" + std::string (RESET) + " : " + std::string (msg);
	msg = addLine (msg, word);
	ErrorMsg errorMsg = {msg, false, false};
	if (__isEnable__) {
	    Error::instance ().nb_errors ++;
	    printf ("%s", errorMsg.msg.c_str ());
	} else __caught__.push_back (errorMsg);
    }

    void Error::missingReturn (Word word, semantic::Symbol type) {
	auto msg = format (getString (MissingReturn), YELLOW, word.getStr (), RESET, YELLOW, type-> typeString (), RESET);
	msg = std::string (RED) + "Error" + std::string (RESET) + " : " + std::string (msg);
	msg = addLine (msg, word);
	ErrorMsg errorMsg = {msg, false, false};
	if (__isEnable__) {
	    Error::instance ().nb_errors ++;
	    printf ("%s", errorMsg.msg.c_str ());
	} else __caught__.push_back (errorMsg);
    }
    
    void Error::returnVoid (Word word, semantic::Symbol type) {
	auto msg = format (getString (ReturnVoid), YELLOW, type-> type-> typeString ().c_str (), RESET);
	msg = std::string (RED) + "Error" + std::string (RESET) + " : " + std::string (msg);
	msg = addLine (msg, word);
	ErrorMsg errorMsg = {msg, false, false};
	if (__isEnable__) {
	    Error::instance ().nb_errors ++;
	    printf ("%s", errorMsg.msg.c_str ());
	} else __caught__.push_back (errorMsg);
    }    
    
    void Error::incompatibleTypes (Word where, semantic::Symbol type, semantic::InfoType other) {
	auto str = getString (IncompatibleTypes);
	auto msg = format (str, YELLOW, type-> type-> typeString ().c_str (), RESET,
			   YELLOW, other-> typeString ().c_str (), RESET
	);
	
	msg = std::string (RED) + "Error" + std::string (RESET) + " : " + std::string (msg);
	msg = addLine (msg, where);
	
	ErrorMsg errorMsg = {msg, false, false};
	if (__isEnable__) {
	    Error::instance ().nb_errors ++;
	    printf ("%s", errorMsg.msg.c_str ());
	} else __caught__.push_back (errorMsg);
    }

    
    void Error::useAsType (Word word) {
	auto str = getString (UseAsType);
	auto msg = format (str, YELLOW, word.getStr ().c_str (), RESET);	
	msg = std::string (RED) + "Error" + std::string (RESET) + " : " + std::string (msg);
	msg = addLine (msg, word);
	ErrorMsg errorMsg = {msg, false, false};
	if (__isEnable__) {
	    Error::instance ().nb_errors ++;
	    printf ("%s", errorMsg.msg.c_str ());
	} else __caught__.push_back (errorMsg);
    }
    
    void Error::undefinedOp (Word begin, semantic::Symbol elem, syntax::ParamList params) {
	auto str = getString (UndefinedOp);
	OutBuffer buf;
	ulong i = 0;
	for (auto it : params-> getParamTypes ()) {
	    buf.write (it-> typeString ());
	    if (i < params-> getParamTypes ().size () - 1)
		buf.write (", ");
	    i++;
	}
	
	auto msg = format (str,
			   YELLOW, begin.getStr().c_str (), RESET,
			   YELLOW, elem-> typeString ().c_str (), RESET,
			   buf.str ().c_str ());

	msg = std::string (RED) + "Error" + std::string (RESET) + " : " + std::string (msg);
	msg = addLine (msg, begin);
	
	ErrorMsg errorMsg = {msg, false, false};
	if (__isEnable__) {
	    Error::instance ().nb_errors ++;
	    printf ("%s", errorMsg.msg.c_str ());
	} else __caught__.push_back (errorMsg);
    }

    void Error::undefinedOp (Word begin, Word end, semantic::Symbol elem, syntax::ParamList params) {
	auto str = getString (UndefinedOpMult);
	OutBuffer buf;
	ulong i = 0;
	for (auto it : params-> getParamTypes ()) {
	    buf.write (YELLOW, it-> typeString (), RESET);
	    if (i < params-> getParamTypes ().size () - 1)
		buf.write (", ");
	    i++;
	}
	
	auto msg = format (str, YELLOW, begin.getStr().c_str (), end.getStr ().c_str (), RESET, YELLOW, elem-> typeString ().c_str (), RESET, buf.str ().c_str ());
	msg = std::string (RED) + "Error" + std::string (RESET) + " : " + std::string (msg);
	msg = addLine (msg, begin, end);
	
	ErrorMsg errorMsg = {msg, false, false};
	if (__isEnable__) {
	    Error::instance ().nb_errors ++;
	    printf ("%s", errorMsg.msg.c_str ());
	} else __caught__.push_back (errorMsg);
    }

    void Error::undefinedOp (Word op, semantic::Symbol left, semantic::Symbol right) {
	auto str = getString (UndefinedOp);
	auto msg = format (str,
			   YELLOW, op.getStr ().c_str (), RESET,
			   YELLOW, left-> type-> typeString ().c_str (), RESET,
			   YELLOW, right-> type-> typeString ().c_str (), RESET
	);
	
	msg = std::string (RED) + "Error" + std::string (RESET) + " : " + std::string (msg);
	msg = addLine (msg, op);
	ErrorMsg errorMsg = {msg, false, false};
	if (__isEnable__) {
	    Error::instance ().nb_errors ++;
	    printf ("%s", errorMsg.msg.c_str ());
	} else __caught__.push_back (errorMsg);
    }	

    void Error::undefinedOp (Word op, semantic::Symbol left) {
	auto str = getString (UndefinedOpUnary);
	auto msg = format (str,
			   YELLOW, op.getStr ().c_str (), RESET,
			   YELLOW, left-> type-> typeString ().c_str (), RESET
	);
	
	msg = std::string (RED) + "Error" + std::string (RESET) + " : " + std::string (msg);
	msg = addLine (msg, op);
	ErrorMsg errorMsg = {msg, false, false};
	if (__isEnable__) {
	    Error::instance ().nb_errors ++;
	    printf ("%s", errorMsg.msg.c_str ());
	} else __caught__.push_back (errorMsg);
    }	
    
    void Error::undefinedOp (Word op, semantic::Symbol left, semantic::InfoType right) {
	auto str = getString (UndefinedOp);
	auto msg = format (str,
			   YELLOW, op.getStr ().c_str (), RESET,
			   YELLOW, left-> type-> typeString ().c_str (), RESET,
			   YELLOW, right-> typeString ().c_str (), RESET
	);
	
	msg = std::string (RED) + "Error" + std::string (RESET) + " : " + std::string (msg);
	msg = addLine (msg, op);
	ErrorMsg errorMsg = {msg, false, false};
	if (__isEnable__) {
	    Error::instance ().nb_errors ++;
	    printf ("%s", errorMsg.msg.c_str ());
	} else __caught__.push_back (errorMsg);
    }	

    
    void Error::templateSpecialisation (Word word, Word word2) {
	auto str = getString (TemplateSpecialisation);
	auto msg = format (str, YELLOW, word.getStr ().c_str (), RESET);	
	msg = std::string (RED) + "Error" + std::string (RESET) + " : " + std::string (msg);
	msg = addLine (msg, word);

	auto msg2 = std::string (getString (And));
	auto aux = std::string (BLUE) + "Note" + std::string (RESET) + " : " + msg2;
	aux = addLine (aux, word2);
	
	ErrorMsg errorMsg = {msg + aux, false, false};
	if (__isEnable__) {
	    Error::instance ().nb_errors ++;
	    printf ("%s", errorMsg.msg.c_str ());
	} else __caught__.push_back (errorMsg);
    }

    void Error::undefVar (Word word, semantic::Symbol alike) {
	std::string msg;
	if (alike != NULL)
	    msg = format (getString (UndefVar2), YELLOW, word.getStr ().c_str (), RESET, YELLOW, alike-> sym.getStr ().c_str (), RESET);
	else
	    msg = format (getString (UndefVar), YELLOW, word.getStr ().c_str (), RESET);
	
	msg = std::string (RED) + "Error" + std::string (RESET) + " : " + std::string (msg);
	msg = addLine (msg, word);
	ErrorMsg errorMsg = {msg, false, false};
	if (__isEnable__) {
	    Error::instance ().nb_errors ++;
	    printf ("%s", errorMsg.msg.c_str ());
	} else __caught__.push_back (errorMsg);
    }
        
    void Error::undefAttr (Word word, semantic::Symbol type, syntax::Var attr) {
	std::string msg = format (getString (UndefinedAttr),
				  YELLOW, attr-> token.getStr ().c_str (), RESET,
				  YELLOW, type-> typeString ().c_str (), RESET);
	
	msg = std::string (RED) + "Error" + std::string (RESET) + " : " + std::string (msg);
	msg = addLine (msg, word);
	ErrorMsg errorMsg = {msg, false, false};
	if (__isEnable__) {
	    Error::instance ().nb_errors ++;
	    printf ("%s", errorMsg.msg.c_str ());
	} else __caught__.push_back (errorMsg);
    }

    void Error::templateInferType (Word token, Word func) {
	std::string msg = format (getString (TemplateInferType));
	std::string msg2 = format (getString (TemplateInferTypeNote));
	
	msg = std::string (RED) + "Error" + std::string (RESET) + " : " + std::string (msg);
	msg = addLine (msg, token);
	
	msg2 = std::string (BLUE) + "Note" + std::string (RESET) + ":" + msg2;
	msg2 = addLine (msg2, func);
	
	ErrorMsg errorMsg = {msg + msg2, false, false};
	if (__isEnable__) {
	    Error::instance ().nb_errors ++;
	    printf ("%s", errorMsg.msg.c_str ());
	} else __caught__.push_back (errorMsg);
    }

    void Error::assert (const char* msg) {
	return __instance__.assert_ (msg);
    }
    
    std::vector <ErrorMsg>& Error::caught () {
	return __caught__;
    }
    
}
