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
CHash::CHash(unsigned int ihashsize)
{
    m_ihashsize = ihashsize;
	m_pElement  = new Element[ihashsize];
	
}

/*deconstruct function*/
CHash::~CHash()
{
    delete [] m_pElement;
}

/*get hash size*/
unsigned int CHash::GetHashSize(void)
{
    return m_ihashsize;
}

/*insert a new node*/
bool CHash::lookup(Element *pElement)
{
    
}




