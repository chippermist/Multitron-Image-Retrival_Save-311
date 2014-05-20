//Written by Chinmay Garg

// This is needed for certain functions to be available in #includes
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>

#include "driver.h"
#include "image.h"

/*
 * Opens the given file, reads the image data, and uses it to initialize an
 * image structure.  See the assignment writeup for a description of the 3II
 * image file format.
 */
int image_init(struct image *img, const char *fname)
{

	//opens file using low-level I/O
	int fd = open(fname, O_RDONLY, S_IRUSR);

	if(fd < 0)
	{
		//prints an error if any error finding the file
		perror("open");
		return 1;
	}

	// printf("\n------\n\n");

	//defining variables to be used
	int data_offset, comment_offset;
	uint16_t comment_size, mask_bits;
	int size;
	uint32_t magic_number;
	

	//checking if it's a valid file
	//seeking for the magic number
	lseek(fd, 0, SEEK_SET);
	//reading in the magic number
	read(fd, &magic_number, 4);

	//converting the magic number from network to host
	//since it's 2 bytes long, it is short
	magic_number = ntohl(magic_number);

	// printf("Magic Number: %x\n", magic_number);

	//checking magic number
	if(magic_number != 0x43530311)
	{
		perror("magic number");
		return -1;
	}

	//end of checking

	//seeking for width
	if(lseek(fd, 4, SEEK_SET) != 4)
	{
		return -1;
	}

	//reading in width
	if(read(fd, &img->w, 2) != 2)
	{
		return -1;
	}

	//converting width from network to host
	img->w = ntohs(img->w);

	if(img->w < 0)
	{
		perror("width");
		return -1;
	}

	// printf("width: %d\n", width );

	//seeking for height
	if(lseek(fd, 6, SEEK_SET) != 6)
	{
		return -1;
	}
	//reading in height
	if(read(fd, &img->h, 2) != 2)
	{
		return -1;
	}

	//converting height from network to host
	img->h = ntohs(img->h);

	if(img->h < 0)
	{
		perror("height");
		return -1;
	}

	// printf("height: %d\n", height);

	//calculating the size
	size = (img->w)*(img->h);
	// printf("size: %d\n", size);
	
	if(size < 0)
	{
		perror("size");
		return -1;
	}

	// free(img->data);

	//allocating data memory where size = width * height
	img->data = malloc(size);


	//getting the data location

	//seeking for data offset
	if(lseek(fd, 8, SEEK_SET) != 8)
	{
		return -1;
	}
	//reading in data offset
	if(read(fd, &data_offset, 4) != 4)
	{
		return -1;
	}

	//converting long network data offset into host
	data_offset = ntohl(data_offset);

	if(data_offset <= 0)
	{
		perror("data offset");
		return -1;
	}

	// printf("Data Offset: %d\n", (int)data_offset);

	//seeking for data
	if(lseek(fd, data_offset, SEEK_SET) != data_offset)
	{
		return -1;
	}
	//reading in data
	if(read(fd, img->data, size) != size)
	{
		return -1;
	}


	//Trying to seek and retrive comment length
	if(lseek(fd, 16, SEEK_SET) != 16)
	{
		return -1;
	}
	//readinf in comment size
	if(read(fd, &comment_size, 2) != 2)
	{
		return -1;
	}

	//reading and converting comment size to host
	comment_size = ntohs(comment_size);

	// printf("comment size: %d\n", comment_size);

	if(comment_size <= 0)
	{
		return -1;
	}

	//seeking for comment offset
	if(lseek(fd, 12, SEEK_SET) != 12)
	{
		return -1;
	}
	//reading comment offset
	if(read(fd, &comment_offset, 4) != 4)
	{
		return -1;
	}
	//converting comment offset into host
	comment_offset = ntohl(comment_offset);

	if(comment_offset <= 0)
	{
		return -1;
	}

	// printf("Comment Offset : %d\n", (int)comment_offset);

	//allocating the memory for comment
	// + 1 for null terminator
	img->comment = malloc(comment_size + 1);

	// printf("Comment Size (before): %d\n", (int)comment_size);

	//seeking for comment data
	if(lseek(fd, comment_offset, SEEK_SET) != comment_offset)
	{
		return -1;
	}
	//reading in comment data
	if(read(fd, img->comment, comment_size) != comment_size )
	{
		perror("img->comment");
		return -1;
	}

	//setting the null terminator
	img->comment[comment_size] = '\0';

	//pread(fd, &comment, comment_size, comment_offset);
	// printf("Comment Size (after): %d\n", (int)comment_size);
	
	// printf("%d, %d", sizeof(img->data), sizeof(img->comment));

	//mask

	//seeking for mask bits
	if(lseek(fd, 18, SEEK_SET) != 18)
	{
		return -1;
	}
	//reading in mask bits
	if(read(fd, &mask_bits, 2) != 2)
	{
		return -1;
	}
	// pread(fd, &mask_bits, 2, SEEK_CUR);

	//converting mask bits form network to host
	mask_bits = ntohs(mask_bits);

	// free(img->mask);
	img->mask = malloc(size);

	//memcpy for maskbits into mask
	//memcpy(img->mask, &mask_bits, size);

	// printf("Mask Bits: %d\n", mask_bits);

	//checking if there is  a mask bit set or not
	if(mask_bits == 4)
	{	
		//mask bit is set
		//seeking the mask data
		if(lseek(fd, (size+data_offset), SEEK_SET) != (size+data_offset))
		{
			return -1;
		}

		//reading the mask data
		if(read(fd, img->mask, size) != size)
		{
			return -1;
		}	
	}
	else
	{	
		//seeting empty memory to 0 when mask is not set
		memset(img->mask, 0xFF, size);
	}

	
	//closing the file
	if(close(fd) != 0)
	{
		return 1;
	}
	//seeting the file to -1 for no error and no reuse
	fd = -1;
	return 0;
}

