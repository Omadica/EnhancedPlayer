#include <stdio.h> 
#include "fisheyedll.h"

#define ALIGN4(X)		( ( ( X ) + 3 ) / 4 * 4 )


void LoadFisheyeImg(BITMAPINFOHEADER *infoHeader, unsigned char ** inputFile)
{
	FILE * iFile = fopen("fisheye.bmp", "rb");
	int error = 0;
	char buf[14];

	if(iFile)
	{
		int count = 0;

		count = fread(buf, 1, 14, iFile);	//read bitmapfileheader;

		if(!count)
		{
			error = 1;
		}

		if(!error)
		{
			count = fread(infoHeader, sizeof(BITMAPINFOHEADER), 1, iFile);
			if(!count)
			{
				error = 1;
				printf("error1\n");
			}
		}
		if(!error)
		{
			//printf("%dx%d\n", infoHeader->biWidth, infoHeader->biHeight);
			*inputFile = new unsigned char [ALIGN4(infoHeader->biWidth * 4) * infoHeader->biHeight  ];
			fread(*inputFile, 1, ALIGN4(infoHeader->biWidth * 4 ) * infoHeader->biHeight, iFile);
		}

		fclose(iFile);
	}
	else
	{
		error = 1;
	}
	if(error)
	{
		printf("failed to get fisheye.bmp\n");
	}
	
} 
void WriteImg(char * fileName, BITMAPINFOHEADER *info_header, unsigned char *outBuf)
{
	FILE * oFile = fopen(fileName, "wb+");
	unsigned int fileSize =  40 + 14 + ALIGN4(info_header->biWidth * 3) * info_header->biHeight;
	unsigned int reserved = 0;
	unsigned int offset = 54;

	fwrite("BM", 1, 2, oFile);
	fwrite(&fileSize, 4, 1, oFile);
	fwrite(&reserved, 4, 1, oFile);
	fwrite(&offset, 4, 1, oFile);

	fwrite(info_header, sizeof(BITMAPINFOHEADER), 1, oFile);
	fwrite(outBuf, 1, fileSize - offset, oFile);
	fclose(oFile);
}

int main()
{
	int fd;

	unsigned char fish_imgBuf;
	BITMAPINFOHEADER fish_header;
	unsigned char * fish_buf = NULL;
	BITMAPINFOHEADER out_header;
	int error;
	while(true)
	{
		fd = Fisheye_NewObj();
		LoadFisheyeImg(&fish_header, &fish_buf);

		unsigned char out_buf[1200 * 1200 * 3] = {0};


		Fisheye_SetParameters(fd, 2592, 1944, 1287, 984, 2010, 477, FISHEYE_STEREOGRAPHIC, FISHEYE_NORMAL, &fish_header);

		//Fisheye_Dewarp(fd, -113, 76, 1.0f, &fish_header, fish_buf, &out_header, out_buf, 1200, 1200);        // ok
		//Fisheye_Wall_Dewarp(fd, -113, 76, 1.0f, &fish_header, fish_buf, &out_header, out_buf, 1200, 1200);   // ok
		//Fisheye_Wall_180_Panorama(fd, &fish_header, fish_buf, &out_header, out_buf, 1200, 1200);             // ok

		// it's just a small slab
		Fisheye_360_Standard(fd, 90, 1, 1, 1, 90, &fish_header, fish_buf, &out_header, out_buf, 1200, 1200); // leaks!


		Fisheye_Free(fd);
		printf("%s\n", "asdasd");


		delete fish_buf;
	}

	return 0;
}

