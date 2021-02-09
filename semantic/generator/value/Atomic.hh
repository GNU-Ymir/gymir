#pragma once

#include <ymir/semantic/generator/Value.hh>

namespace semantic {

    namespace generator {

	class AtomicLocker : public Value {

	    Generator _who;

	    bool _isMonitor;
	    
	private :

	    friend Generator;
	    
	    AtomicLocker ();

	    AtomicLocker (const lexing::Word & loc, const Generator & who, bool isMonitor);

	public :

	    static Generator init (const lexing::Word & loc, const Generator & who, bool isMonitor);
	    
	    Generator clone () const  override;
	    
	    bool equals (const Generator & other) const override;	    

	    const Generator & getWho () const;

	    bool isMonitor () const;
	    
	    std::string prettyString () const override;
	};


	class AtomicUnlocker : public Value {

	    Generator _who;

	    bool _isMonitor;
	    
	private :

	    friend Generator;
	    
	    AtomicUnlocker ();

	    AtomicUnlocker (const lexing::Word & loc, const Generator & who, bool isMonitor);

	public :

	    static Generator init (const lexing::Word & loc, const Generator & who, bool isMonitor);
	    
	    Generator clone () const  override;
	    
	    bool equals (const Generator & other) const override;	    

	    const Generator & getWho () const;

	    bool isMonitor () const;
	    
	    std::string prettyString () const override;
	};

	
    }

}
