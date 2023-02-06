#include <turbojpeg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#define uint8_t u_int8_t

#define STB_IMAGE_STATIC
#define STBIR_DEFAULT_FILTER_DOWNSAMPLE  STBIR_FILTER_LANCZOS3

//#define STBIR_DEFAULT_FILTER_DOWNSAMPLE  STBIR_FILTER_MITCHELL
//#define STBIR_DEFAULT_FILTER_DOWNSAMPLE  STBIR_FILTER_CATMULLROM
//#define STBIR_DEFAULT_FILTER_DOWNSAMPLE  STBIR_FILTER_CUBICBSPLINE

#define STBI_ONLY_JPEG
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION


#include "../lib_stb_test/stb_image.h"
#include "../lib_stb_test/stb_image_resize.h"
#include "../lib_stb_test/stb_image_write.h"

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

/*
gcc -o ttest x.c -lturbojpeg

*/

// https://github.com/libjpeg-turbo/libjpeg-turbo
// https://github.com/libjpeg-turbo/libjpeg-turbo/blob/main/turbojpeg.h
// https://cpp.hotexamples.com/zh/examples/-/-/tjDecompressHeader2/cpp-tjdecompressheader2-function-examples.html
// https://github.com/LuaDist/libjpeg/blob/master/example.c
// https://stackoverflow.com/questions/43092625/tjdecompress2-takes-more-time-to-execute-if-called-after-a-delay

int ra_readfile(char * filename, unsigned char ** buffer);
int ra_tj_get_scaling_factor(
	tjscalingfactor* scalingFactors, 
	int scalingFactorCount, 
	int jpegWidth, int jpegHeight, 
	int targetWidth, 
	int targetHeight, 
	int largerThanRequired );
	
int main()
{
	char * infile_name = "/home/kami/misc/treasure_dog/TreasurePicks/Treasure_to_site/20221006_204641.jpg";
	
	unsigned char* compressedImage;	
	unsigned long jpegSize = ra_readfile(infile_name, &compressedImage);
	
	printf("%ld\n", jpegSize);
	
	if (compressedImage == NULL)
		printf("compressedImage is null\n");
	if (jpegSize == 0 || compressedImage == NULL)
		return 0;	
	
	// from https://stackoverflow.com/questions/9094691/examples-or-tutorials-of-using-libjpeg-turbos-turbojpeg
	const int JPEG_QUALITY = 60;
	const int COLOR_COMPONENTS = 3; //rgb.
	
	int jpegSubsamp, width, height;
	
	
	tjhandle decompressor = tjInitDecompress();
	
	if(tjDecompressHeader2(decompressor, compressedImage, (unsigned long)jpegSize, &width, &height, &jpegSubsamp))
	{
		printf ("Err : tjDecompressHeader2 - %s\n", tjGetErrorStr());
	}
	else
	{	
		printf("jsize=%ld, width=%d, height=%d, nchannels=%d\n", jpegSize, width, height, jpegSubsamp);
	}
	
	unsigned char * uncompressedBuffer = NULL; //calloc(width* height * COLOR_COMPONENTS,1);
	unsigned char * compressedBuffer = NULL;//calloc(width* height * COLOR_COMPONENTS,1);
	
	/*
	 
	 funny :
		https://stackoverflow.com/questions/7488048/libjpeg-output-scaling
		... Just iterate dx and dy according to the dest_width and dest_height. But really, use a library for this.
		The other guy probably "I thought I was..."
	 this is promising.
		https://github.com/xtech/self-o-mat/blob/8665485f25b65d284753621a26880d0d196f79b5/src/tools/JpegDecoder.cpp
		TJSCALED(dimension, scalingFactor);
		tj3SetScalingFactor(decompressor,tjscalingfactor scalingFactor);
	*/
	
	// attempt at scaling prior to decompress.
	// should this be intialized on every call tjGetScalingFactors?
	int scalingFactorCount;
	tjscalingfactor* scalingFactors = tjGetScalingFactors(&scalingFactorCount);
	printf ("scalingFactorCount=%d, isnull(scalingFactors)= %s\n", scalingFactorCount, scalingFactors? "no": "yes"); // here printed out 16.
	
	if (scalingFactorCount  != 0)
	{
		int indx = ra_tj_get_scaling_factor(
			scalingFactors, 
			scalingFactorCount, 
			width, height, 
			800, // make it bigger than target for stb to work with.
			800, // 
			1 );
			
		printf("indx=%d\n", indx);
		
		if (indx > 0)
		{
			tjscalingfactor factor = scalingFactors[indx];
			int scaledWidth = TJSCALED(width, factor); // return those from scaling factor?  
			int scaledHeight = TJSCALED(height, factor);
			// 3 is rgb, part.
			int nebuflen = 3 * scaledWidth * scaledHeight;
			printf("nebuflken=%d, sw = %d, sh = %d\n", nebuflen, scaledWidth, scaledHeight);
			
			width = scaledWidth;
			height = scaledHeight;
		}
	}
	else
	{	// or.
		//( 2 without scaling)
		//tjDecompress2(decompressor, compressedImage, jpegSize, uncompressedBuffer, width, 0/*pitch*/, height, TJPF_RGB, TJFLAG_FASTDCT);
	}

	uncompressedBuffer = calloc(width* height * COLOR_COMPONENTS,1);
	//uncompressedBuffer = malloc(width* height * COLOR_COMPONENTS);

	tjDecompress2(decompressor, compressedImage, jpegSize, uncompressedBuffer, width, 0/*pitch*/, height, TJPF_RGB, TJFLAG_ACCURATEDCT  );
	
	tjDestroy(decompressor);
	free(compressedImage);
	
	//===============================
	// fuck sake easier to try dropping stb in here than to find how to resize it.
	// turns out you can't.
	//===============================
	int nw = 403;
    int nh = 302;
	    
    int bufflen = nw * nh * 3;
	
    char * out = calloc(bufflen, 1); 
	int ret = stbir_resize_uint8(uncompressedBuffer, width , height , 0, out, nw, nh, 0, STBI_rgb);
	
	//char * out = uncompressedBuffer; 
	//nw = width;
	//nh = height;
	
	// ======================================================
	
	tjhandle compressor = tjInitCompress();

	
	tjCompress2(compressor, out, nw, 0, nh, TJPF_RGB,
          &compressedBuffer, &jpegSize, jpegSubsamp, JPEG_QUALITY,
          TJFLAG_ACCURATEDCT); // TJSAMP_444, TJSAMP_422, <-jpegSubsamp
		  
	FILE * img_out = fopen("testout.jpg", "w");
	fwrite(compressedBuffer,1, jpegSize, img_out);
	fclose(img_out);
	
	tjDestroy(compressor);
	tjFree(uncompressedBuffer);	
	free(compressedBuffer);
	//free(out);
	return 0;
}


