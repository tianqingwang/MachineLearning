#ifndef __LINEAR_REGRESSION_H__
#define __LINEAR_REGRESSION_H__

#include <stdio.h>
#include <stdlib.h>

class CLinearRegression{
    CLinearRegresssion(unsigned int size);
	~CLinearRegression(void);
    
private:
    unsigned int m_size;
    float *m_vector;
}

#endif
