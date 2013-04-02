#include <memory.h>
#include <stdlib.h>
#include <time.h>
#include "hash.h"
#include "mylist2.h"
extern cmyhashvoid myhashGCookieNum ;

extern cmyhashvoid myhashloupangcookie ;
extern char datapath[50];
extern char daytime[50];
cmyhashvoid::cplex* cmyhashvoid::cplex::create(cplex*& phead, int imax, int cbelement)
{
    cplex* p = (cplex*) new char[sizeof(cplex) + imax * cbelement];
    p->pnext = phead;
    phead = p;  // change head (adds in reverse order for simplicity)
    return p;
}

void cmyhashvoid::cplex::freedatachain()
{
    cplex* p = this;

    while (p != NULL) {
        char* bytes = (char*)p;
        cplex* pnext = p->pnext;

        delete[] bytes;
        p = pnext;
    }
}

cmyhashvoid::cmyhashvoid(int ihash, int iblock, size_t ikey, size_t ivalue, int (*compar)(const void *, const void *), unsigned int (*outhashkey)(const void *key))
{
    int imod = (ikey + ivalue) % sizeof(cassoc);
    int idiv = (ikey + ivalue) / sizeof(cassoc);

    m_ihashsize = ihash;
    m_iblocksize = iblock;
    m_ikeysize = ikey;
    m_ivaluesize = ivalue;
    m_compar = compar;
    m_hashkey = outhashkey;
    if (imod)
        m_idatasize = 2 + idiv;
    else
        m_idatasize = 1 + idiv;

    m_phash = NULL;
    m_icount = 0;
    m_max_icount = 0;
    m_pfreelist = NULL;
    m_pblocks = NULL;

    //init
    m_phash = new cassoc* [m_ihashsize];
    memset(m_phash, 0, sizeof(cassoc*) * m_ihashsize);
    m_ilastkey = m_ihashsize / 2;
}

void cmyhashvoid::OutPut(FILE *fp )
{
    int iCount = 0;
    char *szBuffer = (char *)malloc(m_ikeysize + m_ivaluesize);
    cplex* p = m_pblocks;

    memset(szBuffer, 0, m_ikeysize + m_ivaluesize);

    while (p != NULL) {
        cplex* pnext = p->pnext;
        struct cassoc *ss = (struct cassoc *)p->data();

        int ii;
        for (ii=0; ii<m_iblocksize; ii++) {
            if (memcmp(szBuffer, ss->key(), m_ikeysize + m_ivaluesize) != 0) {
                fwrite(ss->key(), m_ikeysize + m_ivaluesize, 1, fp);

                iCount++;
                if (iCount >= m_icount) {
                    free(szBuffer);
                    return;
                }
            }

            ss += m_idatasize;
        }

        p = pnext;
    }

    free(szBuffer);
    return;
}

void cmyhashvoid::OutPutesfnewhouse( )
{
    int iCount = 0;
    char *szBuffer = (char *)malloc(m_ikeysize + m_ivaluesize);
    cplex* p = m_pblocks;

    memset(szBuffer, 0, m_ikeysize + m_ivaluesize);

    while (p != NULL) {
        cplex* pnext = p->pnext;
        struct cassoc *ss = (struct cassoc *)p->data();

        int ii;
        for (ii=0; ii<m_iblocksize; ii++) {
            if (memcmp(szBuffer, ss->key(), m_ikeysize + m_ivaluesize) != 0) {
                char gcookie[MAX_LENGCOOKIE +1] = "";
                memcpy( gcookie, ss->key()  , m_ikeysize);

                CHANNELCOUNT count1 = {0,0};
                memcpy( &count1, ss->value(m_ikeysize)  , m_ivaluesize);
                printf("%s\t%d\t%d\n",gcookie, count1.esfnum, count1.newhousenum );

                iCount++;
                if (iCount >= m_icount) {
                    free(szBuffer);
                    return;
                }
            }

            ss += m_idatasize;
        }

        p = pnext;
    }

    free(szBuffer);
    return;
}

void GetDomain(char *str ,char *buffer)
{

    char *p1 , *p2;
    char strurl[MAX_LENURL +1 ]="";
    strcpy(strurl,str );
    if ( p1 = strstr(strurl, "http://") ) {

        if( p2 = strchr( p1 + 7, '/' )) {
            *p2= 0;
            strncpy(buffer, p1 + 7, MAX_DOMAIN_LEN);
        }

    } else if (p1 = strstr(strurl, "https://") ) {
        if (p2 = strchr(p1 + 8 ,'/' ) ) {
            *p2 = 0;
            strncpy(buffer, p1 + 8, MAX_DOMAIN_LEN);

        }
    }

}

void cmyhashvoid::OutPutesflpid( )
{
    char strFilePath[128] = "";
    FILE * fpesfid;
    sprintf(strFilePath, "%s/%sesfid", datapath,daytime);
    fpesfid = fopen(strFilePath , "wr");



    int iCount = 0;
    char *szBuffer = (char *)malloc(m_ikeysize + m_ivaluesize);
    cplex* p = m_pblocks;

    memset(szBuffer, 0, m_ikeysize + m_ivaluesize);

    while (p != NULL) {
        cplex* pnext = p->pnext;
        struct cassoc *ss = (struct cassoc *)p->data();

        int ii;
        for (ii=0; ii<m_iblocksize; ii++) {
            if (memcmp(szBuffer, ss->key(), m_ikeysize + m_ivaluesize) != 0) {
                char esflpid[20] = "";
                memcpy( esflpid, ss->key()  , m_ikeysize);

                int num;
                memcpy( &num, ss->value(m_ikeysize)  , m_ivaluesize);
                fprintf( fpesfid ,"%s\t%d\n", esflpid, num );

                iCount++;
                if (iCount >= m_icount) {
                    free(szBuffer);
                    return;
                }
            }

            ss += m_idatasize;
        }

        p = pnext;
    }

    free(szBuffer);
    return;
}


