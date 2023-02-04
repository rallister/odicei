#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION


#include "stb_image.h"
#include "stb_image_resize.h"
#include "stb_image_write.h"

#include <stdio.h> 

#define JPEG_QUALITY 0

/* tries to avoid double evaluation like max(++a, ++b) but... 
    why would anyone actually do something like that to begin with.
    
#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })
     
*/
#define MAX(a,b) ((a) > (b) ? (a) : (b))


// I am guessing this is fast dividing by 255
//https://github.com/vsfteam/Gato/blob/bc5447e6c7c6ff0d8dcfa063341d9a59e640f30a/include/color.h#L8
// There was also a massive discussion on 
// https://stackoverflow.com/questions/35285324/how-to-divide-16-bit-integer-by-255-with-using-sse
// just remember in this func sign is important hence deals with -65k to +65k
#define idiv255(x) ((((int)(x) + 1) * 257) >> 16)


typedef struct {
    int last_pos;
    void *context;
} custom_stbi_mem_context;

static void custom_stbi_write_mem(void *context, void *data, int size) {
	
	// no buffer overflow check.

	// size could be 0:
	if (!size)
		return;
	
	custom_stbi_mem_context *c = (custom_stbi_mem_context*)context; 
	char *dst = (char *)c->context;
	char *src = (char *)data;
	int cur_pos = c->last_pos;
	for (int i = 0; i < size; i++) {
	   dst[cur_pos++] = src[i];
	}
	c->last_pos = cur_pos;
}

static void custom_stbi_write_printsize(void *context, void *data, int size) {   
   printf("%d ", size);
   if (size == 0)
        printf("\n");
}

int main(int argc, char ** argv)
{
	stbi_convert_iphone_png_to_rgb(1);
	
    int w,h,nchannels,stride = 0;   
    
    //  don't force channel number ... e.g., STBI_rgb_alpha leave it at 0.
	// but I had to load it here with 3 channels STBI_rgb, to discard alpha from odd png file.
	
    //stbi_uc *data = stbi_load("/home/kami/misc/downl/aa.jpg", &w, &h, &nchannels, 0);  
	stbi_uc *data = stbi_load("/home/kami/misc/_gam_pdf_run/bads/IMG_2272.png", &w, &h, &nchannels, STBI_rgb);  
    printf("w=%d,h=%d,n=%d \n", w, h, nchannels);
    
    int nw = MAX(w / 3, 150);
    int nh = MAX(h / 3, 350);
    nw = 150;
	nh = 350;
    int bufflen = nw * nh * STBI_rgb;
    char * out = calloc(1, bufflen); // sizeof here is colour struct uint8 r,g,b,a
    
	/*
    if (nchannels == 4)
	{		
		// set a of rgba pixel.
		int pix_comp = 4;
		for(int i = 0; i < w * h * nchannels; i+=4)
		{
			*(unsigned char*)(data + i + pix_comp) = 0;
		}
	}
	*/
    int ret = stbir_resize_uint8(data, w , h , stride,
                          out, nw, nh, stride, STBI_rgb); // force rgb

    printf("ret=%d \n", ret);
    
    stbi_write_png("nooo.png", nw, nh, STBI_rgb, out, stride); // force rgb 
    stbi_write_jpg("nooo.jpg", nw, nh, STBI_rgb, out, JPEG_QUALITY); // force rgb
    
    
    // write format to memory using custom_stbi_write_mem
    // then write to file/ stream?
    // https://github.com/nothings/stb/issues/1132#issuecomment-838579144
    // func a bit too long also no checks for buff overrun.
    custom_stbi_mem_context context;
    context.last_pos = 0;
    context.context = (void *)calloc(1, bufflen);
    
    ret = stbi_write_jpg_to_func(custom_stbi_write_mem, &context, nw, nh, STBI_rgb, out, JPEG_QUALITY);
    printf("ret=%d \n", ret);
    
    FILE* fout = fopen("nooo2.jpg", "w");
    fwrite(context.context, context.last_pos, 1, fout);
    fclose(fout);
    
    //===================================== 
    // JPG is written one char at a time.
	// started by writing n chars 25 64 1 64 24 16 12 1 16 162 1 16 12 1 16 162 14 1...
	// then switched to writing 1 byte at a time.
    //ret = stbi_write_jpg_to_func(custom_stbi_write_printsize, NULL, nw, nh, STBI_rgb, out, JPEG_QUALITY);
    //printf("\nret=%d \n", ret);
    
    // PNG is written all in one go.   
    //ret = stbi_write_png_to_func(custom_stbi_write_printsize, NULL, nw, nh, STBI_rgb, out, 0);
    //printf("\nret=%d \n", ret);
    
    // BMP
    // 1 1 4 2 2 4 4 4 4 2 2 4 4 4 4 4 4
	// then 18 63 blocks, followed by 3 & 0 sizes
	// 63 63 63 63 63 63 63 63 63 63 63 63 63 63 63 63 63 63 63 3 0
    //ret = stbi_write_bmp_to_func(custom_stbi_write_printsize, NULL, nw, nh, STBI_rgb, out);
    //printf("\nret=%d \n", ret);
    
	// TGA
    // 1 1 1 2 2 1 2 2 2 2 1 1	
	// then 62, 63 & 64 blocks ending with 38.
    //ret = stbi_write_tga_to_func(custom_stbi_write_printsize, NULL, nw, nh, STBI_rgb, out);
    //printf("\nret=%d \n", ret);
	
	// HDR expects float buffer. 
	// skip.
    //ret = stbi_write_hdr_to_func(custom_stbi_write_printsize, NULL, nw, nh, STBI_rgb, out);
    //printf("\nret=%d \n", ret);
	
    stbi_image_free(data);
    stbi_image_free(out);
    stbi_image_free(context.context);
    
    return 0;
}

