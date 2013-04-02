#ifndef _XHO_HASH
#define	_XHO_HASH
#include <stdio.h>
#include "uv.h"
#define TIMEOUT_SECS		120
#define CHECK_TIMEOUT_SECS	10

#define SAVEMSGFILE_SECS	300
#define SAVESTATUSFILE_SECS	300




typedef float		RTV_TYPE;

typedef unsigned char   	uchar;
typedef unsigned short  	ushort;
typedef unsigned int    	uint;
typedef unsigned long   	ulong;
typedef long long		llong;
typedef unsigned long long	ullong;
typedef ushort			wchar;

/* xho cmyhashvoid *****************************/
class cmyhashvoid
{
protected:
    struct cplex {   // warning variable length structure
        cplex* pnext;

        void* data() {
            return this + 1;
        };

        static cplex* create(cplex*& head, int imax, int cbelement);

        void freedatachain();
    };

    // Association
    struct cassoc {
        cassoc* pnext;

        void* key() {
            return this + 1;
        };
        void* value(size_t ikeysize) {
            return ((char *)(this + 1) + ikeysize);
        };

        cassoc() {
            pnext = NULL;
        }
    };

public:

// Construction
    cmyhashvoid(int ihash, int iblock, size_t ikey, size_t ivalue, int (*compar)(const void *, const void *), unsigned int (*outhashkey)(const void *key));
    ~cmyhashvoid();

// Attributes

    // Lookup
    bool Delete(const void* key);
    bool lookup(const void* key, void* value);
    bool lookup(const void* key);
    void timeoutcheck();
    void freevaluehash();
    void recreate();
    void OutPutItem(void *pItem);

// Operations
    // add a new (key, value) pair
    int incsetat(const void* key);
    int setat(const void* key, void* newValue);
    int setatidf(const void* key, void* newValue);
    uint AddClicks(const void* key, int iClickCount);
    uint SetClicks(const void* key, int iClickCount);
    int resetat(const void* key, void* newValue);

//	bool removekey(int key);
    void removeall();

    // iterating all (key, value) pairs
//	POSITION getstartposition() const;
//	void getnextassoc(POSITION& rpos, int& rkey, int& rvalue) const;

    // advanced features for derived classes
    int gethashtablesize() const;
//	void init(int hashsize, int m_blocksize, size_t ikey, size_t ivalue);

    // Routine used to user-provided hash keys
    unsigned int hashkey(const void* key) const;

// Implementation
protected:
    cassoc** m_phash;
    int m_iblocksize;
    size_t m_ikeysize;
    size_t m_ivaluesize;
    size_t m_idatasize;
    struct cplex* m_pblocks;
    int (*m_compar)(const void *, const void *);
    unsigned int (*m_hashkey)(const void *key);
    void freeassoc(cassoc*);

    int m_max_icount;
    int m_ilastkey;

    cassoc* m_pfreelist;

    cassoc* newassoc();
    cassoc* getassocat(const void*, unsigned int&) const;
    cassoc* getassocat_lookup(const void*, unsigned int&) const;

public:
    int m_icount;
    int m_ihashsize;
    int setat_Time(const void* key, float time);
    void OutPut(FILE *fp);
    void OutPutSrc( );
    void OutPutDomain(FILE *fp);
    void TimeOut();
    void SrcTimeOut();
    void DomainTimeOut();
    void OutPutDay( FILE *fp);
    void OutPutnewhouse( );
    void OutPutloupan( );
    void OutPutLouPanPair( );
    void OutPutesf();
    void OutPutesfrefertitle();
    void OutPutesfnewhouse();
    void OutPutesflpid();
    void OutPutbaidu();
};

#endif