/*
 * Destroys an image structure.  If any resources were allocated in image_init,
 * they should be released here.
 */
void image_destroy(struct image *img)
{	
	//freeing the image data
		free(img->data);
	//freeing the comment data
		free(img->comment);
	//freeing the mask data
		free(img->mask);
	
}

/*
 * Saves the image to the given file, using the 3II file format.  This should
 * overwrite and truncate the file if it already exists.
 */
int image_save(const struct image *img, const char *fname)
{
	//opening the file
	int fd = open(fname, O_CREAT | O_TRUNC | O_RDWR, 0644);

	// printf("\n---------\n\n");

	if(fd < 0)
	{
		perror("open");
		return 1;
	}
	// else
	// {
	// 	printf("File Open\n");
	// }

	//keeping track of how much data has been written
	//part of personal debugging :P
	int bytes_wrote = 0;
	
	//magic numeber definition
	uint32_t magic_number = 0x43530311;

	// printf("Magic Number size: %d\n", (int)sizeof(magic_number));

	//converting magic number from host to network
	magic_number = htonl(magic_number);

	//seeking to the beginning of file
	//not required. but still.
	if(lseek(fd, 0, SEEK_SET) != 0)
	{
		perror("seek");
		return -1;
	}

	//writing the magic number to the file
	//magic number = 2 bytes of data
	if(write(fd, &magic_number, sizeof(magic_number)) != sizeof(magic_number))
	{
		perror("magic number");
		return -1;
	}
	bytes_wrote += sizeof(magic_number);
	
	// //defining the size of the data to be written
	int size;
	int height = NULL, width = NULL;
	size = (img->w) * (img->h);

	// printf("Width: %d\nHeight: %d\n", img->w, img->h);

	//converting width from host to network
	width = htons(img->w);

	// lseek(fd, 4, SEEK_SET);
	
	//writing the width into the file
	if(write(fd, &width, 2) != 2)
	{
		perror("width write");
		return -1;
	}
	bytes_wrote += 2;

	//converting the height from host to network
	height = htons(img->h);

	// lseek(fd, 6, SEEK_SET);
	
	//writing the height to the file
	if(write(fd, &height, 2) != 2)
	{
		perror("height write");
		return -1;
	}

	bytes_wrote += 2;

	//defining the data offset. 20 bytes of file + the length of comment
	uint32_t data_offset = 20 + strlen(img->comment);

	// printf("data offset: %d\n", data_offset);

	//converting the data offset from host to network
	data_offset = htonl(data_offset);

	//writing the data offset to the file
	if(write(fd, &data_offset, 4) != 4)
	{
		perror("data offset error");
		return -1;
	}

	bytes_wrote += 4;

	//defining the comment offset
	uint32_t comment_offset = 20;

	// printf("comment offset %d\n", comment_offset);

	//converting the comment offset from host to network
	comment_offset = htonl(comment_offset);

	//writing the comment offset to the file
	if(write(fd, &comment_offset, 4) != 4)
	{
		perror("comment offset error");
		return -1;
	}


	bytes_wrote += 4;

	//defining the comment size
	//strlen to find the size of a string in C
	uint16_t comment_size = strlen(img->comment);


	// printf("comment size: %d\n", comment_size);

	//converting the comment from host to network
	comment_size = htons(comment_size);

	
	//writing the comment size to the file
	if(write(fd, &comment_size, 2) != 2)
	{
		perror("comment size");
		return -1;
	}

	bytes_wrote += 2;

	//defining blank
	uint32_t blank = 0xFF;

	// int mask = 0;
	uint16_t mask_present = 4, mask_empty = 0;

	//converting values to network from host
	mask_present = htons(mask_present);
	mask_empty = htons(mask_empty);

	// if(write(fd, &blank, 2) != 2)
	// {
	// 	perror("write mask");
	// 	return -1;
	// }

	//seeking for mask bits
	if(lseek(fd, 18, SEEK_SET) != 18)
	{
		perror("lseek mask bits");
		return -1;
	}

	//comparing if the mask bit is blank or not
	if(memcmp(img->mask, &blank, size) == 0)
	{
		// mask = 1;
		if(write(fd, &mask_empty, 2) != 2);
		{	
			perror("write mask bit- non empty");
			return -1;
		}
	}
	else
	{
		if(write(fd, &mask_present, 2) != 2)
		{
			perror("write mask bit- empty");
			return -1;
		}
	}

	bytes_wrote += 2;

	//lseek(fd, ntohl(comment_offset), SEEK_SET);

	//always assuming that mask has a bit and then writing it
	if(write(fd, img->comment, ntohs(comment_size)) != ntohs(comment_size))
	{
		perror("write comment");
		return -1;
	}
	
	bytes_wrote += strlen(img->comment);

	//lseek(fd, ntohl(data_offset), SEEK_SET);

	// printf("Bytes to write: %d\n", size);

	//writing the image data
	if(write(fd, img->data, size) != size)
	{
		perror("write data");
		return -1;
	}

	
	bytes_wrote += size;
	
	// if(mask == 4) 
	// {
	//writing the mask data

		if(write(fd, img->mask, size) != size)
		{
			perror("write mask");
			return -1;
		}

		bytes_wrote += size;

	// }
	// // write(fd, img->data, sizeof(img->data));

	//closing the file
	if(close(fd) != 0)
	{
		perror("close");
		return -1;
	}

	fd = -1;

	// printf("\n*********BYTES WROTE: %d\n", bytes_wrote);
	return 0;
}

