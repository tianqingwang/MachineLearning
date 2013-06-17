
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
//#include "floatfann.h"
#include "gif.h"

#define PI 3.1415926535898
#define MAX_FEATURE_LEN    (192)
#define NORM_WIDTH         (12)
#define NORM_HEIGHT        (16)
#define MAX_CHAR_WIDTH     (11)
#define WINDOW_STEP        (3)
#define MIN_WINDOW_LEN     (4)
#define MAX_WINDOW_LEN     (10)
#define ANN_OUTPUT_NUM     (4)
#define MAX_TRAINING_PAIR  (1000)
#define MAX_TRAINING_ELEM  (200)
#define MIN_SIM            (0.81)

#define DEBUG              (1)


/*global variable*/
/*in training file*/
int train_template[MAX_TRAINING_PAIR][MAX_TRAINING_ELEM];
int train_value[MAX_TRAINING_PAIR];

int train_totalPair;
int train_vectorsize;
int train_valuesize;



void recogBMP(char* filename);
unsigned int  getImageWidth(unsigned char *data,int width,int height, int *left_pos, int *right_pos);
unsigned int  getImageHeight(unsigned char *data,int width,int height,int *top_height,int *bot_height);
unsigned int  getSlicedHeight(unsigned char *data, int left, int right, int image_width, int image_height, int *top_height, int *bot_height);
unsigned int  getSlicedWidth(unsigned char *data, int left, int right, int image_width, int image_height, int *left_pos, int *right_pos);
unsigned int ImageRotation(unsigned char *data, int width, int height);
//unsigned int ImageSplit(unsigned char *data, int width, int height);
unsigned int ImageProcessing(unsigned char *data, int width, int height);
unsigned int ImageThinning(unsigned char *data, int width, int height);
unsigned int getfeatureVector(unsigned char *data, float *vector,int image_width, int image_height);
unsigned int recogDigital(float *vector,float *calc_out);
void set_verticalbar_blue(unsigned char *data, int width, int height, int leftpos, int rightpos);
void set_verticalbar_red(unsigned char *data, int width, int height, int leftpos, int rightpos);
void copy_char(unsigned char *src,unsigned char *dest, int left,int right, int top, int bot, int image_width);
void PRINT_NORM(unsigned char *norm_data,int width, int height);
void PRINT_FEATURE(float *vector, int feature_len);
int recog_one_sliced(unsigned char *data,int sliced_left, int sliced_right, int top_height, int bot_height, int image_width,int image_height);
unsigned int IsHorizonLine(unsigned char *data, int left,int right,int image_width,int image_height);
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
//		fp = fopen("./train/new_feature_train.set","r");
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
#if 0
    unsigned char info[54];
    fread(info, sizeof(unsigned char), 54, f); // read the 54-byte header

    int width = *(int*)&info[18];
    int height = *(int*)&info[22];

    /*read image data*/
    int l_width = ((width*3 + 3)>>2)<<2; /*alignment*/
    int size =l_width*height;

    unsigned char* data = new unsigned char[size];
    fread(data,sizeof(unsigned char),size,f);
    fclose(f);

    /*gray picture*/
    for (i=0; i<height; i++) {
        for (j=0; j<width; j++) {
            k = i*l_width+ j*3;
            unsigned char tmp = data[k];
            data[k] = data[k+2];
            data[k+2] = tmp;

            int gray = (int)(((int)data[k])*30 + ((int)data[k+1])*59 + ((int)data[k+2])*11 + 50)/100;
            if (gray >= 230) {/*white color*/
                data[k] = 255;
                data[k+1] = 255;
                data[k+2] = 255;
            } else { /*black color*/
                data[k] = 0;
                data[k+1] = 0;
                data[k+2] = 0;
            }
        }
    }
