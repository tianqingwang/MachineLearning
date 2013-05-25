
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <string.h>


#define PI 3.1415926535898
#define MAX_FEATURE_LEN    (13)
#define NORM_WIDTH         (16)
#define NORM_HEIGHT        (28)

unsigned char * readBMP(char* filename);
unsigned int  getImageWidth(unsigned char *data,int width,int height);
unsigned int  getImageHeight(unsigned char *data,int width,int height);
unsigned int ImageRotation(unsigned char *data, int width, int height);
unsigned int ImageSplit(unsigned char *data, int width, int height);
unsigned int ImageThinning(unsigned char *data, int width, int height);
unsigned int getfeatureVector(unsigned char *data, unsigned int *vector,int image_width, int image_height);

int main(int argc, char * argv[])
{
    char filename[256];
	strcpy(filename, argv[1]);
	printf("%s\n",filename);
	readBMP(filename);
	
	return 0;
}

unsigned char * readBMP(char* filename)
{
    int i = 0;
	int j = 0;
	int k = 0;
	
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
	for (i=0; i<height; i++){
	    for (j=0; j<width; j++){
		    k = i*l_width+ j*3;
			unsigned char tmp = data[k];
			data[k] = data[k+2];
			data[k+2] = tmp;
			
			int gray = (int)(((int)data[k])*30 + ((int)data[k+1])*59 + ((int)data[k+2])*11 + 50)/100;
			if (gray >= 200){
			    data[k] = 255;
				data[k+1] = 255;
				data[k+2] = 255;
			}
			else{
			    data[k] = 0;
				data[k+1] = 0;
				data[k+2] = 0;
			}
		}
	}

//	int new_width = getImageWidth(data,width,height);
//	int new_height = getImageHeight(data,width,height);
    ImageRotation(data,width,height);
//    ImageThinning(data,width,height);	
    ImageSplit(data,width,height);

	printf("width    :%d\n",width);
	printf("height   :%d\n",height);

//	printf("new_width:%d\n",new_width);
//	printf("new_height:%d\n",new_height);
//	printf("rot_width:%d\n",rot_width);
//	printf("rot_height:%d\n",rot_height);
	
    /*draw horizon*/
/*
	for (i = 0; i<width*3; i+=3){
	    data[(height>>1)*width*3 +i] = 255;
		data[(height>>1)*width*3 +i + 1] = 0;
		data[(height>>1)*width*3 +i + 2] = 0;
	}
	
	/*draw vertical*/
/*
	int j = 0;
	i = 40;
	for (j=0; j<height; j++){
	    data[j*width*3 + (i*3)] = 0;
        data[j*width*3 + (i*3) + 1] = 0;
        data[j*width*3 + (i*3) + 2] = 255;  		
	}
*/	
	
	FILE *fo = fopen("b.bmp","wb");
	
	fwrite(info,sizeof(unsigned char),54,fo);
	fwrite(data,sizeof(unsigned char),size,fo);
/*
	if (width % 2 == 1){
	    char *data_padded = new char[3*height];
		memset(data_padded,0xFF,3*height);
		fwrite(data_padded,sizeof(char),3*height,fo);
	}
*/	
	fclose(fo);
	
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
		angle = 13;
		/*end test*/
	    for (i = 0; i < height; i++){
            for (j = 0; j<width; j++){
			    double arc_angle = angle*PI/180.0;
				k = i*l_width + j*3;
				if (data[k] == 0 && data[k+1] == 0 && data[k+2] == 0){
				    int new_x = (int)(j-i*tan(arc_angle));
					int new_y = i;
					if (new_x >=0 && new_x < width){
					    int new_k = new_y*l_width + new_x*3;
						rot_data[new_k] = data[k];
						rot_data[new_k+1] = data[k+1];
						rot_data[new_k+2] = data[k+2];
					}
				}

			}
		}
        
        for (i=0; i<height; i++){
            for (j=0; j<width; j++){
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
	/*scan from left to right*/
	
	int count = 0;
	
	for (i=0; i<width; i++){
	    for (j=0; j<height; j++){
		    if (data[j*width*3 + (i*3)] == 0 && data[j*width*3 + (i*3) + 1] == 0 && data[j*width*3 + (i*3) + 2] == 0)
			{
			    count++;
			    if (count == 2){
			        left_border = i;
				    break;
				}
				
			}
		}
	}
    /*draw vertical*/
	for (j = 0; j<height; j++){
	    data[j*width*3 + (left_border*3)] = 0;
        data[j*width*3 + (left_border*3) + 1] = 0;
        data[j*width*3 + (left_border*3) + 2] = 255;  
	}
	
	printf("left_border:%d\n",left_border);
	/*scan from right to left*/
	count = 0;
	for (i=width; i>=0; i--){
		for (j=0; j<height; j++){
		    if (data[j*width*3 + (i*3)-1] == 0 && data[j*width*3 + i*3 -2] == 0 && data[j*width*3 + i*3 -3] == 0){
			    count++;
				if (count == 2){
				    right_border = i;
				    break;
				}
			}
		}
	}
	
	/*draw vertical*/
	for (j = 0; j<height; j++){
	    data[j*width*3 + (right_border*3)] = 0;
        data[j*width*3 + (right_border*3) + 1] = 255;
        data[j*width*3 + (right_border*3) + 2] = 0;  
	}
	
	printf("right_border:%d\n",right_border);
	
	return right_border - left_border;
}

unsigned int getImageHeight(unsigned char *data,int width,int height)
{
    unsigned int top_border     = height;
	unsigned int bottom_border  = 0;
	int i = 0;
	int j = 0;
    
	/*scan from bottom to top*/
	for (i=0; i<height; i++){
	    for (j=0; j<width; j++){
		    if (data[i*width*3 + j*3] == 0 && data[i*width*3 + j*3 + 1] == 0 && data[i*width*3 + j*3 + 2] == 0){
			    top_border = i;
				break;
			}
		}
	}
	
	printf("top_border:%d\n",top_border);
	
	/*draw horizon*/
	for (i=0; i<width; i++){
	    data[top_border*width*3 + i*3] = 200;
		data[top_border*width*3 + i*3 + 1] = 100;
		data[top_border*width*3 + i*3 + 2] = 0;
	}

	/*scan from top to bottom*/
	for (i=height-1; i>=0; i--){
	    for (j=0; j<width; j++){
		    if (data[i*width*3 + j*3] == 0 && data[i*width*3 + j*3 + 1] == 0 && data[i*width*3 + j*3 + 2] == 0){
			    bottom_border = i;
				break;
			}
		}
	}
	
	printf("bottom_border:%d\n",bottom_border);
	
	/*draw horizon*/
	for (i=0; i<width; i++){
	    data[bottom_border*width*3 + i*3] = 100;
		data[bottom_border*width*3 + i*3 + 1] = 155;
		data[bottom_border*width*3 + i*3 + 2] = 244;	
	}	
	
	return (top_border - bottom_border);
}

static int IsForeGroundPixel(unsigned char *data){
    int retval = 0;
	
	if (data[0] == 0 && data[1] == 0 && data[2] == 0){
	    retval = 1;
	}
	
	return retval;
}

static int GetNeighbors(unsigned char *data,unsigned char *list,int pos_w,int pos_h,int width){
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

static int DetectConnectivity(unsigned char *list){
    int count = 0;
	
	count = list[6] - list[6]*list[7]*list[0];
	count+= list[0] - list[0]*list[1]*list[2];
	count+= list[2] - list[2]*list[3]*list[4];
	count+= list[4] - list[4]*list[5]*list[6];
	
	return count;
}

unsigned int ImageThinning(unsigned char *data, int width, int height)
{
    int i,j;
	unsigned char *p = new unsigned char[8];
	int loop = 1;
	
	unsigned char *mask = new unsigned char[width*height];
	memset(mask, 0, width*height);
	
	/*Hilditch Algorithm*/
	while(loop){
	    loop = 0;
		
		for (i=1; i<height-1; i++){
		    for (j=1; j<width-1; j++){
			    /*condition 1: must be foreground pixel*/
				if (data[i*width*3 + j*3] != 0 && data[i*width*3 + j*3 + 1] != 0 && data[i*width*3+j*3+2] != 0){
				    continue;
				}
				
				//p3 p2 p1
				//p4 p  p0
				//p5 p6 p7
				memset(p,0,8);
				GetNeighbors(data,p,j,i,width);
				/*condition2: p0,p2,p4,p6 must have one backgroup pixel*/
				if (p[0] == 0 && p[2] == 0&& p[4] == 0 && p[6] == 0){
				    continue;
				}
				
				/*condition3: p0 ~ p7 must have 2 foregroup pixels at least*/
				int count = 0;
				int k = 0;
				for (k=0; k<8; k++){
				    count += p[k];
				}
				
				if (count < 2){
				    continue;
				}
				
				/*condition4:connectivity = 1*/
				if (DetectConnectivity(p) != 1){
				    continue;
				}
				
				/*condition5: if p2 is masked as deleted, set p2 as backgroup, the connectivity is still equal 1*/
				if (mask[j*width+i+1] == 1){
				    int temp = p[2];
				    p[2] = 1;
					if (DetectConnectivity(p) != 1){
					    continue;
					}
					p[2] = temp;
				}
				
				/*condition6:if p4 is masked as deleted, set p4 as backgroup, the connectivity is still equal 1*/
				if (mask[(j-1)*width+i] == 1){
				    int temp = p[4];
				    p[4] = 1;
					if (DetectConnectivity(p) != 1){
					    continue;
					}
					p[4] = temp;
				}
				
				mask[j*width+i] = 1;
				loop = 1;
			}
		}
		
		/*delete all masked as backgroup*/
		for (i=0; i<height; i++){
		    for (j=0; j<width; j++){
			    if (mask[i*width + j] == 1){
				    data[i*width*3 + j*3] = 255;
					data[i*width*3 + j*3 + 1] = 255;
					data[i*width*3 + j*3 + 2] = 255;
					mask[i*width+j] = 0;
				}
			}
		}
	}
	
	delete[] mask;
}

static int RoughSplit(unsigned char *data,int *pos,int width,int height)
{
    int i,j,k;
	int l_width = ((width*3 + 3)>>2)<<2; /*width alignment as 4 times*/
	printf("width = %d, l_width = %d\n",width,l_width);
	for (i=0; i<width; i++){
	    int pixel_num = 0;
		for (j=0; j<height; j++){ /*scan column*/
		    k = j*l_width + i*3;
			if (data[k] == 0 && data[k+1] == 0 && data[k+2] == 0){
			    pixel_num ++;
			}
		}
		
		if (pixel_num <=3){
		    pos[i] = 0;
		}
		else{
		    pos[i] = 1;
		}
	}
	
	int count = 0;
	int flag = 0;
	int seg_start = 0;
	int seg_end   = 0;
	for (i=0; i<width; i++){
	    /*skip 0*/
		if (pos[i] == 0 && flag == 0){
		    continue;
		}
		
		if (flag == 0){
		    seg_start = i;
		}
		
		flag = 1;
		if (pos[i] == 1 && flag == 1){
		    count ++;
		}
		else{
		    seg_end = i-1;
			
			if (count > 12 && count < 24){
			    /*two numbers*/
				pos[seg_start + count/2] = 0;
			}
			else if (count >= 24){
			    pos[seg_start + count/3] = 0;
				pos[seg_start + 2*count/3] = 0;
			}
			count = 0;
			flag = 0;
		}
		
	}
	
	return 0;
}

static unsigned char *ImageNorm(unsigned char *data,int width, int height)
{
    int i,j;
	int new_x, new_y;

    /*normalize the data to NORM_WIDTH X NORM_HEIGHT*/
    unsigned char *norm_data  = new unsigned char[NORM_WIDTH*NORM_HEIGHT];
	memset(norm_data,0,NORM_WIDTH*NORM_HEIGHT);

    for ( i = 0; i<width; i++){
	    for (j=0; j<height; j++){
		    new_x = (int)(NORM_WIDTH*i/width);
			new_y = (int)(NORM_HEIGHT*j/height);
			if (data[j*width + i] == 0){
			    norm_data[new_y*NORM_WIDTH + new_x] = 0;
			}
			else{
			    norm_data[new_y*NORM_WIDTH + new_x] = 1;
			}
		}
	}

    
	return norm_data;
}

unsigned int ImageSplit(unsigned char *data, int width, int height)
{
    int i = 0;
	int j = 0;
	int k = 0;
	int start = 0;
	int end   = 0;
    int *pos = new int[width];
	
	int l_width = ((width*3 + 3)>>2)<<2;
	
	RoughSplit(data,pos,width,height);
    
	
	while(i<width){
	    do{
		   i++;
		}while(pos[i] == 0);
		
		start = i;
		
		do{
		   i++;
		}while(pos[i] == 1);
		
		end = i-1;
		
		/*read the segmented data*/
		int new_width = end-start + 1;
#if 0
		unsigned char *one_ch = new unsigned char[height*new_width*3];
		int k = 0;
		for (k = 0; k<height; k++){
		    for (j=start; j<=end; j++){
		        one_ch[k*new_width*3 + (j-start)*3] = data[k*l_width + j*3];
				one_ch[k*new_width*3 + (j-start)*3 + 1] = data[k*l_width + j*3 + 1];
				one_ch[k*new_width*3 + (j-start)*3 + 2] = data[k*l_width + j*3 + 2];
		    }
		}
#endif	
        
        unsigned char *one_ch = new unsigned char[height*new_width];
        memset(one_ch,0,height*new_width);
		for (k=0; k<height; k++){
		    for (j=start; j<=end; j++){
			    int l = k*l_width + j*3;
				if (data[l] == 0 && data[l+1] == 0 && data[l+2] == 0){
				    one_ch[k*new_width + j-start] = 1;
				}
				else{
				    one_ch[k*new_width + j-start] = 0;
				}
			}
		}
		
#if 0
        for (k=0; k<height; k++){
		    for (j=0; j<new_width; j++){
			    if (one_ch[k*new_width + j] == 0){
				    printf("%d ",one_ch[k*new_width + j]);
				}else{
				    printf("%d ",one_ch[k*new_width + j]);
				}
			}
			printf("\n");
		}
		printf("\n\n\n");
#endif

                
		
		unsigned char *norm_data = new unsigned char[NORM_WIDTH*NORM_HEIGHT];
		
		norm_data = ImageNorm(one_ch,new_width,height);
		
		unsigned int *feature_vector = new unsigned int[MAX_FEATURE_LEN];
		memset(feature_vector,0,MAX_FEATURE_LEN);
		
		getfeatureVector(norm_data,feature_vector,NORM_WIDTH,NORM_HEIGHT);
#if 1	
        	
		for (k=NORM_HEIGHT-1; k>=0; k--){
		    for (j=0; j<NORM_WIDTH; j++){
			    if (norm_data[k*NORM_WIDTH + j] == 0){
				    printf("  ");
				}
				else{
			        printf("%d ",norm_data[k*NORM_WIDTH + j]);
				}
			}
			printf("\n");
		}
		printf("\n");
		for (k=0; k<MAX_FEATURE_LEN; k++){
		    printf("%d ",feature_vector[k]);
		}
		
		printf("\n\n\n");
#endif
        
	}
	
#if 1
    /*draw vertical bar*/
	for (i=0; i<width; i++){
	    if (pos[i] == 0){
            /*draw vertical bar for test*/
		    for (j=0; j<height; j++){
			    data[j*l_width + i*3] = 255; 
				data[j*l_width + i*3 +1 ] = 0;
				data[j*l_width + i*3 + 2] = 0;
			}
		}
	}
#endif

    
	return 0;
}


unsigned int getfeatureVector(unsigned char *data, unsigned int *vector,int image_width, int image_height)
{
    int i,j;
	/*split image into 3*3 blocks*/
//	___________________(thd_width,thd_height)
//	|   7 |  8  |  9  |
//	|_____|_____|_____|
//	|   4 |  5  |  6  |
//	|_____|_____|_____|
//	|   1 |  2  |  3  |
//	|_____|_____|_____|(thd_width,first_height)
// (first_width,first_height)	
    int first_height = image_height/3;
	int first_width  = image_width/3;
	
	int count = 0;
	int s,w;
	
	for (s = 0; s<3; s++){
	    for (w=0; w<3; w++){
		    count = 0;
		    for (i=s*first_height; i<(s+1)*first_height; i++){
			    for (j=w*first_width; j<(w+1)*first_width; j++){
				    if (data[i*image_width + j] == 1){
					    count++;
					}
				}
			}
			vector[3*s+w] = count;
		}
	}
	
	/*project to horizontal and vertical axis*/
	count = 0;
	for (i=0; i<image_height; i++){
	    for (j=0; j<image_width/2; j++){
		    if (data[i*image_width + j] == 1){
			     count++;
			}
		}
	}
	vector[9] = count;
	
	count = 0;
	for (i=0; i<image_height; i++){
	    for (j=image_width/2; j<image_width; j++){
		    if (data[i*image_width + j] == 1){
			    count++;
			}
		}
	}
	vector[10] = count;
	
	
	count = 0;
	for (i=0; i<image_height/2; i++){
	    for (j=0; j<image_width; j++){
		    if (data[i*image_width +j] == 1){
			    count++;
			}
		}
	}
	vector[11] = count;
	
	count = 0;
	for (i=image_height/2; i<image_height; i++){
	    for (j=0; j<image_width; j++){
		    if (data[i*image_width + j] == 1){
			    count++;
			}
		}
	}
	vector[12] = count;
	
}




