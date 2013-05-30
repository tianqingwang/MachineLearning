
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <string.h>
#include "floatfann.h"

#define PI 3.1415926535898
#define MAX_FEATURE_LEN    (13)
#define NORM_WIDTH         (12)
#define NORM_HEIGHT        (12)
#define MAX_CHAR_WIDTH     (11)
#define WINDOW_STEP        (3)
#define MIN_WINDOW_LEN     (3)
#define MAX_WINDOW_LEN     (12)
#define ANN_OUTPUT_NUM     (4)

unsigned char * recogBMP(char* filename);
unsigned int  getImageWidth(unsigned char *data,int width,int height);
unsigned int  getImageHeight(unsigned char *data,int width,int height,int *top_height,int *bot_height);
unsigned int ImageRotation(unsigned char *data, int width, int height);
unsigned int ImageSplit(unsigned char *data, int width, int height);
unsigned int ImageThinning(unsigned char *data, int width, int height);
unsigned int getfeatureVector(unsigned char *data, unsigned int *vector,int image_width, int image_height);
unsigned int recogDigital(unsigned int *vector,float *calc_out);
void set_verticalbar_blue(unsigned char *data, int width, int height, int leftpos, int rightpos);
void set_verticalbar_red(unsigned char *data, int width, int height, int leftpos, int rightpos);
void copy_char(unsigned char *src,unsigned char *dest, int left,int right, int top, int bot, int image_width);
void PRINT_NORM(unsigned char *norm_data,int width, int height);
void PRINT_FEATURE(unsigned int *vector, int feature_len);

//unsigned int MedFilter(unsigned char *data,int width, int height);

int main(int argc, char * argv[])
{
    char filename[256];
	int i =0;
	
	if (argc < 2){
	    printf("Usage:\n");
		printf(" ./bmp file_to_recog\n");
		exit(0);
	}
	else{
	    for (i=1; i<argc; i++){
            strcpy(filename, argv[i]);
            printf("%s\n",filename);
            recogBMP(filename);
		}
    }
    return 0;
}

