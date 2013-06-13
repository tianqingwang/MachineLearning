#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <math.h>
#include <string.h>
#include "gif.h"

typedef unsigned char byte;
typedef unsigned short WORD;
typedef unsigned long DWORD;

#pragma pack(1)

#define        MAXCOLORMAPSIZE	      256

#define        TRUE    1
#define        FALSE   0

#define CM_RED	      0
#define CM_GREEN       1
#define CM_BLUE 	      2

#define        MAX_LWZ_BITS	      12

#define INTERLACE	      0x40
#define LOCALCOLORMAP  0x80
#define BitSet(byte, bit)      (((byte) & (bit)) == (bit))
#define        ReadOK(file,buffer,len) (fread(buffer, len, 1, file) != 0)
#define LM_to_uint(a,b) 		    (((b)<<8)|(a))

typedef struct tagRGBQUAD {
	byte	rgbBlue;
	byte	rgbGreen;
	byte	rgbRed;
	byte	rgbReserved;
} RGBQUAD;

static struct {
       unsigned int    Width;
       unsigned int    Height;
	   RGBQUAD ColorMap[MAXCOLORMAPSIZE];
       unsigned int    BitPixel;
       unsigned int    ColorResolution;
       unsigned int    Background;
       unsigned int    AspectRatio;
       /*
       **
       */
       int	      GrayScale;
} GifScreen;

static struct {
       int     transparent;
       int     delayTime;
       int     inputFlag;
       int     disposal;
} Gif89 = { -1, -1, -1, 0 };

long wib;
int bitCount;

void pm_error(char *msg){
	fprintf(stderr, "%s\n", msg);
	exit(5);
}
#if 0
int main(int argc, char **argv)
{
    char *srcname = 0;
	
	srcname = argv[1];
	
	FILE *in = fopen(srcname,"rb");
	if (in == 0){
	    perror(srcname);
		exit(1);
	}
	
	int width,height;
	
	ReadGIFInfo(in,&width,&height);

	unsigned char *data = (unsigned char*)malloc(width*height*sizeof(unsigned char));
	ReadGIFData(in,data,width,height);
	
	int i,j;
	for (i=0; i<height; i++){
	    for (j=0; j<width; j++){
		    if (data[i*width + j] == 0){
			    printf("  ");
			}
			else {
			    printf("1 ");
			}
		}
		printf("\n");
	}
	
	printf("line=8\n");
	for (j=0; j<width; j++){
	     if (data[8*width + j] == 0){
		     printf("  ");
		 }
		 else{
		     printf("1 ");
		 }
	}
	printf("\n");
}
#endif

static int dib_wib(int bitcount, int wi){
	switch (bitcount){
	case 1: wi= (wi+31) >> 3; break;
	case 4: wi= (wi+7)  >> 1; break;
	case 8: wi=  wi+3; break;
	case 16:wi= (wi*2)+3; break;
	case 24:wi= (wi*3)+3; break;
	case 32:return wi*4;
	}
	return wi & ~3;
}

int    ZeroDataBlock = FALSE;

static int GetDataBlock(FILE *fd, unsigned char  *buf){
	unsigned char	count;

	if (! ReadOK(fd,&count,1)) {
		fprintf(stderr,"error in getting DataBlock size\n" );
		return -1;
	}

	ZeroDataBlock = count == 0;

	if ((count != 0) && (! ReadOK(fd, buf, count))) {
		fprintf(stderr,"error in reading DataBlock\n" );
		return -1;
	}

	return count;
}

