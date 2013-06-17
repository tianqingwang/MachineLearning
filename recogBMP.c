
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include "gif.h"

#define PI 3.1415926535898
#define MAX_FEATURE_LEN    (192)
#define NORM_WIDTH         (12)
#define NORM_HEIGHT        (16)
#define MIN_WINDOW_LEN     (4)
#define MAX_WINDOW_LEN     (11)
#define MAX_TRAINING_PAIR  (1000)
#define MAX_TRAINING_ELEM  (200)
#define MIN_SIM            (0.82)

#define DEBUG              (0)


/*global variable*/
/*in training file*/
int train_template[MAX_TRAINING_PAIR][MAX_TRAINING_ELEM];
int train_value[MAX_TRAINING_PAIR];

int train_totalPair;
int train_vectorsize;
int train_valuesize;



void recogBMP(char* filename);
unsigned int  getSlicedHeight(unsigned char *data, int left, int right, int image_width, int image_height, int *top_height, int *bot_height);
unsigned int ImageRotation(unsigned char *data, int width, int height);
//unsigned int ImageSplit(unsigned char *data, int width, int height);
unsigned int ImageProcessing(unsigned char *data, int width, int height);
unsigned int getfeatureVector(unsigned char *data, float *vector,int image_width, int image_height);
void copy_char(unsigned char *src,unsigned char *dest, int left,int right, int top, int bot, int image_width);
void PRINT_NORM(unsigned char *norm_data,int width, int height);
void PRINT_FEATURE(float *vector, int feature_len);
int IsItalic(unsigned char *data, int width, int height);

void gettemplate(FILE *fd,int num,int iNum,int oNum);
void gettemplateInfo(FILE *fd,int *num, int *iNum, int *oNum);
float getsimvalue(float *vector, int *result_index);

int main(int argc, char * argv[])
{
    char filename[256];
	int i =0;
	
	if (argc < 2){
	    printf("Usage:\n");
		printf(" ./recdigit file_to_recog\n");
		exit(0);
	}
	else{
	    FILE *fp;
		fp = fopen("feature.tpl","r");
		if (fp == NULL){
		    printf("can't open new_feature_train.set\n");
			exit(1);
			}
	    gettemplateInfo(fp,&train_totalPair,&train_vectorsize,&train_valuesize);
	    gettemplate(fp,train_totalPair,train_vectorsize,train_valuesize);
	    for (i=1; i<argc; i++){
            strcpy(filename, argv[i]);
            printf("%s\n",filename);
			
			
			
            recogBMP(filename);
		}
    }
    return 0;
}

void recogBMP(char* filename)
{
    int i = 0;
    int j = 0;
    int k = 0;
	
	int top_height,bot_height;

    /*read image head information*/
    FILE *f = fopen(filename,"rb");


    int width, height;
    ReadGIFInfo(f,&width,&height);
    unsigned char *data=(unsigned char*)malloc(width*height*sizeof(unsigned char));
    ReadGIFData(f,data,width,height);    
    
	if (IsItalic(data,width,height) == 1){
        ImageRotation(data,width,height);  /*for italic*/
    }
	  
      ImageProcessing(data,width,height);

    delete[] data;
    return;
}

/*need to add automatic angle adjusting*/
unsigned int ImageRotation(unsigned char *data, int width, int height)
{
    int angle  = 0;

    int i = 0;
    int j = 0;
    int k = 0;

    int new_x, new_y;

    unsigned char *rot_data =  (unsigned char*)malloc(width*height*sizeof(unsigned char));

    memset(rot_data,0,width*height);

//	for (angle = 0; angle <= 30; angle ++){
    /*for test*/
    angle = -12;
    /*end test*/
    for (i = 0; i < height; i++) {
        for (j = 0; j<width; j++) {
            double arc_angle = angle*PI/180.0;
            
            if (data[i*width+j] == 1) {
                int new_x = (int)(j-i*tan(arc_angle));
                int new_y = i;
				new_x += 5;
                if (new_x >=0 && new_x < width) {
                    int new_k = new_y*width + new_x;
                    rot_data[new_k] = 1;
                }
            }

        }
    }

    for (i=0; i<height; i++) {
        for (j=0; j<width; j++) {
            k=i*width+j;
            data[k] = rot_data[k];
        }
    }
//	}


    free(rot_data);
    return 0;
}

void gettemplateInfo(FILE *fd,int *num, int *iNum, int *oNum)
{
    char buffer[256] = "";
	char newbuf[200] = {0};
	
	fgets(buffer,256,fd);
	
	char *p1 = strstr(buffer," ");
	strncpy(newbuf,buffer,p1 -buffer);
	*num = atoi(newbuf);
	memset(newbuf,0,16);
	
	char *p = p1;
	while(*p == ' '){
	    p++;
	}
	
	p1 = strstr(p," ");
	
	strncpy(newbuf,p,p1-p);
	strcpy(newbuf,p);
	*iNum = atoi(newbuf);
	memset(newbuf,0,16);
	
	p = p1;
	while(*p == ' '){
	    p++;
	}
	strcpy(newbuf,p);
	*oNum = atoi(newbuf);
	memset(newbuf,0,16);
	
	return;
}


