/* gif2bmp.c  -- See end-of-file for (C)*/
/* This was written for Microsoft 32-bit-integer compilers--it
 * should compile with
 *	cl gif2bmp.c
 * to produce the executable gif2bmp.exe
 *
 * The Microsoft oddities are:
 *   The funny include files -- they may be omitted on compilers
 *   that can not find them (stdio.h better be there, however)
 * #pragma pack(1) -- This forces the BITMAPFILEHEADER structure
 *   to be the same length in memory as in files (otherwise, the structure
 *   is padded out to be a multiple of 8? bytes long
 * Note that Microsoft uses all-caps for typedefs.  Ugh.
 */
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <math.h>
typedef unsigned char byte;
typedef unsigned short WORD;
typedef unsigned long DWORD;

#pragma pack(1)
typedef struct tagRGBQUAD {
	byte	rgbBlue;
	byte	rgbGreen;
	byte	rgbRed;
	byte	rgbReserved;
} RGBQUAD;
typedef struct tagBITMAPFILEHEADER {
	WORD	bfType;
	DWORD	bfSize;
	WORD	bfReserved1;
	WORD	bfReserved2;
	DWORD	bfOffBits;
} BITMAPFILEHEADER;
typedef BITMAPFILEHEADER *PBITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER{
	DWORD	  biSize;
	DWORD	  biWidth;
	DWORD	  biHeight;
	WORD	  biPlanes;
	WORD	  biBitCount;

	DWORD	  biCompression;
	DWORD	  biSizeImage;
	DWORD	  biXPelsPerMeter;
	DWORD	  biYPelsPerMeter;
	DWORD	  biClrUsed;
	DWORD	  biClrImportant;
} BITMAPINFOHEADER;

BITMAPFILEHEADER hdr;
BITMAPINFOHEADER bi;
long wib;
RGBQUAD cmap[256];
FILE *fp;
int bit8= 0;

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
void pm_error(char *msg){
	fprintf(stderr, "%s\n", msg);
	exit(5);
}
/* Output .bmp header */
int spew_header(int wi, int hi, int n, RGBQUAD *cmap){
	/* Writes to the global fp */

	if(n>16 || bit8)
		bi.biBitCount= 8;
	else if(n>2)
		bi.biBitCount= 4;
	else
		bi.biBitCount= 1;
	n= 1 << ((int)bi.biBitCount);
	bi.biSize= sizeof(bi);
	bi.biWidth= wi;
	bi.biHeight= hi;
	bi.biPlanes= 1;
	bi.biCompression= 0;
	wib= dib_wib((int)bi.biBitCount, (int)bi.biWidth);
	bi.biSizeImage= bi.biHeight * wib;
	bi.biClrUsed= n;
	bi.biClrImportant= n;
	bi.biXPelsPerMeter= 0;
	bi.biYPelsPerMeter= 0;

	hdr.bfType= 0x4d42;	/* BM */
	hdr.bfReserved1= 0;
	hdr.bfReserved2= 0;
	hdr.bfOffBits = sizeof(hdr) + bi.biSize + n*sizeof(RGBQUAD);
	hdr.bfSize= hdr.bfOffBits + bi.biSizeImage;

	fwrite(&hdr,1,sizeof(hdr),fp);
	fwrite(&bi, 1, sizeof(bi), fp);
	if(n>0)
		fwrite(cmap, sizeof(RGBQUAD), n, fp);
	return 1;
}

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

int    verbose;
int    showComment;

static void ReadGIF ( FILE	 *fd, int imageNumber );
static int ReadColorMap ( FILE *fd, int number, RGBQUAD *b);
static int DoExtension ( FILE *fd, int label );
static int GetDataBlock ( FILE *fd, unsigned char  *buf );
static int GetCode ( FILE *fd, int code_size, int flag );
static int LWZReadByte ( FILE *fd, int flag, int input_code_size );
static void ReadImage ( FILE *fd, int len, int height, RGBQUAD *cmap, int gray, int interlace, int ignore );

char *note[]= {
	"Usage: oldfile.gif newfile.bmp",
	"Convert gif image to bmp image",
	"Options:",
	"-8    Output 8-bit per pixel .bmp image",
	"-v    Verbose",
	"-c    Comments",
	"-i #  Get the #th image in the gif",
	0};
