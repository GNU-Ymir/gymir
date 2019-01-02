#include <ymir/utils/Enum.hh>
#include <ymir/utils/OutBuffer.hh>
#include <ymir/utils/Range.hh>

using namespace Ymir;

std::vector<std::string> splitString(const std::string& str, char sep) {
    std::vector<std::string> vecString;
    OutBuffer buf;
    int i = 0;

    bool cuttable = true;
    while (i < str.length ())
	{
	    if (str [i] == sep && cuttable) {
		vecString.push_back(buf.str ());
		buf = OutBuffer ();
	    } else if (str [i] == '"') {
		cuttable = !cuttable;
		buf.write (str [i]);
	    } else buf.write (str [i]);
	    i += 1;
	}

    // We add the last word
    if (buf.str ().length () != 0)
	vecString.push_back (buf.str ());
    
    return vecString;
}

std::string strip (const std::string & elem) {
    Ymir::OutBuffer end;
    bool begin = false;
    int nb = 0;
    for (auto it : elem) {
	if (it == ' ') {
	    if (begin) {
		nb ++;
	    }
	} else {
	    if (begin) {
		for (auto i : Ymir::r (0, nb))
		    end.write (' ');
	    } else begin = true;
	    end.write (it);
	    nb = 0;
	}
    }
    auto result = end.str ();
    if (result [0] == '"' && result [result.length () - 1] == '"')
	return result.substr (1, result.length () - 2);
    return result;
}
