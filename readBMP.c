
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>

#define PI 3.1415926535898

unsigned char * readBMP(char* filename);
unsigned int  getImageWidth(unsigned char *data,int width,int height);
unsigned int  getImageHeight(unsigned char *data,int width,int height);
unsigned int ImageRotation(unsigned char *data, int width, int height);

int main(int argc, char * argv[])
{
    char *filename = "showphone3.bmp";
	readBMP(filename);
	
	return 0;
}

unsigned char * readBMP(char* filename)
{
    int i = 0;
	FILE *f = fopen(filename,"rb");
	unsigned char info[54];
    fread(info, sizeof(unsigned char), 54, f); // read the 54-byte header
	
	int width = *(int*)&info[18];
	int height = *(int*)&info[22];
	
	
	int size =3*width*height;
	
	unsigned char* data = new unsigned char[size];
	unsigned char* rot_data = new unsigned char[4*size];
	fread(data,sizeof(unsigned char),size,f);
	fclose(f);
	
	for (i=0; i<size; i += 3){

	unsigned char tmp = data[i];
		data[i] = data[i+2];
		data[i+2] = tmp;
		
//		printf("r=%u,g=%u,b=%u\n",data[i],data[i+1],data[i+2]);
		int gray = (int)(((int)data[i])*30 + ((int)data[i+1])*59 + ((int)data[i+2])*11 + 50)/100;
		if (gray >= 200){/*white color*/
		    gray = 255;
			data[i] = 255;
			data[i+1] = 255;
			data[i+2] = 255;
		}
		else{/*black color*/
		    gray = 0;
			data[i] = 0;
			data[i+1] = 0;
			data[i+2] = 0;
		}

	}
	
//	int new_width = getImageWidth(data,width,height);
//	int new_height = getImageHeight(data,width,height);
    ImageRotation(data,width,height);	

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
	
	int new_x, new_y;
	unsigned char *rot_data = new unsigned char[3*width*height];
	
	memset(rot_data,255,3*width*height);
	
//	for (angle = 0; angle <= 30; angle ++){
        /*for test*/
		angle = 13;
		/*end test*/
	    for (i = 0; i < height; i++){
            for (j = 0; j<width; j++){
			    double arc_angle = angle*PI/180.0;
				if (data[i*width*3 + j*3] == 0 && data[i*width*3 + j*3 + 1] == 0 && data[i*width*3+j*3+2] ==0){
				    int new_x = (int)(j-i*tan(arc_angle));
					int new_y = i;
					
					new_x += 5;
					
					if (new_x >=0 && new_x < width){
					    rot_data[new_y*width*3+new_x*3] = data[i*width*3 + j*3];
				        rot_data[new_y*width*3+new_x*3 + 1] = data[i*width*3 + j*3 +1];
				        rot_data[new_y*width*3+new_x*3 + 2] = data[i*width*3 + j*3 +2];
					}
				}
			}
		}

        for (i=0; i<height; i++){
	        for (j=0; j<width; j++){
		        data[i*width*3 + j*3] = rot_data[i*width*3 + j*3];
			    data[i*width*3 + j*3 + 1] = rot_data[i*width*3 + j*3 + 1];
			    data[i*width*3 + j*3 + 2] = rot_data[i*width*3 + j*3 + 2];
		    }
	    }	
//	}
	
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