int main(int argc, char **argv){
	FILE		*in;
	int		imageNumber;
	char *cp;
	char *srcname= 0;
	char *dstname= 0;

	imageNumber = 1;
	verbose = FALSE;
	showComment = FALSE;

	while(cp= *++argv)if(*cp++=='-')switch(*cp++){
	case '8':	bit8= 1; break;
	case 'v':	verbose= TRUE;
	case 'c':	showComment= TRUE;
			break;
	case 'i':	imageNumber= atoi(*++argv); break;
	default: err:	for(argv= note; *argv; ++argv)
				fprintf(stderr, "%s\n", *argv);
			exit(0);
	}else if(srcname==0)
		srcname= *argv;
	else if(dstname==0)
		dstname= *argv;
	else goto err;
	if(dstname==0)
		goto err;

	in= fopen(srcname, "rb");
	if(in==0){
		perror(srcname);
		exit(1);
	}
	fp= fopen(dstname, "wb");
	if(fp==0){
		perror(dstname);
		exit(1);
	}
	ReadGIF(in, imageNumber);
	fclose(in);
	fclose(fp);
	return 0;
}
static void ReadGIF(FILE *fd, int imageNumber){
	unsigned char buf[16];
	unsigned char c;
	static RGBQUAD localColorMap[MAXCOLORMAPSIZE];
	int useGlobalColormap;
	int bitPixel;
	int imageCount = 0;
	char version[4];

	if (! ReadOK(fd,buf,6))
		pm_error("error reading magic number" );

	if (strncmp((char *)buf,"GIF",3) != 0)
		pm_error("not a GIF file" );

	strncpy(version, (char *)buf + 3, 3);
	version[3] = '\0';

	if ((strcmp(version, "87a") != 0) && (strcmp(version, "89a") != 0))
		pm_error("bad version number, not '87a' or '89a'" );

	if (! ReadOK(fd,buf,7))
		pm_error("failed to read screen descriptor" );

	GifScreen.Width 	  = LM_to_uint(buf[0],buf[1]);
	GifScreen.Height	  = LM_to_uint(buf[2],buf[3]);
	GifScreen.BitPixel	  = 2<<(buf[4]&0x07);
	GifScreen.ColorResolution = (((buf[4]&0x70)>>3)+1);
	GifScreen.Background	 = buf[5];
	GifScreen.AspectRatio	 = buf[6];

	if (BitSet(buf[4], LOCALCOLORMAP)) {	/* Global Colormap */
		if (ReadColorMap(fd,GifScreen.BitPixel, GifScreen.ColorMap))
			pm_error("error reading global colormap" );
	}
	/*
	if (GifScreen.AspectRatio != 0 && GifScreen.AspectRatio != 49) {
		float	r;
		r = ( (float) GifScreen.AspectRatio + 15.0 ) / 64.0;
		fprintf(stderr,"warning - non-square pixels; to fix do a 'pnmscale -%cscale %g'\n",
		  r < 1.0 ? 'x' : 'y',
		  r < 1.0 ? 1.0 / r : r );
	}
	*/
	for (;;) {
		if (! ReadOK(fd,&c,1))
			pm_error("EOF / read error on image data" );

		if (c == ';') {	 /* GIF terminator */
			if (imageCount < imageNumber){
				fprintf(stderr, "only %d image%s found in file\n",
					imageCount, imageCount>1?"s":"" );
				exit(1);
			}
			return;
		}

		if (c == '!') {	 /* Extension */
			if (! ReadOK(fd,&c,1))
				pm_error("OF / read error on extention function code");
			DoExtension(fd, c);
			continue;
		}

		if (c != ',') {	 /* Not a valid start character */
			fprintf(stderr,"bogus character 0x%02x, ignoring\n", (int) c );
			continue;
		}

		++imageCount;

		if (! ReadOK(fd,buf,9))
			pm_error("couldn't read left/top/width/height");

		useGlobalColormap = ! (buf[8] & LOCALCOLORMAP);

		bitPixel = 1<<((buf[8]&0x07)+1);

		if (! useGlobalColormap) {
			if (ReadColorMap(fd, bitPixel, localColorMap))
				pm_error("error reading local colormap" );
			ReadImage(fd, LM_to_uint(buf[4],buf[5]),
				LM_to_uint(buf[6],buf[7]),
				localColorMap, bitPixel,
				buf[8]&INTERLACE, imageCount != imageNumber);
		} else {
			ReadImage(fd, LM_to_uint(buf[4],buf[5]),
				LM_to_uint(buf[6],buf[7]),
				GifScreen.ColorMap, GifScreen.BitPixel,
				buf[8]&INTERLACE, imageCount != imageNumber);
		}

	}
}