int compare_lppair(const void *arg1, const void *arg2)
{
    LOUPANPAIR *p1 = (LOUPANPAIR *)arg1;
    LOUPANPAIR *p2 = (LOUPANPAIR *)arg2;
    if (p2->num > p1->num)
        return 1;
    return -1;
}

int compare_esftitile(const void *arg1, const void *arg2)
{
    TITLEURL *p1 = (TITLEURL *)arg1;
    TITLEURL *p2 = (TITLEURL *)arg2;
    if (p2->num > p1->num)
        return 1;
    return -1;
}


void cmyhashvoid::OutPutLouPanPair( )
{
    /*      int iCount = 0;
           char *szBuffer = (char *)malloc(m_ikeysize + m_ivaluesize);
           cplex* p = m_pblocks;

           memset(szBuffer, 0, m_ikeysize + m_ivaluesize);

           while (p != NULL)
           {
                   cplex* pnext = p->pnext;
                   struct cassoc *ss = (struct cassoc *)p->data();

                   int ii;
                   for (ii=0; ii<m_iblocksize; ii++)
                   {
                           if (memcmp(szBuffer, ss->key(), m_ikeysize + m_ivaluesize) != 0)
                           {

    			cmyhashvoid * phash;
                                   memcpy( &phash, ss->value(m_ikeysize)  , m_ivaluesize);
                                   LOUPANPAIR  aloupanpair[phash->m_icount];
                                   phash->OutPutItem(aloupanpair );
                                   //if (phash->m_icount >1 )
                                   //              printf("dddd");
                                   int arraycout = phash->m_icount;
    			qsort(aloupanpair, arraycout, sizeof(LOUPANPAIR), compare_lppair);

    			char loupan[MAX_LENLOUPAN ] = "";
    			memcpy( loupan, ss->key()  , m_ikeysize);
    			cmyhashvoid * phashgcookie =NULL ;
    			if( myhashloupangcookie.lookup(loupan,&phashgcookie) )
    			{
    				int pA =  phashgcookie->m_icount;

    				for(int j = 0; j< arraycout && j<5 ; j++ )
    				{
    					printf("%s=====%d\t%d\t%f\n ",aloupanpair[j].name ,aloupanpair[j].num ,pA , float(aloupanpair[j].num )/float(pA)  );
    				}
    			}

                                   iCount++;
                                   if (iCount >= m_icount)
                                   {
                                           free(szBuffer);
                                           return;
                                   }
                           }

                           ss += m_idatasize;
                   }

                   p = pnext;
           }

           free(szBuffer);
           return;
    */
}



void cmyhashvoid::OutPutesfrefertitle( )
{
    int iCount = 0;
    char *szBuffer = (char *)malloc(m_ikeysize + m_ivaluesize);
    cplex* p = m_pblocks;

    memset(szBuffer, 0, m_ikeysize + m_ivaluesize);

    while (p != NULL) {
        cplex* pnext = p->pnext;
        struct cassoc *ss = (struct cassoc *)p->data();

        int ii;
        for (ii=0; ii<m_iblocksize; ii++) {
            if (memcmp(szBuffer, ss->key(), m_ikeysize + m_ivaluesize) != 0) {

                cmyhashvoid * phash;
                memcpy( &phash, ss->value(m_ikeysize)  , m_ivaluesize);
                char refertitle[MAX_TITLE_LEN +1] = "";
                memcpy( refertitle, ss->key()  , m_ikeysize);
                //printf("%s\n", refertitle);
                if( strlen(refertitle)==0 ) {
                    printf("refertitle==%s\n", refertitle);
                    fflush(stdout);
                    ss += m_idatasize;
                    continue;
                }
                TITLEURL  atitleurl[phash->m_icount];
                phash->OutPutItem(atitleurl );
                //if (phash->m_icount >1 )
                //              printf("dddd");
                int arraycout = phash->m_icount;
                qsort(atitleurl, arraycout, sizeof(TITLEURL), compare_esftitile);


                int sum=0;
                for(int k =0; k <arraycout ; k++) {
                    sum+=atitleurl[k].num ;
                }


                for(int j = 0; j< arraycout && j<5 ; j++ ) {
                    printf("%s=====%s\t%d\t%d\t%f\n ",refertitle ,atitleurl[j].name ,atitleurl[j].num ,sum , float(atitleurl[j].num )/float(sum)  );
                }

                iCount++;
                if (iCount >= m_icount) {
                    free(szBuffer);
                    return;
                }
            }

            ss += m_idatasize;
        }

        p = pnext;
    }

    free(szBuffer);
    return;
}




void cmyhashvoid::OutPutloupan( )
{
    int iCount = 0;
    char *szBuffer = (char *)malloc(m_ikeysize + m_ivaluesize);
    cplex* p = m_pblocks;

    memset(szBuffer, 0, m_ikeysize + m_ivaluesize);

    while (p != NULL) {
        cplex* pnext = p->pnext;
        struct cassoc *ss = (struct cassoc *)p->data();

        int ii;
        for (ii=0; ii<m_iblocksize; ii++) {
            if (memcmp(szBuffer, ss->key(), m_ikeysize + m_ivaluesize) != 0) {
                cmyhashvoid * phash;
                memcpy( &phash, ss->value(m_ikeysize)  , m_ivaluesize);
                LOUPAN  loupanarry[phash->m_icount];
                phash->OutPutItem(loupanarry );
                //if (phash->m_icount >1 )
                //		printf("dddd");
                int arraycout = phash->m_icount;
                for(int j=0; j< arraycout && arraycout != 1 ; j++ ) {
                    for( int k = 0 ; k < arraycout && arraycout != 1 ; k++ ) {
                        if( k!=j )
                            printf("%s=====%s\n",loupanarry[j].name , loupanarry[k].name  );
                    }


                }


                iCount++;
                if (iCount >= m_icount) {
                    free(szBuffer);
                    return;
                }
            }

            ss += m_idatasize;
        }

        p = pnext;
    }

    free(szBuffer);
    return;
}