#endif

    int width, height;
    ReadGIFInfo(f,&width,&height);
    unsigned char *data=(unsigned char*)malloc(width*height*sizeof(unsigned char));
    ReadGIFData(f,data,width,height);    
    
	if (IsItalic(data,width,height) == 1){
        ImageRotation(data,width,height);  /*for italic*/
    }
	  
      ImageProcessing(data,width,height);
#if 0
#if DEBUG
    printf("width    :%d\n",width);
    printf("height   :%d\n",height);

    FILE *fo = fopen("b.bmp","wb");

    fwrite(info,sizeof(unsigned char),54,fo);
    fwrite(data,sizeof(unsigned char),size,fo);
    
    fclose(fo);
#endif
#endif
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


unsigned int  getImageWidth(unsigned char *data,int width,int height,int *left_pos, int *right_pos)
{
    int left_border  = 0;
    int right_border = width;
    int i = 0;
    int j = 0;
    int k = 0;
    int l_width = ((width*3 + 3)>>2)<<2;
    /*scan from left to right*/

    for (i=0; i<width; i++) {
        for (j=0; j<height; j++) {
            k=j*l_width + i*3;
            if (data[k] == 0 && data[k+1] == 0 && data[k + 2] == 0) {
			    left_border = i;
				break;
            }
        }
    }
#if 0
    /*draw vertical*/
    for (j = 0; j<height; j++) {
        data[j*l_width + (left_border*3)] = 0;
        data[j*l_width + (left_border*3) + 1] = 0;
        data[j*l_width + (left_border*3) + 2] = 255;
    }
#endif
    printf("left_border:%d\n",left_border);
    /*scan from right to left*/
    for (i=width; i>=0; i--) {
        for (j=0; j<height; j++) {
            k=j*l_width + 3*i;
            if (data[k-1] == 0 && data[k -2] == 0 && data[k -3] == 0) {
                right_border = i; 
				break;
            }
        }
    }

#if 0
    /*draw vertical*/
    for (j = 0; j<height; j++) {
        data[j*l_width + (right_border*3)] = 0;
        data[j*l_width + (right_border*3) + 1] = 255;
        data[j*l_width + (right_border*3) + 2] = 0;
    }
#endif
    printf("right_border:%d\n",right_border);

	*left_pos = left_border;
	*right_pos = right_border;
	
    return (*right_pos - *left_pos + 1);
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

/*check the symbol "-" in digital string*/
unsigned int IsHorizonLine(unsigned char *data, int left,int right,int image_width,int image_height)
{
    /*check top_height and bot_height*/
	int l_width  = ((image_width*3 + 3)>>2)<<2;
	int i,j,k;
	
	int top_height = 0;
	int bot_height = 0;
	
	/*scan from bottom to top*/
	for (i=0; i<image_height; i++){
	    for (j=left; j<=right; j++){
		    k = i*l_width + 3*j;
			if (data[k] == 0 && data[k+1] == 0 && data[k+2] == 0){
			    top_height = i;
				break;
			}
		}
	}
	
	/*scan from top to bottom*/
	for (i=image_height -1; i>=0; i--){
	    for (j=left; j<=right; j++){
		    k = i*l_width + 3*j;
			if (data[k] == 0 && data[k+1] == 0 && data[k+2] == 0){
			    bot_height = i;
				break;
			}
		}
	}
	
	
	return (top_height - bot_height)<=2? 1 : 0;
}

unsigned int getImageHeight(unsigned char *data,int width,int height,int *top_height,int *bot_height)
{
    unsigned int top_border     = height;
    unsigned int bottom_border  = 0;
    int i = 0;
    int j = 0;
    int k = 0;

    int l_width = ((width*3 + 3)>>2)<<2;

    /*scan from bottom to top*/
    for (i=0; i<height; i++) {
        for (j=0; j<width; j++) {
            k = i*l_width + 3*j;
            if (data[k] == 0 && data[k + 1] == 0 && data[k + 2] == 0) {
                top_border = i;
                break;
            }
        }
    }

    printf("top_border:%d\n",top_border);
#if 0
    /*draw horizon*/
    for (i=0; i<width; i++) {
        data[top_border*l_width + i*3] = 200;
        data[top_border*l_width + i*3 + 1] = 155;
        data[top_border*l_width + i*3 + 2] = 0;
    }
#endif
    /*scan from top to bottom*/
    for (i=height-1; i>=0; i--) {
        for (j=0; j<width; j++) {
            k = i*l_width + j*3;
            if (data[k] == 0 && data[k + 1] == 0 && data[k + 2] == 0) {
                bottom_border = i;
                break;
            }
        }
    }

    printf("bottom_border:%d\n",bottom_border);
#if 0
    /*draw horizon*/
    for (i=0; i<width; i++) {
        data[bottom_border*l_width + i*3] = 200;
        data[bottom_border*l_width + i*3 + 1] = 155;
        data[bottom_border*l_width + i*3 + 2] = 0;
    }
#endif	
	*top_height = top_border;
	*bot_height = bottom_border;

    return (top_border - bottom_border + 1);
}

unsigned int  getSlicedHeight(unsigned char *data, int left, int right, int image_width, int image_height, int *top_height, int *bot_height)
{
    int i,j,k;
	int l_width = ((image_width*3 + 3)>>2)<<2;

	for (i=0; i<image_height; i++){
	    for (j=left; j<=right; j++){
//		    k = i*l_width + 3*j;
//			if (data[k] == 0 && data[k+1] == 0 && data[k+2] == 0){
//			    *top_height = i;
//				break;
//			}
            if (data[i*image_width + j] == 1){
			    *top_height = i;
				break;
			}
		}
	}
	
	for (i=image_height-1; i>=0; i--){
	    for (j=left; j<=right; j++){
//		    k = i*l_width + 3*j;
//			if (data[k] == 0 && data[k+1] == 0 && data[k+2] == 0){
//			    *bot_height = i;
//				break;
//			}
            if(data[i*image_width + j] == 1){
			    *bot_height = i;
				break;
			}
		}
	}
	
	return (*top_height - *bot_height + 1);
}

unsigned int  getSlicedWidth(unsigned char *data, int left, int right, int image_width, int image_height, int *left_pos, int *right_pos)
{
    int i,j,k;
	
	*left_pos = left;
	*right_pos = right;
	
	int l_width = ((image_width*3 + 3)>>2)<<2;

#if DEBUG	
	printf("left=%d,right=%d\n",left,right);
#endif	

	for (i=left; i<=right; i++){
	    for (j=0; j<image_height; j++){
		    k = j*l_width + i*3;
			if (data[k] == 0 && data[k+1] == 0 && data[k+2] == 0){
			    *right_pos = i;
				break;
			}
		}
	}
	printf("left_border = %d\n",*left_pos);
	
	for (i=right; i>= left; i--){
	    for (j=0; j<image_height; j++){
		    k=j*l_width + i*3;
			if (data[k-1] ==0 && data[k-2] == 0 && data[k-3] ==0){
			    *left_pos = i;
				break;
			}
		}
	}
	
	printf("right_border=%d\n",*right_pos);
	
	return (*right_pos - *left_pos + 1);
}

static int IsForeGroundPixel(unsigned char *data)
{
    int retval = 0;

    if (data[0] == 0 && data[1] == 0 && data[2] == 0) {
        retval = 1;
    }

    return retval;
}

static int GetNeighbors(unsigned char *data,unsigned char *list,int pos_w,int pos_h,int width)
{
    /*0: foregroup pixel, 1:backgroup pixel*/
    list[0] = data[pos_h*width*3 + (pos_w+1)*3] == 0 ? 0: 1;
    list[1] = data[(pos_h+1)*width*3 + (pos_w+1)*3] == 0? 0 : 1;
    list[2] = data[(pos_h+1)*width*3 + pos_w*3] == 0 ? 0 : 1;
    list[3] = data[(pos_h+1)*width*3 + (pos_w -1)*3] == 0? 0: 1;
    list[4] = data[pos_h*width*3 + (pos_w-1)*3] == 0 ? 0:1;
    list[5] = data[(pos_h-1)*width*3 + (pos_w-1)*3] == 0?0:1;
    list[6] = data[(pos_h-1)*width*3 + pos_w*3] == 0 ? 0:1;
    list[7] = data[(pos_h-1)*width*3 + (pos_w-1)*3] == 0?0:1;
}

static int DetectConnectivity(unsigned char *list)
{
    int count = 0;

    count = list[6] - list[6]*list[7]*list[0];
    count+= list[0] - list[0]*list[1]*list[2];
    count+= list[2] - list[2]*list[3]*list[4];
    count+= list[4] - list[4]*list[5]*list[6];

    return count;
}

unsigned int ImageThinning(unsigned char *data, int width, int height)
{
    static int erasetable[256]={   
                                0,0,1,1,0,0,1,1,   
                                1,1,0,1,1,1,0,1,   
                                1,1,0,0,1,1,1,1,   
                                0,0,0,0,0,0,0,1,   
                                   
                                0,0,1,1,0,0,1,1,   
                                1,1,0,1,1,1,0,1,   
                                1,1,0,0,1,1,1,1,   
                                0,0,0,0,0,0,0,1,   
                                   
                                1,1,0,0,1,1,0,0,   
                                0,0,0,0,0,0,0,0,   
                                0,0,0,0,0,0,0,0,   
                                0,0,0,0,0,0,0,0,   
                                   
                                1,1,0,0,1,1,0,0,   
                                1,1,0,1,1,1,0,1,   
                                0,0,0,0,0,0,0,0,   
                                0,0,0,0,0,0,0,0,   
   
                                0,0,1,1,0,0,1,1,   
                                1,1,0,1,1,1,0,1,   
                                1,1,0,0,1,1,1,1,   
                                0,0,0,0,0,0,0,1,   
                                   
                                0,0,1,1,0,0,1,1,   
                                1,1,0,1,1,1,0,1,   
                                1,1,0,0,1,1,1,1,   
                                0,0,0,0,0,0,0,0,   
                                   
                                1,1,0,0,1,1,0,0,   
                                0,0,0,0,0,0,0,0,   
                                1,1,0,0,1,1,1,1,   
                                0,0,0,0,0,0,0,0,   
   
                                1,1,0,0,1,1,0,0,   
                                1,1,0,1,1,1,0,0,   
                                1,1,0,0,1,1,1,0,   
                                1,1,0,0,1,0,0,0   
                          };  
    
	
	
	
    unsigned char n,e,s,w,ne,se,nw,sw;
	int l_width = ((width*3 + 3)>>2)<<2;
    short Finished = 0;
	int x,y;
	int k ;
	
	unsigned char *thinning_data = new unsigned char[width*height];
	int i,j;
	
	for (i=0; i<height; i++){
	    for(j=0; j<width; j++){
		    if (data[i*width + j] == 1){
			    thinning_data[i*width+j] = 0;
			}
			else{
			    thinning_data[i*width+j] = 255;
			}
		}
	}
	
    while(!Finished){
        Finished = 1;
		for (y=1; y<height-1; y++){
		    x = 1;
			while(x < width - 1){
			    k=y*width + x;
				if (thinning_data[k] == 0 ){
				    w = thinning_data[k-1];
					e = thinning_data[k+1];
					if (w == 255 || e == 255){
					    nw = thinning_data[k+width -1];
						n  = thinning_data[k+width];
						ne = thinning_data[k+width+1];
						sw = thinning_data[k-width -1];
						s  = thinning_data[k-width];
						se = thinning_data[k+width+1];
						int num = nw/255 + n/255*2 + ne/255*4 + w/255*8+e/255*16+sw/255*32+s/255*64+se/255*128;
						if (erasetable[num] == 1){
						    thinning_data[k] = 255;
							Finished = 0;
							x++;
						}
					}
				}
				x++;
			}
		}
		
		for (x=1; x < width-1; x++){
		    y=1;
			while(y < height -1){
			    k=y*width+x;
				if (thinning_data[k] == 0){
				    n = thinning_data[k+width];
					s = thinning_data[k-width];
					if (n==255 || s==255){
					    nw = thinning_data[k+width -1];
						ne = thinning_data[k+width + 1];
						w  = thinning_data[k-1];
						e  = thinning_data[k+1];
						sw = thinning_data[k-width-1];
						se = thinning_data[k-width+1];
						int num=nw/255+n/255*2+ne/255*4+w/255*8+e/255*16+sw/255*32+s/255*64+se/255*128; 
						if(erasetable[num] == 1){
						    thinning_data[k] = 255;
							Finished = 0;
							y++;
						}
					}
				}
				y++;
			}
		}
	}

    for (i=0; i<height; i++){
        for (j=0; j<width; j++){
		    k=i*width+j;
			if (thinning_data[k] == 255){
			    data[k] = 0;
			}
			else{
			    data[k] = 1;
			}
		}
	}	
	
	delete[] thinning_data;
}

static int RoughSplit(unsigned char *data,int *pos,int width,int height)
{
    int i,j,k;
    int l_width = ((width*3 + 3)>>2)<<2; /*width alignment as 4 times*/

    for (i=0; i<width; i++) {
        int pixel_num = 0;
        for (j=0; j<height; j++) { /*scan column*/
            k = j*l_width + i*3;
            if (data[k] == 0 && data[k+1] == 0 && data[k+2] == 0) {
                pixel_num ++;
            }
        }

        if (pixel_num <=3) {
            pos[i] = 0;
        } else {
            pos[i] = 1;
        }
    }


#if 1
    int count = 0;
    int flag = 0;
    int seg_start = 0;
    int seg_end   = 0;
    for (i=0; i<width; i++) {
        /*skip 0*/
        if (pos[i] == 0 && flag == 0) {
            continue;
        }

        if (flag == 0) {
            seg_start = i;
        }

        flag = 1;
        if (pos[i] == 1 && flag == 1) {
            count ++;
        } else {
            seg_end = i-1;

            if (count > 12 && count < 24) {
                /*two numbers*/
                pos[seg_start + count/2] = 0;
            } else if (count >= 24) {
                pos[seg_start + count/3] = 0;
                pos[seg_start + 2*count/3] = 0;
            }

            count = 0;
            flag = 0;
        }

    }
#endif

#if 0
    printf("");
    for (i=0; i<width; i++){
        printf("pos[%d]=%d ",i,pos[i]);
	}
	printf("\n");
#endif
    return 0;
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


static int checkANNOutput(float *ann_output){
    int retvalue = 0;
	int i = 0;
	
	for (i=0; i<ANN_OUTPUT_NUM; i++){
	    if (ann_output[i] < -0.2){
		    retvalue = -1;
			break;
		}
		
		if (ann_output[i] >= 0.25 && ann_output[i] <= 0.75){
		    retvalue = -1;
			break;
		}
		
		if (fabs(ann_output[i]) <= 0.25){
		    retvalue = retvalue*2;
		}
		else{
		    retvalue = retvalue*2 + 1;
		}
	}
	
	return retvalue;
}

unsigned int ImageProcessing(unsigned char *data, int width, int height)
{
    int i,j,k;
	int w,h;
	int left, right;
	float calc_out[ANN_OUTPUT_NUM];
	int recogResult[13] = {0};
	int iResult = 0;
	
//	int l_width = ((width*3 + 3)>>2)<<2;
	
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
//		    k = i*l_width + left*3;
//			if (data[k] == 0 && data[k+1] == 0 && data[k+2] == 0){
//			    pixel_num ++;
//			}
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
			
//			recogDigital(feature_vector,&calc_out[0]);
			
#if DEBUG
            printf("index=%d,simvalue=%0.3f,recog=%d\n",index,simvalue,train_value[index]);
#endif
			
#if DEBUG
            printf("%0.3f %0.3f %0.3f %0.3f\n",calc_out[0],calc_out[1],calc_out[2],calc_out[3]);
#endif
			
#if DEBUG
            PRINT_NORM(norm_data,NORM_WIDTH,NORM_HEIGHT);
#endif

#if DEBUG
			PRINT_FEATURE(feature_vector,MAX_FEATURE_LEN);
#endif

/*judge the output*/
#if 0
            if (simvalue >= 0.88){
#if DEBUG
                printf("recogNUM=%d\n",train_value[index]);
#endif
                recogResult[iResult++] = train_value[index];
                break;
			}
#else
            if (simvalue >= MIN_SIM){
			    if (maxsim[bias+2] < simvalue){
				    maxsim[bias+2] = simvalue;
					window_len[bias+2] = j;
					vIndex[bias+2] = index;
				}
			}
#endif


#if 0
			if (simvalue >= 0.80){
			    /*check NN output*/
				int ann_value = checkANNOutput(calc_out);
#if DEBUG
				printf("ann_value=%d\n",ann_value);
#endif
				if (ann_value >= 0){
				    if (ann_value == train_value[index]){
					    /*trust output*/
#if DEBUG
						printf("recogNUM=%d\n",ann_value);
#endif
                        recogResult[iResult++] = train_value[index];
						break;
					}
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
#if 0
		if (j < MIN_WINDOW_LEN){
		    left++;
		}else{
		    if (recogResult[iResult-1] == 7){
			    left = left + j - 3;
			}
			else{
		        left = left + j -1; /*move left to new position*/
			}
		}
#else

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
#endif

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
#if 0
int recog_one_sliced(unsigned char *data,int sliced_left, int sliced_right, int top_height, int bot_height, int image_width, int image_height)
{
    int i;
	int recogNum = 0;
	float calc_out[ANN_OUTPUT_NUM];
	
	int left_pos = 0;
	int right_pos = 0;
	int top_pos = 0;
	int bot_pos = 0;
	
//	getSlicedWidth(data,sliced_left,sliced_right,image_width,image_height,&left_pos,&right_pos);
	getSlicedHeight(data,sliced_left,sliced_right,image_width,image_height,&top_pos,&bot_pos);
	
//    unsigned char *sliced = new unsigned char[(sliced_right - sliced_left + 1)*(top_height - bot_height + 1)];
    unsigned char *sliced = new unsigned char[(sliced_right - sliced_left + 1)*(top_pos - bot_pos + 1)];
	
	if (sliced == NULL){
	    printf("(line=%d)allocate memory failure!\n",__LINE__);
	    exit(1);
	}
	
//	copy_char(data,sliced,sliced_left,sliced_right,top_height,bot_height,image_width);
    copy_char(data,sliced,sliced_left,sliced_right,top_pos,bot_pos,image_width);
	unsigned char *norm_data = new unsigned char[NORM_WIDTH*NORM_HEIGHT];
//	norm_data = ImageNorm(sliced,sliced_right - sliced_left + 1,top_height - bot_height + 1);
    ImageNorm(sliced,norm_data,sliced_right - sliced_left + 1, top_pos - bot_pos + 1);
//	ImageThinning(norm_data,NORM_WIDTH,NORM_HEIGHT);
	delete[] sliced;
	
	
	float *feature_vector = new float[MAX_FEATURE_LEN];
	if (feature_vector == NULL){
		printf("(line=%d)allocate memory failure!\n",__LINE__);
		exit(1);
	}
	memset(feature_vector,0,MAX_FEATURE_LEN);
	getfeatureVector(norm_data,feature_vector, NORM_WIDTH,NORM_HEIGHT);
				
	PRINT_NORM(norm_data,NORM_WIDTH,NORM_HEIGHT);
	PRINT_FEATURE(feature_vector,MAX_FEATURE_LEN);
	
	recogDigital(feature_vector,&calc_out[0]);
	printf("%f %f %f %f\n",calc_out[0],calc_out[1],calc_out[2],calc_out[3]);

    for (i=0; i<4; i++){
        if (calc_out[i] < (-0.2)){
		    printf("calc_out[%d]=%f\n",i,calc_out[i]);
		    return -1;
		}
		else if (calc_out[i] >= 0.25 && calc_out[i] <= 0.75){
		    printf("calc_out[%d]=%f\n",i,calc_out[i]);
		    return -1;
		}
	}	
	
	for (i=0; i<4; i++){
	    if (fabs(calc_out[i]) > 0.5){
		    recogNum = 2*recogNum + 1;
		}
		else{
		    recogNum = 2*recogNum;
		}
	}
	printf("recogNum=%d\n",recogNum);
	return recogNum;
}

void set_verticalbar_blue(unsigned char *data, int width, int height, int leftpos, int rightpos)
{
    int i,j,k;
	int l_width = ((width*3 + 3)>>2)<<2;
	
	for (i=0; i<height; i++){
	    j = i*l_width + rightpos*3;
	    k = i*l_width + leftpos*3;
		
		data[k] = 255;
		data[k+1] = 0;
		data[k+2] = 0;
		
		data[j] = 255;
		data[j+1] = 0;
		data[j+2] = 0;
	}
	
}

void set_verticalbar_red(unsigned char *data, int width, int height, int leftpos, int rightpos)
{
    int i,j,k;
	int l_width = ((width*3 + 3)>>2)<<2;
	
	for (i=0; i<height; i++){
	    j = i*l_width + rightpos*3;
	    k = i*l_width + leftpos*3;
		
		data[k] = 0;
		data[k+1] = 255;
		data[k+2] = 0;
		
		data[j] = 0;
		data[j+1] = 0;
		data[j+2] = 255;
	}
	
}
#endif

void copy_char(unsigned char *src,unsigned char *dest, int left,int right, int top, int bot, int image_width)
{
    int i,j,k;
	
//	int l_width = ((image_width*3 + 3)>>2)<<2;
	int new_width = right - left + 1;
	
	for (i=bot; i<=top; i++){
	    for (j=left; j<=right; j++){
//		    k = i*l_width + j*3;
//			if (src[k] == 0 && src[k+1] == 0 && src[k+2] == 0){
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



/*水平线穿过height_pos时0变1和1变0的个数*/
unsigned int getHorizonCrossPoint(unsigned int *data,int left,int right,int height_pos,int image_width)
{	
	int i;
	unsigned int count = 0;
	for (i=left+1; i<right -1; i++){
	    if (abs(data[height_pos*image_width + i+1] - data[height_pos*image_width + i]) == 1){
		    count++;
		}
	}
	
	return count;
}

/*垂直线穿过width_pos时0变1和1变0的个数*/
unsigned int getVerticalCrossPoint(unsigned int *data,int width_pos,int image_width,int image_height)
{
    int i = 0;
	unsigned int count = 0;
	
	for (i=1; i<image_height-1; i++){
	    if (abs(data[(i+1)*image_width + width_pos] - data[i*image_width + width_pos]) == 1){
		    count++;
		}		
	}
	
	return count;
}

#if 0
unsigned int recogDigital(float *vector, float *fout)
{
    fann_type *calc_out;
//	int calc_iout[4];
	fann_type input[MAX_FEATURE_LEN];
	int rec_num;
	int i=0;
	
	for (i=0; i<MAX_FEATURE_LEN; i++){
	    input[i] = (fann_type)vector[i];
	}
	/*create ANN*/
	struct fann *ann = fann_create_from_file("digital.net");
	
	calc_out=fann_run(ann,input);
	for (i=0; i<ANN_OUTPUT_NUM; i++){
	    fout[i] = calc_out[i];
	}

	fann_destroy(ann);
	
	return 0;
}
#endif