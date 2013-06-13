#ifndef __GIF_H__
#define __GIF_H__

void ReadGIFInfo(FILE *fd,int *width,int *height);
void ReadGIFData(FILE *fd,unsigned char *data, int width, int height);

#endif