void cmyhashvoid::OutPutDay(FILE *fp)
{
    char strFilePath[128] = "";
    FILE * fpusercountall;
    sprintf(strFilePath, "%s/%susercountall", daytime, datapath);
    fpusercountall = fopen(strFilePath , "wr");

    FILE * fpusercountbaidu;
    sprintf(strFilePath, "%s/%susercountbaidu", daytime,datapath);
    fpusercountbaidu = fopen(strFilePath , "wr");

    FILE * fpusercountallbaidu;
    sprintf(strFilePath, "%s/%susercountallbaidu", daytime,datapath);
    fpusercountallbaidu = fopen(strFilePath , "wr");


    int usercount[DAYNUM ] = {0} ;
    int usercountall[DAYNUM ] = {0} ;
    int usercountbaidu[DAYNUM ]= {0};
    int usercountallbaidu[ DAYNUM] = {0};
    int iCount = 0;
    char *szBuffer = (char *)malloc(m_ikeysize + m_ivaluesize);
    cplex* p = m_pblocks;

    memset(szBuffer, 0, m_ikeysize + m_ivaluesize);

    while (p != NULL) {
        cplex* pnext = p->pnext;
        struct cassoc *ss = (struct cassoc *)p->data();

        int ii;
        for (ii=0; ii<m_iblocksize; ii++) {
            if (memcmp(szBuffer, ss->key(), m_ikeysize + m_ivaluesize) != 0) {

                DAYARRAY dayarray;
                memcpy( &dayarray, ss->value(m_ikeysize)  , m_ivaluesize);
                int i;
                for(i=0; i< DAYNUM; i++ ) { //计算每天来的原用户
                    if(dayarray.day[i] == '1' ) {
                        usercount[i]++;
                        if( strlen(dayarray.rdomain)==0 )
                            usercountbaidu[i]++;
                    }

                }
                for(i=0; i< DAYNUM; i++ ) {	//计算几天内来的原用户数
                    int j;
                    for(j=1 ; j <= i; j++ ) {
                        if(dayarray.day[j] == '1' ) {
                            usercountall[i]++;
                            if( strlen(dayarray.rdomain )==0 )
                                usercountallbaidu[i]++;
                            break;
                        }
                    }
                }

                iCount++;
                if (iCount >= m_icount) {
                    int j;
                    for ( j=0; j<DAYNUM ; j++) {
                        printf("%d,%d\n",j,usercount[j]);
                        fprintf(fp,"usercount[%d] =%d\n",j,usercount[j]);
                    }
                    printf("all---\n");
                    for (j=0; j<DAYNUM; j++	) {
                        printf("%d,%d\n",j,usercountall[j] );
                        fprintf( fpusercountall,"usercountall[%d] =%d\n",j,usercountall[j]);

                    }
                    printf("baidu---\n");
                    for (j=0; j<DAYNUM; j++ ) {
                        printf("%d,%d\n",j,usercountbaidu[j] );
                        fprintf( fpusercountbaidu,"usercountbaidu[%d] =%d\n",j,usercountbaidu[j]);

                    }
                    printf("baiduall---\n");
                    for (j=0; j<DAYNUM; j++ ) {
                        printf("%d,%d\n",j,usercountallbaidu[j] );
                        fprintf( fpusercountallbaidu,"usercountallbaidu[%d] =%d\n",j,usercountallbaidu[j]);

                    }



                    free(szBuffer);
                    return;
                }
            }

            ss += m_idatasize;
        }

        p = pnext;
    }


    free(szBuffer);
    return;
}