void gettemplate(FILE *fd,int num,int iNum,int oNum)
{
    char buff_f[1024];
	char buff_n[1024];
	char buff[256];
	char *p = NULL;
	int i = 0;
	
	memset(buff_f,0,1024);
	
	for (i=0; i<num; i++){
	    fgets(buff_f,1024,fd); /*feature*/
		fgets(buff_n,1024,fd); /*number*/
		
		p = buff_f;
		while((p != NULL) && (*p == ' ')){
		    p++;
		}
		
		int j=0;
		char *p1;
		while(p != '\0'){
		    
			//printf("%s\n",p1);
			p1 = strstr(p," ");
			if (p1 != NULL){
			    strncpy(buff,p,p1-p);
				train_template[i][j++] = atoi(buff);
				
				p = p1;
				while(p != '\0' && (*p == ' ') ){
				    p++;
				}
			}
			else{
			    p = p1;
			}
		}
		
		p = buff_n;
		
		j=0;
		while(p != '\0'){
		    p1 = strstr(p," ");
			int value;
			if (p1 != NULL){
			    strncpy(buff,p,p1-p);
				value = atoi(buff);
				train_value[i] = train_value[i]*2 + value;
				
				p = p1;
				while(p != '\0' && (*p == ' ')){
				    p++;
				}
			}
			else{
			   if (isdigit(*p)){
			       value = atoi(p);
			       train_value[i] = train_value[i]*2 + value;
			   }
			   p = p1;
			   
			}
		}
	}
}


float getsimvalue(float *vector,int *result_index)
{
    int i,j,k;
	float maxsim = 0;
	float sim = 0;
	
	int index = 0;
	
	float sum1 = 0;
	float sum2 = 0;
	float sum3 = 0;
	
	
	for (i=0; i<train_totalPair; i++){
	    sum1 = 0;
		sum2 = 0;
		sum3 = 0;
	    for (j=0; j<train_vectorsize; j++){
		    sum1 += train_template[i][j]*train_template[i][j];
			sum2 += vector[j]*vector[j];
			sum3 += train_template[i][j]*vector[j];
		}
		
		sim = sum3/(sqrt(sum1)*sqrt(sum2));
		
		if (sim > maxsim){
		    maxsim = sim;
			index = i;
		}
	}
	
	//printf("maxsim = %f,index=%d,train_result=%d\n",maxsim,index,train_result[index]);
	*result_index = index;
	return maxsim;
}

/*Is Italic font*/
int IsItalic(unsigned char *data, int width, int height)
{
    int i,j,k;
	int x1,x2;
    /*get the first x pos at the y=8*/

	for (i=0; i<width; i++){
		if (data[14*width + i] == 1){
		    x1=i;
			break;
		}
	}
	
	/*get the first x pos at the y=14*/

	for (i=0; i<width; i++){
	    if (data[18*width+i] == 1){
		    x2 = i;
			break;
		}
	}
	
	
	return (x1==x2)?0:1;
}



unsigned int  getSlicedHeight(unsigned char *data, int left, int right, int image_width, int image_height, int *top_height, int *bot_height)
{
    int i,j,k;
	int l_width = ((image_width*3 + 3)>>2)<<2;

	for (i=0; i<image_height; i++){
	    for (j=left; j<=right; j++){
            if (data[i*image_width + j] == 1){
			    *top_height = i;
				break;
			}
		}
	}
	
	for (i=image_height-1; i>=0; i--){
	    for (j=left; j<=right; j++){
            if(data[i*image_width + j] == 1){
			    *bot_height = i;
				break;
			}
		}
	}
	
	return (*top_height - *bot_height + 1);
}

void ImageNorm(unsigned char *data,unsigned char *norm_data,int width, int height)
{
    int i,j;
    int new_x, new_y;

    for ( i = 0; i<width; i++) {
        for (j=0; j<height; j++) {
            new_x = (int)(NORM_WIDTH*i/width);
            new_y = (int)(NORM_HEIGHT*j/height);
            if (data[j*width + i] == 0) {
                norm_data[new_y*NORM_WIDTH + new_x] = 0;
            } else {
                norm_data[new_y*NORM_WIDTH + new_x] = 1;
            }
        }
    }
}

