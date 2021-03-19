#pragma once

#define match(X)				\
    auto & ref = X;

#define match_forall(X)				\
    for (auto & ref : X)

#define s_of_u(X)				\
    if (ref.is<X> ()) 

#define of_u(X)					\
    if (ref.is<X> ()) {						

#define s_of(X, x)				\
    if (ref.is <X> ())				\
	if (auto & x = ref.to <X> () ; true)	

#define of(X, x)				\
    if (ref.is <X> ()) {			\
	if (auto & x = ref.to <X> () ; true)	

#define elof(X, x)				\
    } else of (X, x)

#define elof_u(X)				\
    } else if (ref.is<X> ()) {

#define fo }					\

#define elfo } else				\



#define string_match(X)				\
    auto & ref = X;

#define eq(V)					\
    if (ref == V)