void cmyhashvoid::OutPutSrc( )
{

    char strFilePath[128] = "";

    FILE * fpout;
    sprintf(strFilePath,"%s/%sout.txt",datapath,daytime);
    fpout = fopen(strFilePath, "wr");

    FILE * fpoutallsplit;
    sprintf(strFilePath,"%s/%soutallsplit.txt",datapath,daytime);
    fpoutallsplit = fopen(strFilePath, "wr");


    FILE * fpreferno;
    sprintf(strFilePath, "%s/%sreferno", datapath,daytime);
    fpreferno = fopen(strFilePath , "wr");

    FILE * fpreferout;
    sprintf(strFilePath, "%s/%sreferout",  datapath,daytime);
    fpreferout = fopen(strFilePath , "wr");

    FILE * fpreferin;
    sprintf(strFilePath, "%s/%sreferin",  datapath,daytime);
    fpreferin = fopen( strFilePath , "wr");

    FILE * fpbaidu;
    sprintf(strFilePath, "%s/%sbaidu",  datapath,daytime);
    fpbaidu = fopen( strFilePath , "wr");


    FILE * fpdayfirsturl;
    sprintf(strFilePath, "%s/%sdayfirsturl", datapath,daytime);
    fpdayfirsturl = fopen( strFilePath ,"wr" );

    FILE * fpiscookie0;
    sprintf(strFilePath, "%s/%siscookie0", datapath,daytime);
    fpiscookie0 = fopen( strFilePath ,"wr" );

    FILE * fptypecommon;
    sprintf(strFilePath, "%s/%stypecommon", datapath,daytime);
    fptypecommon = fopen( strFilePath ,"wr" );

    FILE * fptypesoufun;
    sprintf(strFilePath, "%s/%stypesoufun", datapath,daytime);
    fptypesoufun = fopen( strFilePath ,"wr" );

    FILE * fptypecomp;
    sprintf(strFilePath, "%s/%stypecomp", datapath,daytime);
    fptypecomp = fopen( strFilePath ,"wr" );

    FILE * fptypepass;
    sprintf(strFilePath, "%s/%stypepass", datapath,daytime);
    fptypepass = fopen( strFilePath ,"wr" );


    FILE * fpnewact;
    sprintf(strFilePath, "%s/%sactnew", datapath,daytime);
    fpnewact = fopen( strFilePath ,"wr" );

    FILE * fpoldact;
    sprintf(strFilePath, "%s/%sactold", datapath,daytime);
    fpoldact = fopen( strFilePath ,"wr" );

    FILE * fpdayfirsturlsplit;
    sprintf(strFilePath, "%s/%sdayfirsturlsplit", datapath,daytime);
    fpdayfirsturlsplit = fopen( strFilePath ,"wr" );

    FILE * fpnewhouse;
    sprintf(strFilePath, "%s/%snewhouse", datapath,daytime);
    fpnewhouse = fopen( strFilePath ,"wr" );

    FILE * fpnoise;
    sprintf(strFilePath, "%s/%snoise", datapath,daytime);
    fpnoise = fopen( strFilePath ,"wr" );

    FILE * fpmzero;
    sprintf(strFilePath, "%s/%smzero", datapath,daytime);
    fpmzero = fopen( strFilePath ,"wr" );



    int newact = 0;
    int oldact = 0;
    int typecommon = 0;
    int typesoufun = 0;
    int typecomp = 0;
    int typepass = 0;

    int iCount = 0;
    char *szBuffer = (char *)malloc(m_ikeysize + m_ivaluesize);
    cplex* p = m_pblocks;

    memset(szBuffer, 0, m_ikeysize + m_ivaluesize);

    while (p != NULL) {
        cplex* pnext = p->pnext;
        struct cassoc *ss = (struct cassoc *)p->data();

        int ii;
        for (ii=0; ii<m_iblocksize; ii++) {
            if (memcmp(szBuffer, ss->key(), m_ikeysize + m_ivaluesize) != 0) {
                //fwrite(ss->key(), m_ikeysize + m_ivaluesize, 1, fp);
                cmemalloc list;
                memcpy( &list, ss->value(m_ikeysize)  , m_ivaluesize);
                MY_LIST *p = list.m_pend;  		//用户链表尾节点
                char strLine [MAX_LENLINE + 1] = "";
                char strLinesplit[2*MAX_LENLINE +1 ] ="";
                do {
                    LOG_INFO        sLogInfo;
                    sLogInfo = p->sloginfo;

                    char domain [MAX_DOMAIN_LEN +1]="";
                    GetDomain(sLogInfo.refer,domain );  //refer的域名

                    sprintf(strLine,"%s^%s^l=%s^r=%s^g=%s^u=%s^c=%c^a=%c^s=%s^m=%d^t=^i=%s^%ld",
                            sLogInfo.ip ,sLogInfo.severtime, sLogInfo.local, sLogInfo.refer,
                            sLogInfo.gcookie, sLogInfo.ucookie, sLogInfo.iscookie, sLogInfo.isnewact,
                            sLogInfo.scheck ,sLogInfo.mousenum , sLogInfo.struinfo, sLogInfo.time );

                    char refertitle[MAX_TITLE_LEN +1] = "";

                    MY_LIST *pforward = p;  //refer的title
                    while( pforward->pnext !=NULL ) {
                        if( !strcmp(sLogInfo.refer,pforward->pnext->sloginfo.local) ) {
                            strcpy(refertitle,pforward->pnext->sloginfo.title);
                            break;
                        }
                        pforward =  pforward->pnext ;
                    }
                    sprintf(strLinesplit,"%s\t%s\t%s\t%s\t%s\t%s\t%s=====%s",
                            sLogInfo.gcookie, domain , sLogInfo.localchannel, sLogInfo.refercharnel, sLogInfo.local, sLogInfo.refer, sLogInfo.title, refertitle);

                    if(sLogInfo.iscookie == '0' ) { //iscooie 等于0
                        fprintf(fpiscookie0 ,"%s\n",strLine);
                        p = p->pnext;
                        continue;
                    }
                    if(list.m_sum >800 ) { //去噪
                        fprintf(fpnoise, "%s\n" ,strLine );
                        p = p->pnext;
                        continue;
                    }
                    /*		if(list.m_zero >10 ) //去噪
                    		{
                    			fprintf(fpmzero, "%s\n" ,strLine );
                                                        p = p->pnext;
                                                        continue;

                    		}
                    */


                    if( strcmp(sLogInfo.localchannel, "新房") && strstr(sLogInfo.title, "楼盘详情" ) ) {

                        fprintf( fpnewhouse,"%s\n",strLine );

                    }

                    if(p->pnext ==NULL  ) { //等于NULL 的时候是用户访问第一条记录

                        if(sLogInfo.isnewact =='0' || sLogInfo.isnewact =='g' ) { // 老用户
                            oldact++;
                            fprintf(fpoldact,"%s\t%d\n",strLinesplit ,list.m_sum);

                        } else if( sLogInfo.isnewact =='1'|| sLogInfo.isnewact =='s') { //新用户
                            newact++;
                            fprintf(fpnewact,"%s\t%d\n",strLinesplit ,list.m_sum);

                        }

                        if(sLogInfo.userinfo.type ==0 ) { // 普通用户
                            typecommon++;
                            fprintf(fptypecommon,"%s\t%d\n",strLinesplit ,list.m_sum);
                        } else if(sLogInfo.userinfo.type == 100 ) { //公司内部用户
                            typesoufun++;
                            fprintf(fptypesoufun,"%s\t%d\n",strLinesplit ,list.m_sum);

                        } else if(sLogInfo.userinfo.type == 2 ) { //企业用户
                            typecomp++;
                            fprintf(fptypecomp,"%s\t%d\n",strLinesplit ,list.m_sum );

                        } else if(sLogInfo.userinfo.type == 1 ) { //通行证用户
                            typepass++;
                            fprintf(fptypepass,"%s\t%d\n",strLinesplit ,list.m_sum);

                        }

                        int lenrefer = strlen(sLogInfo.refer);
                        if (lenrefer==0) {	// 没有refer
                            fprintf(fpreferno,"%s\t%d\n",strLinesplit , list.m_sum );
                        } else {
                            if( strstr( domain ,"soufun" ) ) { // refer是soufun
                                fprintf(fpreferin,"%s\t%d\n",strLinesplit ,list.m_sum);
                            } else {			 //refer是外部
                                if( strstr(domain,"baidu") ) { //refer是baidu
                                    MY_LIST *pbaidu = list.m_pend;
                                    do {
                                        fprintf(fpbaidu, "%s======%s=====%s",pbaidu->sloginfo.localchannel,pbaidu->sloginfo.local,pbaidu->sloginfo.refer);
                                        if( pbaidu->pnext != NULL )
                                            fprintf(fpbaidu,"\n" );
                                        pbaidu = pbaidu->pnext;
                                    } while( pbaidu !=NULL);
                                    fprintf(fpbaidu, "^=====%d\n\n", list.m_sum);

                                }
                                fprintf(fpreferout,"%s\n", strLinesplit , list.m_sum);
                            }
                        }

                        fprintf(fpdayfirsturlsplit, "%s\t%d\n" ,strLinesplit, list.m_sum );
                        fprintf(fpdayfirsturl, "%s\n", strLine);	//输出每天访问的第一个日志
                    }

                    long int difftime=0;	//计算页面间访问时间间隔,单位秒
                    if(p->pnext !=NULL ) {
                        difftime = (p->sloginfo.time - p->pnext->sloginfo.time)/1000 ;
                    }
                    //按用户输出全部访问日志
                    fprintf(fpout,"%s\n", strLine );
                    //按用户输出全部访问的频道
                    fprintf(fpoutallsplit, "%s\n",strLinesplit);

                    p = p->pnext;

                } while( p !=NULL);

                fprintf(fpout,"-----------%d\n\n",list.m_sum); //输出用户访问页面数
                //fprintf(fpoutallsplit ,"-----------%d\n\n",list.m_sum); //输出用户访问页面数

                iCount++;
                if (iCount >= m_icount) {
                    printf ("newact=%d\n",newact );
                    printf ("oldact=%d\n",oldact );
                    printf ("typecommon =%d\n", typecommon  );
                    printf ("typesoufun =%d\n", typesoufun  );
                    printf ("typecomp =%d\n",typecomp );
                    printf ("typepass =%d\n",typepass );

                    free(szBuffer);
                    return;
                }
            }

            ss += m_idatasize;
        }

        p = pnext;
    }

    free(szBuffer);
    return;

}