static int DoExtension(FILE *fd, int label){
	static char	buf[256];
	char		*str;

	switch (label) {
	case 0x01:		/* Plain Text Extension */
		strcpy(str,"Plain Text Extension");
#ifdef notdef
		if (GetDataBlock(fd, (unsigned char*) buf) == 0)
			;

		lpos	= LM_to_uint(buf[0], buf[1]);
		tpos	= LM_to_uint(buf[2], buf[3]);
		width	= LM_to_uint(buf[4], buf[5]);
		height = LM_to_uint(buf[6], buf[7]);
		cellw	= buf[8];
		cellh	= buf[9];
		foreground = buf[10];
		background = buf[11];

		while (GetDataBlock(fd, (unsigned char*) buf) != 0) {
			PPM_ASSIGN(image[ypos][xpos],
					cmap[CM_RED][v],
					cmap[CM_GREEN][v],
					cmap[CM_BLUE][v]);
			++index;
		}

		return FALSE;
#else
		break;
#endif
	case 0xff:		/* Application Extension */
		strcpy(str,"Application Extension");
		GetDataBlock(fd, (unsigned char*) buf);
#if 0
		if (showComment){
			fprintf(stderr, "Application Extension: %c%c%c%c%c%c%c%c ",
				buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7]);
			fprintf(stderr, " Authentication Code=)%02x %02x %02x\n",
				buf[8], buf[9], buf[10]);
		}
#endif
		break;
	case 0xfe:		/* Comment Extension */
		strcpy(str,"Comment Extension");
		while (GetDataBlock(fd, (unsigned char*) buf) != 0) {
#if 0
	    	if (showComment)
				fprintf(stderr,"gif comment: %s\n", buf );
#endif
		}
		return FALSE;
	case 0xf9:		/* Graphic Control Extension */
		strcpy(str,"Graphic Control Extension");
		(void) GetDataBlock(fd, (unsigned char*) buf);
		Gif89.disposal    = (buf[0] >> 2) & 0x7;
		Gif89.inputFlag   = (buf[0] >> 1) & 0x1;
		Gif89.delayTime   = LM_to_uint(buf[1],buf[2]);
		if ((buf[0] & 0x1) != 0)
			Gif89.transparent = buf[3];

		while (GetDataBlock(fd, (unsigned char*) buf) != 0)
			;
		return FALSE;
	default:
		str = buf;
		sprintf(buf, "UNKNOWN (0x%02x)", label);
		break;
	}

	fprintf(stderr,"got a '%s' extension\n", str );

	while (GetDataBlock(fd, (unsigned char*) buf) != 0)
		;

	return FALSE;
}

static int GetCode(FILE *fd, int code_size, int flag){
	static unsigned char	buf[280];
	static int		curbit, lastbit, done, last_byte;
	int			i, j, ret;
	unsigned char		count;
	
	char *str;

	if (flag) {
		curbit = 0;
		lastbit = 0;
		done = FALSE;
		return 0;
	}

	if ( (curbit+code_size) >= lastbit) {
		if (done) {
			if (curbit >= lastbit)
			    strcpy(str,"ran off the end of my bits");
				pm_error(str);
			return -1;
		}
		buf[0] = buf[last_byte-2];
		buf[1] = buf[last_byte-1];

		if ((count = GetDataBlock(fd, &buf[2])) == 0)
			done = TRUE;

		last_byte = 2 + count;
		curbit = (curbit - lastbit) + 16;
		lastbit = (2+count)*8 ;
	}

	ret = 0;
	for (i = curbit, j = 0; j < code_size; ++i, ++j)
		ret |= ((buf[ i / 8 ] & (1 << (i % 8))) != 0) << j;

	curbit += code_size;

	return ret;
}

