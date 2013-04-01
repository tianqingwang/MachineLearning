#ifndef __HASH_H__
#define __HASH_H__

#include <stdio.h>
#include <stdlib.h>

/*
typedef unsigned int      uint;
typedef unsigned short    ushort;
typedef unsigned long     ulong;
typedef long long         llong;
*/

/*You can fill data here*/
struct Element{
    unsigned int ikey;
	int value;
};

class CHash{
public:
    /*constructor function*/
    CHash(unsigned int ihashsize);
	/*deconstructor function*/
	~CHash();
	/*return the hash size*/
	unsigned int GetHashSize(void);
	
private:
    unsigned int m_ihashsize;
	Element  *m_pElement;
};

#endif