void cmyhashvoid::OutPutnewhouse( )
{
    char strFilePath[128] = "";


    FILE * fpnewhouse;
    sprintf(strFilePath, "%s/%snewhouse", datapath,daytime);
    fpnewhouse = fopen( strFilePath ,"wr" );


    int iCount = 0;
    char *szBuffer = (char *)malloc(m_ikeysize + m_ivaluesize);
    cplex* p = m_pblocks;

    memset(szBuffer, 0, m_ikeysize + m_ivaluesize);

    while (p != NULL) {
        cplex* pnext = p->pnext;
        struct cassoc *ss = (struct cassoc *)p->data();

        int ii;
        for (ii=0; ii<m_iblocksize; ii++) {
            if (memcmp(szBuffer, ss->key(), m_ikeysize + m_ivaluesize) != 0) {
                //fwrite(ss->key(), m_ikeysize + m_ivaluesize, 1, fp);
                cmemalloc list;
                memcpy( &list, ss->value(m_ikeysize)  , m_ivaluesize);
                MY_LIST *p = list.m_pend;               //用户链表尾节点
                char strLine [MAX_LENLINE + 1] = "";
                char strLinesplit[2*MAX_LENLINE +1 ] ="";
                do {
                    LOG_INFO        sLogInfo;
                    sLogInfo = p->sloginfo;

                    char domain [MAX_DOMAIN_LEN +1]="";
                    GetDomain(sLogInfo.refer,domain );  //refer的域名

                    sprintf(strLine,"%s^%s^l=%s^r=%s^g=%s^u=%s^c=%c^a=%c^s=%s^m=%d^t=%s^i=%s^%ld",
                            sLogInfo.ip ,sLogInfo.severtime, sLogInfo.local, sLogInfo.refer,
                            sLogInfo.gcookie, sLogInfo.ucookie, sLogInfo.iscookie, sLogInfo.isnewact,
                            sLogInfo.scheck ,sLogInfo.mousenum,sLogInfo.title , sLogInfo.struinfo, sLogInfo.time );

                    if(sLogInfo.iscookie == '0' ) {
                        //  fprintf(fpiscookie0 ,"%s\n",strLine);
                        p = p->pnext;
                        continue;
                    }
                    if(list.m_sum >800 ) { //去噪
                        //       fprintf(fpnoise, "%s\n" ,strLine );
                        p = p->pnext;
                        continue;
                    }
                    /*      if(list.m_zero >10 ) //去噪
                            {
                              //      fprintf(fpmzero, "%s\n,strLine" );
                                    p = p->pnext;
                                    continue;

                            }
                    				 */
                    if( ( !strcmp(sLogInfo.localchannel, "城市首页") || !strcmp(sLogInfo.localchannel, "新房") )   && strstr(sLogInfo.title, "楼盘详情" ) ) {

                        fprintf( fpnewhouse,"%s\n",strLine );

                    }


                    p = p->pnext;

                } while( p !=NULL);


                iCount++;
                if (iCount >= m_icount) {
                    free(szBuffer);
                    return;
                }
            }

            ss += m_idatasize;
        }

        p = pnext;
    }

    free(szBuffer);
    return;


}