static int LWZReadByte(FILE *fd, int flag, int input_code_size){
	static int	fresh = FALSE;
	int		code, incode;
	static int	code_size, set_code_size;
	static int	max_code, max_code_size;
	static int	firstcode, oldcode;
	static int	clear_code, end_code;
	static int	table[2][(1<< MAX_LWZ_BITS)];
	static int	stack[(1<<(MAX_LWZ_BITS))*2], *sp;
	register int	i;
	
	char *str;

	if (flag) {
		set_code_size = input_code_size;
		code_size = set_code_size+1;
		clear_code = 1 << set_code_size ;
		end_code = clear_code + 1;
		max_code_size = 2*clear_code;
		max_code = clear_code+2;

		GetCode(fd, 0, TRUE);

		fresh = TRUE;

		for (i = 0; i < clear_code; ++i) {
			table[0][i] = 0;
			table[1][i] = i;
		}
		for (; i < (1<<MAX_LWZ_BITS); ++i)
			table[0][i] = table[1][0] = 0;

		sp = stack;

		return 0;
	} else if (fresh) {
		fresh = FALSE;
		do {
			firstcode = oldcode =
				GetCode(fd, code_size, FALSE);
		} while (firstcode == clear_code);
		return firstcode;
	}

	if (sp > stack)
		return *--sp;

	while ((code = GetCode(fd, code_size, FALSE)) >= 0) {
		if (code == clear_code) {
			for (i = 0; i < clear_code; ++i) {
				table[0][i] = 0;
				table[1][i] = i;
			}
			for (; i < (1<<MAX_LWZ_BITS); ++i)
				table[0][i] = table[1][i] = 0;
			code_size = set_code_size+1;
			max_code_size = 2*clear_code;
			max_code = clear_code+2;
			sp = stack;
			firstcode = oldcode =
					GetCode(fd, code_size, FALSE);
			return firstcode;
		} else if (code == end_code) {
			int		count;
			unsigned char   buf[260];

			if (ZeroDataBlock)
				return -2;

			while ((count = GetDataBlock(fd, buf)) > 0)
				;

			if (count != 0)
				fprintf(stderr,"missing EOD in data stream (common occurence)\n");
			return -2;
		}

		incode = code;

		if (code >= max_code) {
			*sp++ = firstcode;
			code = oldcode;
		}

		while (code >= clear_code) {
			*sp++ = table[1][code];
			if (code == table[0][code]){
			    strcpy(str,"circular table entry BIG ERROR");
				pm_error(str);
			}
			code = table[0][code];
		}

		*sp++ = firstcode = table[1][code];

		if ((code = max_code) <(1<<MAX_LWZ_BITS)) {
			table[0][code] = oldcode;
			table[1][code] = firstcode;
			++max_code;
			if ((max_code >= max_code_size) &&
				(max_code_size < (1<<MAX_LWZ_BITS))) {
				max_code_size *= 2;
				++code_size;
			}
		}

		oldcode = incode;

		if (sp > stack)
			return *--sp;
	}
	return code;
}

static int ReadColorMap(FILE *fd, int number, RGBQUAD *buffer){
	int i;
	unsigned char rgb[3];
	char *str;
	for (i = 0; i < number; ++i, buffer++) {
		if (! ReadOK(fd, rgb, sizeof(rgb))){
			strcpy(str,"bad colormap");
			pm_error(str);
		}
		buffer->rgbRed= rgb[0];
		buffer->rgbGreen= rgb[1];
		buffer->rgbBlue= rgb[2];
		buffer->rgbReserved= 0;
	}
	return 0;
}

void ReadGIFInfo(FILE *fd,int *width,int *height)
{
    unsigned char buf[16];
	unsigned char c;
	char version[4];
	char *str;
	
	if (!ReadOK(fd,buf,6)){
	    strcpy(str,"error reading magic number");
	    pm_error(str);
	}
	
	if (strncmp((char*)buf,"GIF",3) != 0){
	    strcpy(str,"not a GIF file");
	    pm_error(str);
	}
	
	strncpy(version,(char*)buf+3,3);
	version[3] = '\0';
	
	if ((strcmp(version,"87a") != 0) && (strcmp(version,"89a") != 0)){
	    strcpy(str,"bad version number, not '87a' or '89a'");
	    pm_error(str);
	}
	
	/*read logical screen*/
	if (!ReadOK(fd,buf,7)){
	    strcpy(str,"failed to read screen descriptor");
	    pm_error(str);
	}
	
	GifScreen.Width 	  = LM_to_uint(buf[0],buf[1]);
	GifScreen.Height	  = LM_to_uint(buf[2],buf[3]);
	GifScreen.BitPixel	  = 2<<(buf[4]&0x07);
	GifScreen.ColorResolution = (((buf[4]&0x70)>>3)+1);
	GifScreen.Background	 = buf[5];
	GifScreen.AspectRatio	 = buf[6];	
	
	if (BitSet(buf[4],LOCALCOLORMAP)){
	    if (ReadColorMap(fd,GifScreen.BitPixel,GifScreen.ColorMap)){
		    strcpy(str,"error reading global color map");
		    pm_error(str);
		}
	}
	
	if (!ReadOK(fd,&c,1)){
	    strcpy(str,"EOF / read error on image data");
	    pm_error(str);
	}
	
	if (c == ';'){/*0x3B, GIF terminator*/
	    return;
	}
	
	if (c == '!'){/*0x21, Extension*/
	    if (!ReadOK(fd,&c,1)){
		    strcpy(str,"OF / read error on extension function code");
		    pm_error(str);
		}
		DoExtension(fd,c);
	}
	
	if (!ReadOK(fd,buf,8)){
	    strcpy(str,"couldn't read left/top/width/height");
	    pm_error(str);
	}
	
	*width = LM_to_uint(buf[4],buf[5]);
	*height = LM_to_uint(buf[6],buf[7]);
	
	return;
}

