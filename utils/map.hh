#pragma once

/**
 * \brief C++ map does not have a simple insert_or_replace function ...
 * \brief It is mandatory for the map to have access to a init constructor for T3, which is not always possible
 */
template <typename T, typename T2, typename T3> 
void insert_or_assign (T & a, const T2 & b, const T3 & c) {
    if (a.find (b) != a.end ())
	a.erase (b);
    a.emplace (b, c);
}