void cmyhashvoid::OutPutbaidu( )
{
    char strFilePath[128] = "";

    int iCount = 0;
    char *szBuffer = (char *)malloc(m_ikeysize + m_ivaluesize);
    cplex* p = m_pblocks;

    memset(szBuffer, 0, m_ikeysize + m_ivaluesize);

    while (p != NULL) {
        cplex* pnext = p->pnext;
        struct cassoc *ss = (struct cassoc *)p->data();

        int ii;
        for (ii=0; ii<m_iblocksize; ii++) {
            if (memcmp(szBuffer, ss->key(), m_ikeysize + m_ivaluesize) != 0) {
                //fwrite(ss->key(), m_ikeysize + m_ivaluesize, 1, fp);
                cmemalloc list;
                memcpy( &list, ss->value(m_ikeysize)  , m_ivaluesize);
                MY_LIST *p = list.m_pend;               //用户链表尾节点
                char strLine [MAX_LENLINE + 1] = "";
                char strLinesplit[2*MAX_LENLINE +1 ] ="";
                do {
                    LOG_INFO        sLogInfo;
                    sLogInfo = p->sloginfo;


                    sprintf(strLine,"%s^%s^l=%s^r=%s^g=%s^u=%s^c=%c^a=%c^s=%s^m=%d^t=%s^i=%s^%ld",
                            sLogInfo.ip ,sLogInfo.severtime, sLogInfo.local, sLogInfo.refer,
                            sLogInfo.gcookie, sLogInfo.ucookie, sLogInfo.iscookie, sLogInfo.isnewact,
                            sLogInfo.scheck ,sLogInfo.mousenum,sLogInfo.title , sLogInfo.struinfo, sLogInfo.time );

                    char loupan[MAX_LENLOUPAN ] = "";
                    char strgcookie[MAX_LENGCOOKIE + 1] ="";
                    memcpy( strgcookie, ss->key()  , m_ikeysize);
                    int num=0;
                    if( myhashGCookieNum.lookup(strgcookie,&num) ) {
                        printf("%s\t%d\n",strLine,num );
                    }
                    p = p->pnext;

                } while( p !=NULL);


                iCount++;
                if (iCount >= m_icount) {
                    free(szBuffer);
                    return;
                }
            }

            ss += m_idatasize;
        }

        p = pnext;
    }

    free(szBuffer);
    return;


}



void cmyhashvoid::OutPutesf( )
{
    char strFilePath[128] = "";


    FILE * fpesf;
    sprintf(strFilePath, "%s/%sesf", datapath,daytime);
    fpesf = fopen( strFilePath ,"wr" );


    int iCount = 0;
    char *szBuffer = (char *)malloc(m_ikeysize + m_ivaluesize);
    cplex* p = m_pblocks;

    memset(szBuffer, 0, m_ikeysize + m_ivaluesize);

    while (p != NULL) {
        cplex* pnext = p->pnext;
        struct cassoc *ss = (struct cassoc *)p->data();

        int ii;
        for (ii=0; ii<m_iblocksize; ii++) {
            if (memcmp(szBuffer, ss->key(), m_ikeysize + m_ivaluesize) != 0) {
                //fwrite(ss->key(), m_ikeysize + m_ivaluesize, 1, fp);
                cmemalloc list;
                memcpy( &list, ss->value(m_ikeysize)  , m_ivaluesize);
                MY_LIST *p = list.m_pend;               //用户链表尾节点
                char strLine [MAX_LENLINE + 1] = "";
                char strLinesplit[2*MAX_LENLINE +1 ] ="";
                do {
                    LOG_INFO        sLogInfo;
                    sLogInfo = p->sloginfo;

                    char domain [MAX_DOMAIN_LEN +1]="";
                    GetDomain(sLogInfo.refer,domain );  //refer的域名

                    sprintf(strLine,"%s^%s^l=%s^r=%s^g=%s^u=%s^c=%c^a=%c^s=%s^m=%d^t=%s^i=%s^%ld",
                            sLogInfo.ip ,sLogInfo.severtime, sLogInfo.local, sLogInfo.refer,
                            sLogInfo.gcookie, sLogInfo.ucookie, sLogInfo.iscookie, sLogInfo.isnewact,
                            sLogInfo.scheck ,sLogInfo.mousenum,sLogInfo.title , sLogInfo.struinfo, sLogInfo.time );

                    char refertitle[MAX_TITLE_LEN +1] = "";
                    MY_LIST *pforward = p;  //refer的title
                    while( pforward->pnext !=NULL ) {
                        if( !strcmp(sLogInfo.refer,pforward->pnext->sloginfo.local) ) {
                            strcpy(refertitle,pforward->pnext->sloginfo.title);
                            break;
                        }
                        pforward =  pforward->pnext ;
                    }
                    sprintf(strLinesplit,"%s=====%s=====%s==%s",sLogInfo.gcookie,
                            refertitle, sLogInfo.title,sLogInfo.local );




                    if(sLogInfo.iscookie == '0' ) {
                        //  fprintf(fpiscookie0 ,"%s\n",strLine);
                        p = p->pnext;
                        continue;
                    }
                    if(list.m_sum >800 ) { //去噪
                        //       fprintf(fpnoise, "%s\n" ,strLine );
                        p = p->pnext;
                        continue;
                    }
                    /*      if(list.m_zero >10 ) //去噪
                            {
                              //      fprintf(fpmzero, "%s\n,strLine" );
                                    p = p->pnext;
                                    continue;

                            }
                     */
                    if(  !strcmp(sLogInfo.localchannel, "二手房")  && strstr(sLogInfo.local, "chushou" ) &&
                         ( strstr(sLogInfo.refer,"house/" )  || strstr(sLogInfo.refer,"house-" )  )  && sLogInfo.userinfo.type!=2 && sLogInfo.userinfo.type!=100 ) {

                        fprintf( fpesf,"%s\n",strLinesplit );

                    }


                    p = p->pnext;

                } while( p !=NULL);


                iCount++;
                if (iCount >= m_icount) {
                    free(szBuffer);
                    return;
                }
            }

            ss += m_idatasize;
        }

        p = pnext;
    }

    free(szBuffer);
    return;


}




