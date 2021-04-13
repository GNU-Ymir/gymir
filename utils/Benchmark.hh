#pragma once
#include <time.h>
#include <ymir/utils/OutBuffer.hh>

#define STAMP(x)					\
    static int x##__all__ = 0, x##__recursive__ = 0, x##__nb_call__ = 0; \
    x##__recursive__ += 1;						\
    x##__nb_call__ += 1;						\
    int x = clock ();							\

#define ELAPSED(x)							\
    x##__recursive__ -= 1;						\
    if (x##__recursive__ == 0) {					\
	x = clock () - x;						\
	x##__all__ += (x);						\
	println (#x, " ", x##__nb_call__, " ", ((float) x) / CLOCKS_PER_SEC, " / ", ((float) x##__all__) / CLOCKS_PER_SEC); \
    }


#define ELAPSED_MSG(x, msg)						\
    x##__recursive__ -= 1;						\
    if (x##__recursive__ == 0) {					\
	x = clock () - x;						\
	x##__all__ += (x);						\
	println (#x, " ", msg, " ", x##__nb_call__, " ", ((float) x) / CLOCKS_PER_SEC, " / ", ((float) x##__all__) / CLOCKS_PER_SEC); \
    }


#define IGNORE_SEG(x)				\
    x##__recursive__ += 1;

#define END_IGNORE_SEG(x)			\
    x##__recursive__ -= 1; 
    
