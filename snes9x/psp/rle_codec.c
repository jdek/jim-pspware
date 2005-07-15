#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int  rle_encode (const void* data,     const unsigned int data_len, void* buf, unsigned int buf_len);
int  rle_decode (const void* rle_data, const unsigned int rle_len,  void* buf, unsigned int buf_len);
void dump       (const void* data,     const unsigned int data_len);


#ifndef PSP
int main (int argc, char** argv)
{
	FILE* file;
	char* filename;
	int   filesize;
	char  filedata [327680];

	char* rle_output;
	int   rle_length;

	char* rld_output;
	int   rld_length;

	unsigned int i,j;
	
	if (argc < 2)
	{
		printf ("This program requires a filename...\n");
		return -1;
	}

	filename = argv [1];
	file     = fopen (filename, "rb");
	
	if (! file) {
		printf ("Could not open file: '%s'!\n", filename);
		return -1;
	}

	fseek (file, 0, SEEK_END);
	filesize = ftell (file);
	rewind (file);

	fread  (filedata, filesize, 1, file);
	fclose (file);

	rle_output = rle_encode (filedata, filesize, &rle_length);
	printf ("Size before: %d\n"
	        "Size after:  %d\n"
		"Space saved: %2.2f%%\n",
		    filesize,
		        rle_length,
			    100.0f * (1.0f - ((float)rle_length / (float)filesize)));

//	dump (rle_output, rle_length);

	rld_output = rle_decode (rle_output, rle_length, &rld_length);
	printf ("Runlength Decode Size: %d\n", rld_length);

	j = 0;
	for (i = 0; i < filesize; i++) {
		j += (unsigned int)filedata [i];
	}

	printf ("Checksum Before: %X\n", j);

	j = 0;
	for (i = 0; i < rld_length; i++) {
		j += (unsigned int)rld_output [i];
	}

	printf ("Checksum After:  %X\n", j);



	return 0;
}
#endif

int rle_encode (const void* data, const unsigned int data_len, void *buf, unsigned int buf_len)
{
	unsigned int rle_len;

	char *input,
	     *end,
	     *output;

	unsigned char val;
	unsigned char last_val;
	unsigned char repeat_count;

	input  = (char *)data;
	end    = (input + data_len);

	output  = (char *)buf;
	rle_len = 0;

	repeat_count = 0;
	last_val     = 0xff;

	while ((input <= end) && (rle_len < buf_len))
	{
		val = *input++;

		if (val == last_val) {
			if (repeat_count == 255) {
				*output++ = (repeat_count);
				rle_len++;

				repeat_count = 0;
			}
			else {
				if (repeat_count == 0) {
					*output++ = val;
					rle_len++;
				}

				repeat_count++;
			}
		} else {
			if (repeat_count > 0) {
				*output++ = (repeat_count - 1);
				rle_len++;
			}

			repeat_count = 0;

			*output++ = last_val = val;
			rle_len++;
		}
	}

	if (input == end) {
		if (repeat_count > 0) {
			*output++ = (repeat_count - 1);
			rle_len++;
		}
	}

	return rle_len;
}

int rle_decode (const void* rle_data, const unsigned int rle_len, void *buf, unsigned int buf_len)
{
	unsigned int data_len;

	char *input,
	     *end,
	     *output;

	unsigned char val;
	unsigned char last_val;
	unsigned char num_repeats;

	input  = (char *)rle_data;
	end    = (input + rle_len);

	output   = (char *)buf;
	data_len = 0;

	last_val = 0xff;

	while ((input <= end) && (data_len < buf_len))
	{
		val = *input++;

		if (val == last_val) {
			*output++ = val;
			data_len++;

			num_repeats = *(unsigned char *)input++;
			memset (output, val, num_repeats);

			output   += num_repeats;
			data_len += num_repeats;
		} else {
			last_val  = val;
			*output++ = val;
			data_len++;
		}
	}

	return data_len;
}

#ifndef PSP
// Generic dump, for unknown data where lexical analysis is needed.
void dump (const void* data, const unsigned int len)
{
   unsigned width;
   char *str;
   unsigned int j, i;

  printf ("Size:\t%d\n", len);
  
  if (len > 0) {
    width = 16;
    str   = (char *)data;
    j = i = 0;

    while (i < len) {
      printf (" ");

      for (j = 0; j < width; j++) {
        if (i + j < len)
          printf ("%02x ", (unsigned char) str [j]);

        else
          printf ("   ");

        if ((j + 1) % (width / 2) == 0)
          printf (" -  ");
      }

      for (j = 0; j < width; j++) {
        if (i + j < len)
          printf ("%c", isprint (str [j]) ? str [j] : '.');

        else
          printf (" ");
      }

      str += width;
      i   += j;

      printf ("\n");
    }
  }
}
#endif