unsigned char * recogBMP(char* filename)
{
    int i = 0;
    int j = 0;
    int k = 0;
	
	int top_height,bot_height;

    /*read image head information*/
    FILE *f = fopen(filename,"rb");
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
            if (gray >= 200) {/*white color*/
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
	
    ImageRotation(data,width,height);
//    ImageThinning(data,width,height);
    ImageSplit(data,width,height);
#if 1
    /*for test only*/
    printf("width    :%d\n",width);
    printf("height   :%d\n",height);

    FILE *fo = fopen("b.bmp","wb");

    fwrite(info,sizeof(unsigned char),54,fo);
    fwrite(data,sizeof(unsigned char),size,fo);
    
    fclose(fo);
#endif
    return data;
}

/*need to add automatic angle adjusting*/
unsigned int ImageRotation(unsigned char *data, int width, int height)
{
    int angle  = 0;

    int i = 0;
    int j = 0;
    int k = 0;

    int new_x, new_y;

    int l_width = ((width*3 + 3)>>2)<<2;

    unsigned char *rot_data = new unsigned char[3*l_width*height];

    memset(rot_data,255,3*l_width*height);

//	for (angle = 0; angle <= 30; angle ++){
    /*for test*/
    angle = 12;
    /*end test*/
    for (i = 0; i < height; i++) {
        for (j = 0; j<width; j++) {
            double arc_angle = angle*PI/180.0;
            k = i*l_width + j*3;
            if (data[k] == 0 && data[k+1] == 0 && data[k+2] == 0) {
                int new_x = (int)(j-i*tan(arc_angle));
                int new_y = i;
                if (new_x >=0 && new_x < width) {
                    int new_k = new_y*l_width + new_x*3;
					new_x += 3;
                    rot_data[new_k] = data[k];
                    rot_data[new_k+1] = data[k+1];
                    rot_data[new_k+2] = data[k+2];
                }
            }

        }
    }

    for (i=0; i<height; i++) {
        for (j=0; j<width; j++) {
            k = i*l_width + j*3;
            data[k] = rot_data[k];
            data[k+1] = rot_data[k+1];
            data[k+2] = rot_data[k+2];
        }
    }
//	}


    delete[] rot_data;
    return 0;
}


unsigned int  getImageWidth(unsigned char *data,int width,int height)
{
    unsigned int left_border  = 0;
    unsigned int right_border = width;
    int i = 0;
    int j = 0;
    int k = 0;
    int l_width = ((width*3 + 3)>>2)<<2;
    /*scan from left to right*/
    int count = 0;

    for (i=0; i<width; i++) {
        for (j=0; j<height; j++) {
            k=j*l_width + i*3;
            if (data[k] == 0 && data[k+1] == 0 && data[k + 2] == 0) {
                count++;
                if (count == 2) {
                    left_border = i;
                    break;
                }

            }
        }
    }
    /*draw vertical*/
    for (j = 0; j<height; j++) {
        data[j*l_width + (left_border*3)] = 0;
        data[j*l_width + (left_border*3) + 1] = 0;
        data[j*l_width + (left_border*3) + 2] = 255;
    }

    printf("left_border:%d\n",left_border);
    /*scan from right to left*/
    count = 0;
    for (i=width; i>=0; i--) {
        for (j=0; j<height; j++) {
            k=j*l_width + 3*i;
            if (data[k-1] == 0 && data[k -2] == 0 && data[k -3] == 0) {
                count++;
                if (count == 2) {
                    right_border = i;
                    break;
                }
            }
        }
    }

    /*draw vertical*/
    for (j = 0; j<height; j++) {
        data[j*l_width + (right_border*3)] = 0;
        data[j*l_width + (right_border*3) + 1] = 255;
        data[j*l_width + (right_border*3) + 2] = 0;
    }

    printf("right_border:%d\n",right_border);

    return right_border - left_border;
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

static unsigned char *ImageNorm(unsigned char *data,int width, int height)
{
    int i,j;
    int new_x, new_y;

    /*normalize the data to NORM_WIDTH X NORM_HEIGHT*/
    unsigned char *norm_data  = new unsigned char[NORM_WIDTH*NORM_HEIGHT];
    memset(norm_data,0,NORM_WIDTH*NORM_HEIGHT);

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


    return norm_data;
}

unsigned int ImageSplit(unsigned char *data,int width, int height){
    int i,j,k;
	int w,h;
	int left,right;
	int pixel_num = 0;
	int top_height,bot_height;
	int new_height = getImageHeight(data,width,height,&top_height,&bot_height);
	int l_width = ((width*3+3)>>2)<<2;
	
	float calc_out[ANN_OUTPUT_NUM];
	int   calc_iout[ANN_OUTPUT_NUM];
	
	left = right = 0;
	
	while(1){
		if (left >= width){
		    break;
		}
		
		while(left < width){
		    pixel_num = 0;
		    for (i=0; i<height; i++){/*scan from bottom to top*/
                k = i*l_width + left*3;
				if (data[k] == 0 && data[k+1] == 0 && data[k+2] == 0){
				    pixel_num ++;
				}
			}
			
			if (pixel_num <= 3){
			    left ++;
			}
			else{
			    break;
			}
		}
		
		right = left + 1;
		
		if (right >= width){
		    break;
		}
		
		while(right < width){
		    pixel_num = 0;
		    for (i=0;i<height; i++){
                k = i*l_width + right*3;
				if (data[k] == 0 && data[k+1] == 0 && data[k+2] == 0){
				    pixel_num ++;
				}
			}
			
			if (pixel_num > 3){
			    right++;
			}
			else{
			    break;
			}
		}
		
		if (right >= width){
		    break;
		}
		
		if ((right-left) <= 2){
		    left = right + 1;
			continue;
		}
		
		
		int recogNum = 0;
		if ((right-left) <= MAX_CHAR_WIDTH){
		    unsigned char *one_ch = new unsigned char[(right-left)*(top_height-bot_height+1)];
			if (one_ch == NULL){
			    printf("(%d) allocate memory failed\n",__LINE__);
			}
			copy_char(data,one_ch,left,right-1,top_height,bot_height,width);
			unsigned char *norm_data = new unsigned char[NORM_WIDTH*NORM_HEIGHT];
			if (norm_data == NULL){
			    printf("(%d) allocate memory failed\n",__LINE__);
			}
			
			norm_data = ImageNorm(one_ch,right-left,top_height-bot_height+1);
			
			unsigned int *feature_vector = new unsigned int[MAX_FEATURE_LEN];
			if (feature_vector == NULL){
			    printf("(%d) allocate memory failed\n",__LINE__);
			}
            memset(feature_vector,0,MAX_FEATURE_LEN);

            getfeatureVector(norm_data,feature_vector,NORM_WIDTH,NORM_HEIGHT);
#if 1
			PRINT_NORM(norm_data,NORM_WIDTH,NORM_HEIGHT);
			PRINT_FEATURE(feature_vector,MAX_FEATURE_LEN);
#endif
            
            recogDigital(feature_vector,&calc_out[0]);
			
			printf("%f %f %f %f\n",calc_out[0],calc_out[1],calc_out[2],calc_out[3]);
			
			for (i=0; i<4; i++){
			    if (fabs(calc_out[i]) > 0.5){
				    calc_iout[i] = 1;
					
				}
				else{
				    calc_iout[i] = 0;
				}
			}
			
		    printf("%d  %d %d %d\n",calc_iout[0],calc_iout[1],calc_iout[2],calc_iout[3]);
			recogNum = (calc_iout[0]<<3)+(calc_iout[1]<<2) + (calc_iout[2]<<1) + calc_iout[3];
			
		    if (recogNum > 9){
		         printf("recognized failure(recogNum = %d)\n",recogNum);
			     
//			     exit(1);
		    }
		    else{
		        printf("%d\n",recogNum);
		    }

#if 1			
			set_verticalbar_blue(data,width,height,left,right-1);
#endif
            delete[] one_ch;
            delete[] norm_data;
			delete[] feature_vector;
		}
		else{

		    int new_left  = left;
		    int new_right = left + 6;
			
		    while(new_right < right){
			    printf("\n\nnew_left=%d  new_right=%d\n",new_left,new_right);
			    unsigned char *sliced = new unsigned char[(new_right - new_left + 1)*(top_height - bot_height + 1)];
				if (sliced == NULL){
				    printf("(line=%d)allocate memory failure!\n",__LINE__);
					exit(1);
				}
				
				copy_char(data,sliced,new_left,new_right,top_height,bot_height,width);
				
				unsigned char *norm_data = new unsigned char[NORM_WIDTH*NORM_HEIGHT];
				if (norm_data == NULL){
				    printf("(line=%d)allocate memory failure!\n",__LINE__);
					exit(1);
				}
				norm_data = ImageNorm(sliced,new_right - new_left + 1,top_height - bot_height + 1);
				
				delete[] sliced;
				
				unsigned int *feature_vector = new unsigned int[MAX_FEATURE_LEN];
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
				
				int flag = 0;
				for (i=0; i<4; i++){
				    if (calc_out[i] < -0.2){
					    flag = 1;
						break;
					}
					else if (calc_out[i] > 0 && fabs(calc_out[i] - 0.5) <= 0.15){
					    flag = 1;
						break;
					}
				}
				
				if (flag == 1){
				    /**/
					new_right ++;
				}
				else{
				    /*recog this number*/
					for (i=0; i<4; i++){
					    if (fabs(calc_out[i]) > 0.5){
						    calc_iout[i] = 1;
						}
						else{
						    calc_iout[i] = 0;
						}
					}
					
					recogNum = (calc_iout[0]<<3) + (calc_iout[1]<<2) + (calc_iout[2]<<1) + calc_iout[3];
					if (recogNum > 9){
					    printf("recognized failure(recogNum = %d)\n",recogNum);
					}
					else{
					    printf("%d\n",recogNum);
					}
					set_verticalbar_red(data,width,height,new_left,new_right-1);
				    new_left = new_right + 1;
					new_right = new_left + 6;
					if (new_right > right && (new_right - right) <=2){
					    new_right = right -1;
					}
				}
				
				delete[] norm_data;
				delete[] feature_vector;
			}
		    
		}
		
		left = right;
		
	}
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

void copy_char(unsigned char *src,unsigned char *dest, int left,int right, int top, int bot, int image_width)
{
    int i,j,k;
	
	int l_width = ((image_width*3 + 3)>>2)<<2;
	int new_width = right - left + 1;
	
	for (i=bot; i<=top; i++){
	    for (j=left; j<=right; j++){
		    k = i*l_width + j*3;
			if (src[k] == 0 && src[k+1] == 0 && src[k+2] == 0){
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
    for (k=height-1; k>=0; k--) {
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

void PRINT_FEATURE(unsigned int *vector, int feature_len)
{
    int i = 0;
	for (i=0; i<feature_len; i++){
	    printf("%d ",vector[i]);
	}
	printf("\n");
}



unsigned int getfeatureVector(unsigned char *data, unsigned int *vector,int image_width, int image_height)
{
    int i,j;
    /*split image into 4*2 blocks*/
//  _____________
//  |  7  | 8   |     
//  |_____|_____|
//  |  5  | 6   |     
//  |_____|_____|
//  |  3  | 4   |     
//  |_____|_____|
//  |  1  | 2   |     
//  |_____|_____|

    int first_height = image_height/4;
    int first_width  = image_width/2;
	
	int third_width  = image_width/3;
	int third_height = image_height/3;
	

    int count = 0;
    int s,w;

    for (s = 0; s<4; s++) {
        for (w=0; w<2; w++) {
            count = 0;
            for (i=s*first_height; i<(s+1)*first_height; i++) {
                for (j=w*first_width; j<(w+1)*first_width; j++) {
                    if (data[i*image_width + j] == 1) {
                        count++;
                    }
                }
            }
            vector[2*s+w] = count;
        }
    }

    /*垂直线穿过1/3,2/3宽度位置时的黑像素个数*/
    count = 0;
    for (i=0; i<image_height; i++){
	    if (data[i*image_width + third_width] == 1){
		    count ++;
		}
	}
	vector[8] = count;
	
	count = 0;
	for (i=0; i<image_height; i++){
	    if (data[i*image_width + 2*third_width] == 1){
		    count++;
		}
	}
	vector[9] = count;
	


    /*水平线穿过图像1/3和2/3高度位置时的黑像素个数*/
    count = 0;
	for (i=0; i<image_width; i++){
	    if (data[third_height*image_width + i] == 1){
		    count++;
		}
	}
	vector[10] = count;
	
	count = 0;
	for (i=0; i<image_width; i++){
	    if (data[2*third_height*image_width + i] == 1){
		    count++;
		}
	}
	vector[11] = count;
	
	/*图像总的黑像素个数*/
	count = 0;
	for (i=0; i<image_height; i++){
	    for (j=0; j<image_width; j++){
		    if (data[i*image_width + j] == 1){
			    count++;
			}
		}
	}
	vector[12] = count;
        
	
	return 0;
}

unsigned int recogDigital(unsigned int *vector, float *fout)
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
