#include "hash.h"

/*famous BKDR Hash function for string*/
inline unsigned int CHash::hashkey(const void *key) const
{
    unsigned int ihash = 0;
	unsigned int seed  = 131;
	
	unsigned char *pkey = (unsigned char*)key;
	
	while(*pkey){
	    ihash = ihash*seed + (*pkey++);
	}
	
	return (ihash & 0x7FFFFFFF);
}

/*contruct function*/
CHash::CHash()
{
    m_ihashsize = 0;
}

/*deconstruct function*/
CHash::~CHash()
{
    
}


void CHash::SetHashSize(unsigned int hashSize)
{
    m_ihashsize = hashSize;
}


unsigned int CHash::GetHashSize(void)
{
    return m_ihashsize;
}





