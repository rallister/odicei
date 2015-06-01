/*
======================================================================
Testing libuv reading/writing files.

	1) Write sample_text to file.
	2) Read file in using 3 char buffer into a a buffer size of file.
	3) Write it out to file out. (nah).	
	
======================================================================
*/

#include <stdio.h>
#include <uv.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>

char* sample_text =	"A0123456789^\n"
					"B0123456789^\n"
					"C0123456789^\n"
					"D0123456789^\n"
					"E0123456789^\n"
					"z\n"
					;

#define CHK(x) if(x < 0) { fprintf(stderr, "error: %s\n", uv_strerror(x)); return; }					
					
#define FILEIN "infile.txt"
#define FILEOUT "outfile.txt"

void on_stat(uv_fs_t *r);
void on_open(uv_fs_t *r);
void on_read(uv_fs_t *r);

void prep_infile()
{
	FILE *f = fopen(FILEIN, "w");
	fprintf(f, "%s", sample_text);
	fflush(f);
	fclose(f);
}

int main(int argc, char ** argv)
{
	prep_infile();
	
	uv_fs_t* r = (uv_fs_t*)malloc(sizeof(uv_fs_t));
	uv_loop_t * loop = uv_default_loop();
	uv_fs_stat(loop, r, FILEIN, on_stat);
	uv_run(loop, UV_RUN_DEFAULT);
	
	return 0;
}

void on_stat(uv_fs_t *r)
{
	CHK(r->result)	
	
	// still available here.
	printf("on_stat name: %s\tsize: %zd\n", r->path, r->statbuf.st_size);
	uv_fs_open(r->loop, r, r->path, O_RDONLY, S_IRUSR, on_open);	
}

void on_open(uv_fs_t *r)
{
	CHK(r->result)
	
	r->data = (char*)malloc(r->statbuf.st_size);
	memset(r->data, 0, r->statbuf.st_size);
	int size = 3; // buf size 3 for a laugh.
	
	char* buf = (char*)malloc(size + 1);
	memset(buf, 0, size + 1);
	
	// still available here.
	printf("on_open name: %s\tsize: %zd\n", r->path, r->statbuf.st_size);
	
	uv_buf_t iov = uv_buf_init(buf, size);
	uv_fs_read(r->loop, r, r->result, &iov, 1, 0 , on_read);
}

void on_read(uv_fs_t * r)
{
	CHK(r->result)
		
	if(r -> result == 0)
	{	// no more data.
		// r->path is null because it gets clobbered by uv_fs_read
		// which does INIT(READ) that kinda sucks.
		// at least statbuf is available.
		printf("on_read name: %s\tsize: %zd\n", r->path, r->statbuf.st_size);
		r->data -= (r->statbuf.st_size + 2);
		printf("%s", (char*)r->data);		
		
		// call back if provided does not get called.
		free(r->data);
		uv_fs_close(r->loop, r, r->result, NULL); 		
		uv_fs_req_cleanup(r);		
		return;
	}
	
	// copy to buffer that was created earlier
	strncpy(r->data, r->bufsml->base, r->bufsml->len);
	r->data += r->bufsml->len;
	
	// if there was no buffer can just call that
	//printf("%s", r->bufsml->base);
	
	uv_fs_read(r->loop, r, r->file, r->bufsml, r->bufsml->len, r->off += r->bufsml->len, on_read); 
}
