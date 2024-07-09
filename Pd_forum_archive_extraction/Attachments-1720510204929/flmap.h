/* 

flext - C++ layer for Max/MSP and pd (pure data) externals

Copyright (c) 2001-2004 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

/*! \file flmap.h
	\brief special map class for all 32-bit key/value-pairs   
*/

#ifndef __FLMAP_H
#define __FLMAP_H

#include <map>

/*!	\defgroup FLEXT_SUPPORT Flext support classes
	@{
*/

//! Base class for maps
class AnyMap:
    public std::map<unsigned int,unsigned int>
{
    typedef std::map<unsigned int,unsigned int> Parent;
public:
    AnyMap();
    ~AnyMap();
    iterator find(unsigned int k);
    unsigned int &operator [](unsigned int k);

    typedef std::pair<unsigned int,unsigned int> pair;
};

//! Specialized map class for any 32-bit key/value types
template <class K,class T>
class DataMap:
    public AnyMap
{
public:
    class iterator:
        public AnyMap::iterator
    {
    public:
        iterator() {}
#if defined(_MSC_VER) && (_MSC_VER < 0x1300)
        // with the MSVC6 STL implementation iterators can't be initialized...
        iterator(AnyMap::iterator &it) { static_cast<AnyMap::iterator &>(*this) = it; }
#else
        iterator(AnyMap::iterator &it): AnyMap::iterator(it) {}
#endif

        inline K &key() const { return *(K *)&((*this)->first); }
        inline T &data() const { return *(T *)&((*this)->second); }
    };

    class pair:
        public AnyMap::pair
    {
	public:
        inline K &key() const { return *(K *)&first; }
        inline T &data() const { return *(T *)&second; }
	};

    inline iterator find(K k) { return AnyMap::find(*(unsigned int *)&k); }
    inline T &operator [](K k) { return *(T *)&(AnyMap::operator [](*(unsigned int *)&k)); }
    inline void erase(K k) { AnyMap::erase(*(unsigned int *)&k); }
};

//! @} // FLEXT_SUPPORT

#endif