static void ReadImage(FILE *fd,unsigned char *data,int len, int height, RGBQUAD *cmap, int bpp, int interlace)
{
   unsigned char c;
   int v;
   int xpos = 0, ypos=0, pass=0;
   unsigned char *scanline;
   char *str;
   
   if (!ReadOK(fd,&c,1)){
       strcpy(str,"EOF / read error on image data");
       pm_error(str);
   }
   
   if (LWZReadByte(fd,TRUE,c)<0){
       strcpy(str,"error reading image");
       pm_error(str);
   }
   
   if((scanline = (unsigned char*)malloc(len)) == NULL){
       strcpy(str,"couldn't allocate space for image");
       pm_error(str);
   }
   
   while(ypos < height && (v = LWZReadByte(fd,FALSE,c)) >= 0){
       switch(bitCount){
	       case 1:
		       if (v){
			       scanline[xpos>>3] |= 128 >> (xpos&7);
			   }
			   else{
			       scanline[xpos>>3] &= 0xff7f >> (xpos&7);
			   }
			   break;
		   case 4:
		       if (xpos&1){
			       scanline[xpos>>1] |= v&15;
			   }
			   else{
			       scanline[xpos>>1] = (v&15) << 4;
			   }
			   break;
		   case 8:
		       scanline[xpos] = v;
			   break;
	   }
	   
	   ++xpos;
	   
	   if (xpos == len){
	       int i=0; 
		   for (i=0; i<len; i++){
		       int gray = (int)(cmap[scanline[i]].rgbRed*30 + cmap[scanline[i]].rgbGreen*59+cmap[scanline[i]].rgbBlue*11 + 50)/100;
		       if (gray >= 230){
			       data[ypos*len+i] = 0;
			   }
			   else{
			       data[ypos*len+i] = 1;
			   }
		   }
	   
	       xpos = 0;
		   if (interlace){
		       static int dpass[] = {8,8,4,2};
			   ypos += dpass[pass];
			   if (ypos >= height){
			       static int restart[] = {0,4,2,1,32767};
				   ypos = restart[++pass];
			   }
		   }
		   else{
		       ++ypos;
		   }
	   }
   }
#if 0
   int i,j;
   for (i=0; i<height; i++){
       for (j=0; j<len; j++){
	       if (data[i*len + j]==0){
		       printf("  ");
		   }
		   else{
		       printf("1 ");
		   }
	   }
	   printf("\n");
   }
#endif
}

void ReadGIFData(FILE *fd,unsigned char *data, int width, int height)
{
    unsigned char buf[16];
	static RGBQUAD localColorMap[MAXCOLORMAPSIZE];
	int useGlobalColormap;
	int bitPixel;
	char *str;
	
	if (!ReadOK(fd,buf,1)){
	    strcpy(str,"couldn't read local flag");
	    pm_error(str);
	}
	
	useGlobalColormap = !(buf[0] & LOCALCOLORMAP);
	bitPixel = 1<<((buf[0]&0x07)+1);
	
	if (bitPixel > 16){
	    bitCount = 8;
	}
	else if (bitPixel > 2){
	    bitCount = 4;
	}
	else{
	    bitCount = 1;
	}
	
	wib = dib_wib(bitCount,width);
	
	if (!useGlobalColormap){
	    if (ReadColorMap(fd,bitPixel,localColorMap)){
		    strcpy(str,"error reading local colormap");
		    pm_error(str);
		}
		ReadImage(fd,data,width,height,localColorMap,bitPixel,buf[0]&INTERLACE);
	}
	else{
	    
	    ReadImage(fd,data,width,height,GifScreen.ColorMap,GifScreen.BitPixel,buf[0]&INTERLACE);
	}
	
}