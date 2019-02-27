#pragma once

/**
 * Implementation of a proxy design pattern
 * This allows to use polymorphism with hidden allocation, and ensure that all allocation or freed when unused
 * Each instance of the proxy have their own data
 * \param T inner type that will be collected
 * \param I the heir type that is the real proxy
 */
template <typename T, class I> class Proxy {    

    bool _ownership = true;

protected:

    T * _value;
    
public:
    
    Proxy<T, I> (T * a) {
	this-> _value = a;
    }

    Proxy<T, I> (const Proxy<T, I> & ot) {
	I a (ot.clone ());
	a._ownership = false;
	this-> _value = a._value;
    }

    Proxy<T, I>& operator=(const Proxy<T, I> & ot) {
	I a (ot.clone ());
	a._ownership = false;

	// if (this-> _ownership && this-> _value != nullptr)
	//     delete this-> _value;
	
	this-> _value = a._value;
	
	return *this;
    }

    bool isRef () {
	return this-> _ownership == false;
    }
    
    void setRef (T * a) {
	this-> _value = a;
	this-> _ownership = false;
    }
    
    I clone () const {
	if (this-> _value != nullptr)
	    return this-> _value-> clone ();
	else return I (nullptr);
    }
    
    virtual ~Proxy<T, I> () {
	// if (this-> _ownership && this-> _value != nullptr)
	//     delete this-> _value;
    }
    
};