void cmyhashvoid::TimeOut()
{
}

void cmyhashvoid::SrcTimeOut()
{
}

void cmyhashvoid::DomainTimeOut()
{
}

inline unsigned int cmyhashvoid::hashkey(const void* key) const
{
    unsigned int ihash = 0;
    char *pkey = (char *)key;

    for (size_t ii=0; ii<m_ikeysize; ii++)
        ihash = (ihash<<5) + ihash + *pkey++;

    return ihash;
    /*
    	int i,l;
    	unsigned int ret=0;
    	unsigned short *s;

    	l = m_ikeysize / 2;
    	s = (unsigned short *)key;
    	for (i=0; i<l; i++)
    		ret^=(s[i]<<(i&0x0f));
    	return(ret);
    */

    /*
    	const char *pkey = (const char *)key;
    	const char *pend = pkey + m_ikeysize;
    	unsigned int ihash;
    	for (ihash = 0; pkey < pend; pkey++)
    	{
    		ihash *= 16777619;
    		ihash ^= (unsigned int) *(unsigned char*)pkey;
    	}
    */
}


void cmyhashvoid::removeall()
{
    if (m_phash != NULL) {
        // free hash table
        delete[] (m_phash);
        m_phash = NULL;
    }

    m_pfreelist = NULL;
    m_pblocks->freedatachain();
    m_icount = 0;
    m_max_icount = 0;
    m_pblocks = NULL;
}

void cmyhashvoid::recreate()
{
    removeall();
    m_phash = new cassoc* [m_ihashsize];
    memset(m_phash, 0, sizeof(cassoc*) * m_ihashsize);
}

void cmyhashvoid::timeoutcheck()
{
}


cmyhashvoid::~cmyhashvoid()
{
    removeall();
}

uint cmyhashvoid::AddClicks(const void* key, int iClickCount)
{
}

uint cmyhashvoid::SetClicks(const void* key, int iClickCount)
{
}


int cmyhashvoid::setatidf(const void* key, void* newvalue)
{
}

void cmyhashvoid::OutPutItem(void *pItem)
{
    int iCount = 0;
    char *szBuffer = (char *)malloc(m_ikeysize + m_ivaluesize);
    cplex* p = m_pblocks;

    memset(szBuffer, 0, m_ikeysize + m_ivaluesize);

    while (p != NULL) {
        cplex* pnext = p->pnext;
        struct cassoc *ss = (struct cassoc *)p->data();

        int ii;
        for (ii=0; ii<m_iblocksize; ii++) {
            if (memcmp(szBuffer, ss->key(), m_ikeysize + m_ivaluesize) != 0) {
                //if (iCount < MAX_BUDDY_NUM)
                {
                    memcpy(pItem, ss->key(), m_ikeysize + m_ivaluesize);
                    pItem = (char *)pItem + m_ikeysize + m_ivaluesize;
                }

                iCount++;
                if (iCount >= m_icount) {
                    free(szBuffer);
                    return;
                }
            }

            ss += m_idatasize;
        }

        p = pnext;
    }

    free(szBuffer);
    return;
}


int cmyhashvoid::incsetat(const void* key)
{
    register unsigned int ihash;
    register cassoc* passoc;
    if ((passoc = getassocat(key, ihash)) == NULL) {
        passoc = newassoc();

        memcpy(passoc->key(), key, m_ikeysize);

        uint newvalue =  1;
        memcpy(passoc->value(m_ikeysize), &newvalue, sizeof(uint));
        passoc->pnext = m_phash[ihash];
        m_phash[ihash] = passoc;
        return 1;
    }

    (*(uint *)passoc->value(m_ikeysize))++;
    return *(uint *)passoc->value(m_ikeysize);
}



int cmyhashvoid::setat(const void* key, void* newvalue)
{
    register unsigned int ihash;
    register cassoc* passoc;
    if ((passoc = getassocat(key, ihash)) == NULL) {
        passoc = newassoc();

        memcpy(passoc->key(), key, m_ikeysize);
        memcpy(passoc->value(m_ikeysize), newvalue, m_ivaluesize);
        passoc->pnext = m_phash[ihash];
        m_phash[ihash] = passoc;
        return 0;
    }

    memcpy(passoc->value(m_ikeysize), newvalue, m_ivaluesize);
    return 1;
}