static int ReadColorMap(FILE *fd, int number, RGBQUAD *buffer){
	int i;
	unsigned char rgb[3];

	for (i = 0; i < number; ++i, buffer++) {
		if (! ReadOK(fd, rgb, sizeof(rgb)))
			pm_error("bad colormap" );
		buffer->rgbRed= rgb[0];
		buffer->rgbGreen= rgb[1];
		buffer->rgbBlue= rgb[2];
		buffer->rgbReserved= 0;
	}
	return 0;
}
static int DoExtension(FILE *fd, int label){
	static char	buf[256];
	char		*str;

	switch (label) {
	case 0x01:		/* Plain Text Extension */
		str = "Plain Text Extension";
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
		str = "Application Extension";
		GetDataBlock(fd, (unsigned char*) buf);
		if (showComment){
			fprintf(stderr, "Application Extension: %c%c%c%c%c%c%c%c ",
				buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7]);
			fprintf(stderr, " Authentication Code=)%02x %02x %02x\n",
				buf[8], buf[9], buf[10]);
		}
		break;
	case 0xfe:		/* Comment Extension */
		str = "Comment Extension";
		while (GetDataBlock(fd, (unsigned char*) buf) != 0) {
			if (showComment)
				fprintf(stderr,"gif comment: %s\n", buf );
		}
		return FALSE;
	case 0xf9:		/* Graphic Control Extension */
		str = "Graphic Control Extension";
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
static int GetCode(FILE *fd, int code_size, int flag){
	static unsigned char	buf[280];
	static int		curbit, lastbit, done, last_byte;
	int			i, j, ret;
	unsigned char		count;

	if (flag) {
		curbit = 0;
		lastbit = 0;
		done = FALSE;
		return 0;
	}

	if ( (curbit+code_size) >= lastbit) {
		if (done) {
			if (curbit >= lastbit)
				pm_error("ran off the end of my bits" );
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
			if (code == table[0][code])
				pm_error("circular table entry BIG ERROR");
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
static void ReadImage(FILE *fd, int len, int height, RGBQUAD *cmap,
 int bpp, int interlace, int ignore){
	unsigned char	c;
	int		v;
	int		xpos = 0, ypos = 0, pass = 0;
	unsigned char *scanline;

	/*
	**  Initialize the Compression routines
	*/
	if (! ReadOK(fd,&c,1))
		pm_error("EOF / read error on image data" );

	if (LWZReadByte(fd, TRUE, c) < 0)
		pm_error("error reading image" );

	/*
	**  If this is an "uninteresting picture" ignore it.
	*/
	if (ignore) {
		if (verbose)
			fprintf(stderr,"skipping image...\n" );

		while (LWZReadByte(fd, FALSE, c) >= 0)
			;
		return;
	}
	if ((scanline= (unsigned char *)malloc(len)) == NULL)
		pm_error("couldn't alloc space for image" );


	if (verbose)
		fprintf(stderr,"reading %d by %d%s GIF image\n",
			len, height, interlace ? " interlaced" : "" );
	spew_header(len, height, bpp, cmap);

	/* Fill the whole file with junk */
	for(v= 0; v<height; v++)
		fwrite(scanline, 1, (int)wib, fp);

	while (ypos<height && (v = LWZReadByte(fd,FALSE,c)) >= 0) {
		switch(bi.biBitCount){
		case 1:
			if(v)
				scanline[xpos>>3] |= 128 >> (xpos&7);
			else
				scanline[xpos>>3] &= 0xff7f >> (xpos&7);
			break;
		case 4:
			if(xpos&1)
				scanline[xpos>>1] |= v&15;
			else
				scanline[xpos>>1] = (v&15) << 4;
			break;
		case 8:
			scanline[xpos]= v;
			break;
		}
		++xpos;
		if (xpos == len) {
			fseek(fp, -(ypos+1)*wib, SEEK_END);
			fwrite(scanline, 1, (int)wib, fp);
			xpos = 0;
			if (interlace) {
				static int dpass[]= {8,8,4,2};
				ypos += dpass[pass];
				if (ypos >= height) {
					static int restart[]= {0,4,2,1,32767};
					ypos= restart[++pass];
				}
			} else
				++ypos;
		}
	}
	if(LWZReadByte(fd, FALSE,c) >= 0)
		fprintf(stderr,"too much input data, ignoring extra...\n");
}
/* +-------------------------------------------------------------------+ */
/* | Copyright 1990, 1991, 1993, David Koblas.  (koblas@netcom.com)    | */
/* +-------------------------------------------------------------------+ */
/*****************************************************************************\
*               Windows functions, types, and definitions                     *
*               Version 3.10                                                  *
*               Copyright (c) 1985-1992, Microsoft Corp. All rights reserved. *
*******************************************************************************
*
*(C)1996 Ephraim Cohen--subject to the above
** permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notices appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/