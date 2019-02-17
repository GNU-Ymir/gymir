#pragma once

#define match(X)				\
    auto & ref = X;

#define of(X, x, LMD)				\
if (ref.is<X> ()) {				\
    auto & x = ref.to <X> ();			\
    LMD;					\
 }						

#define of_u(X, LMD)				\
if (ref.is<X> ()) {				\
    LMD;					\
 }						

#define string_match(X)				\
    auto & ref = X;

#define eq(V, RESULT)				\
    if (ref == V) {				\
	RESULT;					\
    }