#ifdef DEBUG
int cmyhashvoid::setat_Time(const void* key, float time)
{
    register unsigned int ihash;
    register cassoc* passoc;
    if ((passoc = getassocat(key, ihash)) == NULL) {
        passoc = newassoc();

        memcpy(passoc->key(), key, m_ikeysize);

        TEST_TIME *pTime = (TEST_TIME *)passoc->value(m_ikeysize);

        pTime->iTotalCount = 1;
        pTime->fTotalTime = pTime->minTime = pTime->maxTime = time;
        passoc->pnext = m_phash[ihash];

        m_phash[ihash] = passoc;
        return 0;
    }

    TEST_TIME *pTime = (TEST_TIME *)passoc->value(m_ikeysize);

    pTime->iTotalCount++;
    pTime->fTotalTime += time;
    if (time < pTime->minTime)
        pTime->minTime = time;
    else if (time > pTime->maxTime)
        pTime->maxTime = time;

    return 1;
}
#endif

int cmyhashvoid::resetat(const void* key, void* newvalue)
{
    unsigned int ihash;
    cassoc* passoc;
    if ((passoc = getassocat(key, ihash)) == NULL) {
        passoc = newassoc();

        memcpy(passoc->key(), key, m_ikeysize);
        memcpy(passoc->value(m_ikeysize), newvalue, m_ivaluesize);
        passoc->pnext = m_phash[ihash];
        m_phash[ihash] = passoc;
        return 0;
    }

    memcpy(passoc->value(m_ikeysize), newvalue, m_ivaluesize);
    return 1;
}

cmyhashvoid::cassoc* cmyhashvoid::newassoc()
{
    if (m_pfreelist == NULL) {
        cplex* newblock = cplex::create(m_pblocks, m_iblocksize,
                                        m_idatasize * sizeof(cmyhashvoid::cassoc));
        cmyhashvoid::cassoc* passoc = (cmyhashvoid::cassoc*) newblock->data();
        memset((char *)passoc, 0, m_iblocksize * m_idatasize
               * sizeof(cmyhashvoid::cassoc));
        passoc += (m_iblocksize - 1) * (m_idatasize);
        for (int i = m_iblocksize - 1; i >= 0; i--) {
            passoc->pnext = m_pfreelist;
            m_pfreelist = passoc;
            passoc -= m_idatasize;
        }
    }

    cmyhashvoid::cassoc* passoc = m_pfreelist;
    m_pfreelist = m_pfreelist->pnext;
    m_icount++;
    m_max_icount++;

    return passoc;
}

void cmyhashvoid::freeassoc(cmyhashvoid::cassoc* passoc)
{
    if (passoc == NULL)
        return;

    memset(passoc->key(), 0, (m_idatasize - 1) * sizeof(cmyhashvoid::cassoc));
    passoc->pnext = m_pfreelist;
    m_pfreelist = passoc;
    m_icount--;

    //if (m_icount == 0) 	removeall();
}

inline cmyhashvoid::cassoc* cmyhashvoid::getassocat(const void* key, unsigned int& ihash) const
// find association (or return NULL)
{
    if (m_hashkey == NULL)
        ihash = hashkey(key) % m_ihashsize;
    else
        ihash = (*m_hashkey)(key) % m_ihashsize;

    // see if it exists
    register cassoc* passoc;
    for (passoc = m_phash[ihash]; passoc != NULL; passoc = passoc->pnext) {
        if (m_compar == NULL) {
            if (memcmp(passoc->key(), key, m_ikeysize) == 0)
                return passoc;
        } else {
            if ((*m_compar)(passoc->key(), key))
                return passoc;
        }
    }
    return NULL;
}

cmyhashvoid::cassoc* cmyhashvoid::getassocat_lookup(const void* key, unsigned int& ihash) const
// find association (or return NULL)
{
    if (m_hashkey == NULL)
        ihash = hashkey(key) % m_ihashsize;
    else
        ihash = (*m_hashkey)(key) % m_ihashsize;

    // see if it exists
    cassoc* passoc;
    cassoc* prevpassoc;
    for (passoc = prevpassoc = m_phash[ihash]; passoc != NULL; passoc = passoc->pnext) {
        if (m_compar == NULL) {
            if (memcmp(passoc->key(), key, m_ikeysize) == 0)
                break;
        } else {
            if ((*m_compar)(passoc->key(), key))
                break;
        }

        prevpassoc = passoc;
    }

    if (prevpassoc != NULL && passoc != NULL) {
        if (prevpassoc == passoc) {
            m_phash[ihash] = passoc->pnext;
            passoc->pnext = NULL;
        } else {
            prevpassoc->pnext = passoc->pnext;
            passoc->pnext = NULL;
        }
    }

    return passoc;
}

bool cmyhashvoid::Delete(const void* key)
{
    unsigned int ihash = 0;
    cassoc* passoc = getassocat_lookup(key, ihash);
    if (passoc == NULL)	return false;

    //memcpy(value, passoc->value(m_ikeysize), m_ivaluesize);
    freeassoc(passoc);
    return true;
}

bool cmyhashvoid::lookup(const void* key, void* value)
{
    unsigned int ihash = 0;
    cassoc* passoc = getassocat(key, ihash);
    if (passoc == NULL)	return false;

    memcpy(value, passoc->value(m_ikeysize), m_ivaluesize);
    return true;
}

bool cmyhashvoid::lookup(const void* key)
{
    unsigned int ihash = 0;
    cassoc* passoc = getassocat(key, ihash);
    if (passoc == NULL)	return false;
    return true;
}

inline static int compare(const void *arg1, const void *arg2)
{
    return *(int *)arg1 == *(int *)arg2;;
}

inline static unsigned int outhash(const void *key)
{
    return *(unsigned int *)key;
}