/*
 * Draws an image onto the MultiTron, using the appropriate bitwise masking
 * operations.  Bits which are 1 in the mask should have the corresponding data
 * drawn, and bits which are 0 in the mask should be left as they were.
 */
int image_draw(const struct image *img, struct multitron *mtron, int x, int y)
{

	int i=0;
	//allocating memory for the screen data to be stored in
	//using calloc because setting data worth size each value 1 byte
	uint8_t *on_screen = calloc(img->w*img->h, 1);
	
	//**SOLUTIONS**//

	//when it's one it's from the data and
	//when it's 0 it is considered transparent
	//so its taken in from the data already there on the screen

	//possible solutions....&(AND) from screen and ~mask(NOT) 
	//and &(AND) between data and mask

	//final would be an |(OR) between both the data's calculated above

	//calling getrect and storing data from the screen into the buffer
	if(mtron_getrect(mtron, x, y, img->w, img->h, on_screen) != 0)
	{
		perror("mtron_getrect");
		return -1;
	}

	//applying the operation
	for(i=0; i<(img->w*img->h); i++)
	{
		//explained above
		on_screen[i] = ( on_screen[i] & ~(img->mask[i]) ) | (img->data[i] & img->mask[i]);
	}

	//calling putrect and writing the final data in the buffer
	//to the screen
	//ERROR HANDLING: assuming error handling taken care by the functions
	if(mtron_putrect(mtron, x, y, img->w, img->h, on_screen) != 0)
	{
		perror("mtron_putrect");
		return -1;
	}

	//free for the buffer data
	free(on_screen);

	return 0;
}
/*
 * BONUS
 * Retrieves pixel data from the MultiTron into an existing image structure.
 * Only retrieves the bits which are part of the mask; the rest of the bits
 * in the data should be 0.
 */
int image_get(struct image *img, struct multitron *mtron, int x, int y)
{
	int i = 0;

	//calling get rect and storing the data from the screen
	//into the struct as said by the bonus part
	//ERROR HANDLING taken care by the function itself
	if(mtron_getrect(mtron, x, y, img->w, img->h, img->data) != 0)
	{	
		perror("mtron_getrect");
		return -1;
	}

	//applying the operations
	for(i=0; i<(img->w*img->h); i++)
	{
		//only storing the data where the mask is 1 and all the rest
		//are being flushed out and set to 0
		img->data[i] = img->data[i] & (img->mask[i]);
	}

	return 0;
}

//end of file