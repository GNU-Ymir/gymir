#include <ymir/utils/Enum.hh>
#include <ymir/utils/OutBuffer.hh>
#include <ymir/utils/Range.hh>

using namespace Ymir;

std::vector<std::string> splitString(const std::string& str, char sep) {
    std::vector<std::string> vecString;
    OutBuffer buf;
    int i = 0;
    
    while (i < str.length ())
	{
	    if (str [i] == sep) {
		vecString.push_back(buf.str ());
		buf = OutBuffer ();
	    } else {
		buf.write (str [i]);
	    }
	}

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
	}
    }
    return end.str ();
}