int ra_readfile(char * filename, unsigned char ** buffer)
{
	struct stat sb;	
	FILE * fd = NULL;
	int ret = 0;
	if (stat(filename, &sb) == 0 && sb.st_size > 0) 
	{  
		ret = (int)sb.st_size;				
		if(fd = fopen(filename, "r"))
		{			
			*buffer = malloc(sb.st_size);
			if (*buffer && fread(*buffer, 1, ret, fd) == ret)
				goto ok;
		}
    }
	
	not_ok:
		if(*buffer)
			free(*buffer);
	ok:
		if(fd)
			fclose(fd);
	return ret;
}


int ra_tj_get_scaling_factor(
	tjscalingfactor* scalingFactors, 
	int scalingFactorCount, 
	int jpegWidth, 
	int jpegHeight, 
	int targetWidth, 
	int targetHeight, 
	int largerThanRequired )
{
	// adapted from:
	// https://github.com/xtech/self-o-mat/blob/8665485f25b65d284753621a26880d0d196f79b5/src/tools/JpegDecoder.h
	
	int scalingFactorIdx = -1;
	
	// Check, if we need to scale. 
	// We only scale down so in order to scale both dimensions must be larger than the target
    if (jpegHeight > targetHeight && jpegWidth > targetWidth) 
	{
		printf("looking for scaling factor\n");
		// Find the scaling factor
		// maybe could return last if exact was found, instead of -1.
		// xxxra: width is main factor, so could find something more optimal than this.
		if (largerThanRequired)
		{	
			// So if scaling factors 16, it will go down to 756 x 1008.
			// and will find it			
			int bestDist = MAX(jpegWidth, jpegHeight);
			for (int i = 0; i < scalingFactorCount; i++) 
			{
				
				int scaledHeight = TJSCALED(jpegHeight, scalingFactors[i]);
				int scaledWidth = TJSCALED(jpegWidth, scalingFactors[i]);
				
				printf("1 -- looking for scaling factor %d ... %d x %d\n", i, scaledHeight, scaledWidth);
				
				if (scaledHeight >= targetHeight && scaledWidth >= targetHeight) 
				{
					int dist = MIN(scaledHeight - targetHeight, scaledWidth - targetWidth);
					if (dist < bestDist) 
					{
						scalingFactorIdx = i;
						bestDist = dist;
					}
				}
			}
		}
		else
		{
			int bestDist = MAX(jpegWidth, jpegHeight);
			
			for (int i = 0; i < scalingFactorCount; i++) 
			{
				// So if scaling factors 16, it will go down to 378 x 504
				// and not find it, so really we could get last.
				int scaledHeight = TJSCALED(jpegHeight, scalingFactors[i]);
				int scaledWidth = TJSCALED(jpegWidth, scalingFactors[i]);
				printf("2 -- looking for scaling factor %d ... %d x %d\n", i, scaledHeight, scaledWidth);
				if (scaledHeight <= targetHeight && scaledWidth <= targetHeight) 
				{
					printf("looking for scaling factor %d ... condition hit.\n", i);
					int dist = MIN(targetHeight - scaledHeight, targetWidth - scaledWidth);
					if (dist < bestDist) 
					{
						scalingFactorIdx = i;
						bestDist = dist;
					}
				}
			}
		}
	}
	
	return scalingFactorIdx;
}