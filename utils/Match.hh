#pragma once

#define match(X)				\
    auto & ref = X;

#define of(X, x, LMD)				\
if (ref.is<X> ()) {				\
    auto & x = ref.to <X> ();			\
    LMD						\
 }						
						
