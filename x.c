#include <stdio.h>
#include <uv.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 6

#define filename "xx.txt"

#define CHK(x) if(x < 0) { fprintf(stderr, "error: %s\n", uv_strerror(x)); return; }

void on_open(uv_fs_t *r);
void on_read(uv_fs_t *r);
void on_stat(uv_fs_t *r);
void on_close(uv_fs_t *r); // did not work, and made no sense anyway.

int main(int argc, char** argv)
{
	uv_fs_t* r = (uv_fs_t*)malloc(sizeof(uv_fs_t));

	uv_loop_t * loop = uv_default_loop();

	uv_fs_stat(loop, r, filename, on_stat);

	uv_run(loop, UV_RUN_DEFAULT);

	return 0;
}

void on_stat(uv_fs_t *r)
{
	CHK(r->result)
	
	printf("\tname: %s\n\tsize: %zd\n", r->path, r->statbuf.st_size);

	uv_fs_open(r->loop, r, r->path, O_RDONLY, S_IRUSR, on_open);
}

void on_open(uv_fs_t *r)
{	
	CHK(r->result)
		
	//if to read whole file at a time.
	char* buf = (char*)malloc(r->statbuf.st_size + 1);
	memset(buf, 0, r->statbuf.st_size + 1);
	
	int size = r->statbuf.st_size;

	// jacking it for testing purposes
	size = 3;
	uv_buf_t iov = uv_buf_init(buf, size);
	// xxxra: for some reason here 1 is the only value that works
	// meaning it reads 1 char at a time.
	// however same read call works ok with buffer.len later on.
	uv_fs_read(r->loop, r, r->result, &iov, 1, 0 , on_read);
}

void on_read(uv_fs_t * r)
{
	// less than 0, then error.
	CHK(r->result)
	
	// no characters read close file
	if(r -> result == 0)
	{
		// could not close it here synchronously.
		// on_close was not getting called.
		//uv_fs_close(r->loop, r, r->result, on_close);

		uv_fs_close(r->loop, r, r->result, NULL);
		//printf("closed\n");

		return;
	}

	// (r->result > 0)
	// is it null terminated because the underlying buffer
	// is filesize below, but buflen was jacked to 3 characters
	// else it would've been crapping out.
	//printf("%s", (char*)r->bufsml->base);

	// on the other hand i am not thrilled about this approach either.
	// the aboveis probably better.
	//printf("%.*s", (int)r->bufsml->len, (char*)r->bufsml->base);

	// so will stick with this approach
	// wide string problem remains i suppose
	printf("%s", r->bufsml->base);

	//iov.len = req->result;
	//uv_fs_write(uv_default_loop(), &write_req, 1, &iov, 1, -1, on_write);
	
	// ++r->off has to be incremented by 1 else it gets stuck. 
	//but value gets updated to the value of buf.len/ chars read.
	// it can be read 1 char at a time as well.
	// reason why it was getting stuck is printf above was printing a charcater
	// and simply lying.
	uv_fs_read(r->loop, r, r->file, r->bufsml, r->bufsml->len, r->off += r->bufsml->len, on_read); 
}

void on_close(uv_fs_t *r)
{
	printf(	"Closing\r" );
}
