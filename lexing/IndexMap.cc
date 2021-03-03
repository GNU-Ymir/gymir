#include <ymir/lexing/IndexMap.hh>
#include <ymir/utils/OutBuffer.hh>

namespace lexing {


    IndexMap::IndexMap (const std::vector <std::string> & tokens) {
	for (auto &it : tokens)
	    this-> insert (it);
    }
    
    void IndexMap::insert (const std::string & token) {
	if (token.length () == 0) return;
	for (auto & it : this-> _heads) {
	    if (it.first == token [0]) {
		it.second-> insert (1, token);
		return;
	    }
	}

	auto node = std::make_shared <Node> (token [0]);
	node-> insert (1, token);
	
	this-> _heads.emplace (token [0], node);       
    }
    
    ulong IndexMap::next (const std::string & content) const {
	return this-> next (0, content);
    }

    ulong IndexMap::next (ulong start, const std::string & content) const {
	for (ulong i = start ; i < content.length () ; i++) {
	    auto fnd = this-> _heads.find (content [i]);
	    if (fnd != this-> _heads.end () && i == start) {
		auto len = fnd-> second-> getLength (start + 1, content);
		if (len != 0) return len;
	    } else if (fnd != this-> _heads.end ()) {
		return i - start;
	    }
	}
	
	return content.length () - start;
    }

    std::list <std::string> IndexMap::tokenize (const std::string & str) const {
	std::list <std::string> result;	
	ulong start = 0;
	while (start < str.length ()) {
	    auto len = this-> next (start, str);	
	    result.push_back (str.substr (start, len));
	    start += len;
	}
		
	return result;
    }

    std::string IndexMap::prettyString () const {
	Ymir::OutBuffer buf;
	buf.write ("{");
	for (auto & i : this-> _heads) {
	    buf.write (i.second-> prettyString (1));
	}
	buf.write ("}");
	return buf.str ();
    }

    
    Node::Node (char key) :
	_key (key)
    {}

    char Node::getKey () const {
	return this-> _key;
    }

    void Node::insert (ulong start, const std::string & token) {
	if (token.length () == start) {
	    this-> _isToken = true;
	    return;
	}
	
	for (auto & it : this-> _heads) {
	    if (it.first == token [start]) {
		it.second-> insert (start + 1, token);
		return;
	    }
	}

	auto node = std::make_shared <Node> (token [start]);
	node-> insert (start + 1, token);
	
	this-> _heads.emplace (token [start], node);
    }

    ulong Node::getLength (ulong start, const std::string & content) const {	
	if (start == content.length ()) {
	    if (this-> _isToken) return 1;
	    else return 0;
	}

	auto fnd = this-> _heads.find (content [start]);
	if (fnd != this-> _heads.end ()) {
	    auto sub_len = fnd-> second-> getLength (start + 1, content);
	    if (sub_len != 0) return 1 + sub_len;
	    else if (this-> _isToken) return 1;
	    else return 0;	
	}
	
	if (this-> _isToken) return 1;
	else return 0;
    }

    std::string Node::prettyString (int padd) const {
	Ymir::OutBuffer buf;
	buf.writef ("%*", padd, '\t');
	buf.writeln ("[", this-> _key, "]", this-> _isToken);
	for (auto it : this-> _heads)
	    buf.write ("=> ", it.second-> prettyString (padd + 1));
	return buf.str ();
    }

    

}