unsigned int ImageProcessing(unsigned char *data, int width, int height)
{
    int i,j,k;
	int w,h;
	int left, right;
	int recogResult[13] = {0};
	int iResult = 0;
	
	int sliced_top, sliced_bot;
	
	int pixel_num = 0;
	
	left = right = 0;
	
	for (i=0; i<13; i++){
	    recogResult[i] = 0;
	}
	
	unsigned char *norm_data = (unsigned char*)malloc(sizeof(unsigned char)*NORM_WIDTH*NORM_HEIGHT);
	
	if (norm_data == NULL){
	    printf("(%d) allocate memory failed\n",__LINE__);
		exit(1);
	}
	
	float *feature_vector = (float*)malloc(sizeof(float)*MAX_FEATURE_LEN);
	if (feature_vector == NULL){
	    printf("(%d) allocate memory failed\n",__LINE__);
		exit(1);
	}
	
	while(left < width){
	    pixel_num = 0;
		
		for (i=0; i<height; i++){

            if (data[i*width + left] == 1){
			    pixel_num ++;
			}
		}
		
		if (pixel_num <= 1){
		    left++;
			continue;
		}
		
		int bias;
		
		float maxsim[5] = {0};
		int window_len[5] = {MAX_WINDOW_LEN};
		int vIndex[5] = {-1};
		
		for (bias = -2; bias <=2; bias ++){
		
		j = MAX_WINDOW_LEN;
		while(j>=MIN_WINDOW_LEN){
		    right = left + bias + j;
			if (right >= width){
			    right = width - 1;
				j = right - (left + bias);
			}
			
			getSlicedHeight(data,left+bias,right,width,height,&sliced_top,&sliced_bot);
#if DEBUG
			printf("left=%d,right=%d\n",left+bias,right);
			printf("sliced_top=%d,sliced_bot=%d\n",sliced_top,sliced_bot);
#endif
			unsigned char *one_ch = (unsigned char*)malloc(sizeof(unsigned char)*(right-(left+bias)+1)*(sliced_top-sliced_bot+1));
			memset(one_ch,0,(right-(left+bias)+1)*(sliced_top-sliced_bot+1));
			
			if (one_ch == NULL){
			    printf("(%d) allocate memory failed\n",__LINE__);
				exit(1);
			}
			
			copy_char(data,one_ch,left+bias,right,sliced_top,sliced_bot,width);
			memset(norm_data,0,NORM_WIDTH*NORM_HEIGHT);
			ImageNorm(one_ch,norm_data,right-(left+bias)+1, sliced_top - sliced_bot + 1);
			
			memset(feature_vector,0,MAX_FEATURE_LEN);
			getfeatureVector(norm_data,feature_vector,NORM_WIDTH,NORM_HEIGHT);
			
			int index;
			float simvalue = getsimvalue(feature_vector,&index);
			
			
#if DEBUG
            printf("index=%d,simvalue=%0.3f,recog=%d\n",index,simvalue,train_value[index]);
#endif
			
#if DEBUG
            PRINT_NORM(norm_data,NORM_WIDTH,NORM_HEIGHT);
#endif

#if DEBUG
			PRINT_FEATURE(feature_vector,MAX_FEATURE_LEN);
#endif

/*judge the output*/
#if 1
            if (simvalue >= MIN_SIM){
			    if (maxsim[bias+2] < simvalue){
				    maxsim[bias+2] = simvalue;
					window_len[bias+2] = j;
					vIndex[bias+2] = index;
				}
			}
#endif			
			free(one_ch);
			
			j--;
		}
#if DEBUG
		printf("maxsim[%d]=%0.3f\n",bias+2,maxsim[bias+2]);
#endif
	    }

        int n=0; 
		int allzero = 1;
		for (n=0; n<5; n++){
		    if (maxsim[n] != 0){
			    allzero = 0;
				break;
			}
		}
		
		if (allzero == 1){
		    left = left + 3;
		}
		else{
		    int maxindex = 0;
		    for (n=0; n<5; n++){
			    if (maxsim[n] > maxsim[maxindex]){
				    maxindex = n;
				}
			}
			
			

#if DEBUG
            printf("maxindex=%d\n",maxindex);
            printf("recogNUM=%d\n",train_value[vIndex[maxindex]]);
#endif
			
			recogResult[iResult++] = train_value[vIndex[maxindex]];
			left = left + maxindex -2 + window_len[maxindex];
			
		}


	}
	
	
	free(norm_data);
	free(feature_vector);
	
	/*output the recognized result*/
	int m = 0;
	for (m=0; m<iResult; m++){
	    printf("%d",recogResult[m]);
	}
	
	printf("\n");
}


void copy_char(unsigned char *src,unsigned char *dest, int left,int right, int top, int bot, int image_width)
{
    int i,j,k;
	
	int new_width = right - left + 1;
	
	for (i=bot; i<=top; i++){
	    for (j=left; j<=right; j++){

            if (src[i*image_width + j] == 1){
			    dest[(i-bot)*new_width + j-left] = 1;
			}
			else{
			    dest[(i-bot)*new_width + j-left] = 0;
			}
		}
	}
}

void PRINT_NORM(unsigned char *norm_data,int width, int height)
{
    int k,j;
    for (k=0; k<height; k++) {
        for (j=0; j<width; j++) {
            if (norm_data[k*width + j] == 0) {
                printf("  ");
            } else {
                printf("%d ",norm_data[k*width + j]);
            }
        }
        printf("\n");
    }
    printf("\n");
}

void PRINT_FEATURE(float *vector, int feature_len)
{
    int i = 0;
	for (i=0; i<feature_len; i++){
	    printf("%d ",(int)vector[i]);
	}
	printf("\n");
}


unsigned int getfeatureVector(unsigned char *data, float *vector,int image_width, int image_height)
{
    int i,j;
	for (i=0; i<image_height; i++){
	    for (j=0; j<image_width; j++){
		    vector[i*image_width + j] = data[i*image_width + j];
		}
	}
	
	return 0;
}
