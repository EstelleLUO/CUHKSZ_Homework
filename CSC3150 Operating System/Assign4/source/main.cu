#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <device_launch_parameters.h>
#include <cuda_runtime.h>
#include <cuda.h>
#include <string.h>

#define DATAFILE "./data.bin"
#define OUTFILE "./snapshot.bin"

//Volume Control Block, which constains volume details
//Including #of blocks, #of free blocks, block size, free block pointers or array
#define SUPERBLOCK_SIZE 			4096 //4KB
//File Control Block, which is a Storage Structure consisting of information about a file
#define FCB_SIZE 					32 //32 bytes per FCB
#define FCB_ENTRIES 				1024
//Total size of avaible memory
#define STORAGE_SIZE 				1085440 //1060KB
#define STORAGE_BLOCK_SIZE 			32

#define MAX_FILENAME_SIZE 			20 //20 bytes
//max number of files 
#define MAX_FILE_NUM 				1024
//max size of file
#define MAX_ONE_FILE_SIZE 			1024
//max size of file name
#define MAX_FILE_NAME 				20

//The maximum size of file memory
#define MAX_FILE_SIZE 				1048576 //1024KB
#define BIT_TO_BYTE 				8 //Used in the conversion of bit to byte
//the start position of file memory
#define FILE_STORAGE_START			(STORAGE_SIZE-MAX_FILE_SIZE)
#define BITS_IN_BYTE 				8

#define OP_ERROR 					-1

#define TRUE 						1
#define FALSE 						0

//for valid/invalid FCB and valid/free block
#define VALID 						1
#define FREE 						0
#define INVALID 					0

//for bitmap
#define FREE_BLOCK_MASK 			0x1
#define FREE_BLOCK_BIT 				0
#define NON_FREE_BLOCK_MASK 		1

//used as read/write flag
#define G_READ 						0
#define G_WRITE 					1

//gsys flags
#define RM 							0
#define LS_D 						1
#define LS_S 						2

typedef unsigned char uchar;
typedef uint32_t u32;

//storing syatem information
typedef struct {
	uchar bitmap[SUPERBLOCK_SIZE];	       	//the bitmap recording free blocks
	u32 file_num ;	 					 	//number of files in the file system
	u32 file_list_time[MAX_FILE_NUM];	    //list files in order of decreasing modified time
	u32 file_list_size[MAX_FILE_NUM];	    //list files in order of decreasing file size
}FileSystem;

//FCB entry
typedef struct {
	char name[MAX_FILE_NAME];				//file name
	u32 valid_entry ;						//indicate whether this entry is valid
	u32 op ;								//the allowed operations of the file
	u32 time ;								//the last modified time of a file
	u32 block_num ;							//the index of its file block
	u32 file_size ;							//the size of a file
}FCB;

//FCB array pointer
__device__ FCB *fcb_table;

//system struct pointer
__device__ FileSystem *file_system;

//total storage 
__device__ uchar volume_d[STORAGE_SIZE];

/* Get the values for bitmap*/
__device__ u32 get_bitmap(u32 index)
{
	u32 start_pos = index / BITS_IN_BYTE;
	u32 offset = index%BITS_IN_BYTE;
	return ((file_system->bitmap[start_pos]) >> offset)&FREE_BLOCK_MASK;
}

/* Set the values for bitmap*/
__device__ void set_bitmap(u32 index, u32 flag)
{
	u32 start_pos = index / BITS_IN_BYTE;
	u32 offset = index%BITS_IN_BYTE;
	if (flag == VALID) file_system->bitmap[start_pos] = file_system->bitmap[start_pos] | (VALID << offset);
	else file_system->bitmap[start_pos] = file_system->bitmap[start_pos] & (~(VALID << offset));
}

/* Compare if two file names are the same */
__device__ bool compare_name(const char *dest, const char *src)
{
	int index = 0;
	while (index<MAX_FILE_NAME) {
		if (src[index] != dest[index]) return false;
		else if (src[index] == '\0' && dest[index] == '\0') return true;
		index++;
	}
	return true;
}


/* Copy file names*/
__device__ void cpy_filename(char *dest, const char *src)
{
	u32 index = 0;
	while (src[index] != '\0') {
		if (index<MAX_FILE_NAME) dest[index] = src[index];
		else {
			printf("The file name exceeds the maximum file name length\n");
			break;
		}
		index++;
	}
	dest[index] = '\0';
}

/* Open Function Implementation*/
__device__ u32 open(const char *s, int op)
{
	/* Implement open operation here */
	//the index of the FCB entry of a file
	u32 file_fcb = -1;
	//the index of a free FCB entry
	u32 free_fcb = -1;
	//Find file is whether exist in FCB or not
	for (int i = 0; i<MAX_FILE_NUM; i++) {
		if (compare_name(fcb_table[i].name, s) ) {
			//If found
			if (fcb_table[i].valid_entry == VALID) {
				file_fcb = i;
				fcb_table[i].op = op;
				return file_fcb;
			}
		}
		if (fcb_table[i].valid_entry == FREE) free_fcb = i;
	}

	//if not found
	if (file_fcb ==-1) {
		//the index of a free block
		u32 free_block = -1;
		//search the bitmap for free block
		for (int i = 0; i<MAX_FILE_NUM; i++) {
			if (get_bitmap(i) == FREE) {
				free_block = i;
				break;
			}
		}
		//if there is a free block, create a new FCB and record its block number
		if (free_block != -1) {
			//renew modified time of other files
			for (int i = 0; i<MAX_FILE_NUM; i++) {
				if (fcb_table[i].valid_entry == VALID) fcb_table[i].time++;
			}
			//set the modified time of the new file
			fcb_table[free_fcb].time = 0;
			fcb_table[free_fcb].op = op;
			//set the FCB entry valid
			fcb_table[free_fcb].valid_entry = TRUE;
			//make FCB point to the free block
			fcb_table[free_fcb].block_num = free_block;
			//set the file name
			cpy_filename(fcb_table[free_fcb].name, s);
			//set bitmap to indicate it is occupied
			set_bitmap(free_block, VALID);
			//renew total number of files
			file_system->file_num++;

			//renew file lists
			file_system->file_list_time[file_system->file_num - 1] = free_fcb;
			file_system->file_list_size[file_system->file_num - 1] = free_fcb;

			return free_fcb;
		}
		//if no free blocks are available, return error;
		else {
			printf("no free block\n");
			return OP_ERROR;
		}
	}
}

/* Remove Function Implementation */
__device__ void rm(const char *fileName)
{
	//the FCB entry of the to-be-removed file
	u32 file_fcb = -1;
	//search for FCB by file name
	for (int i = 0; i<MAX_FILE_NUM; i++) {
		if (compare_name(fcb_table[i].name, fileName)) {
			file_fcb = i;
			break;
		}
	}
	//if found
	if (file_fcb != -1) {
		//the real position of the file
		u32 file_start = fcb_table[file_fcb].block_num*FCB_ENTRIES;
		//the modified time of the file
		u32 time = fcb_table[file_fcb].time;
		u32 flag = FALSE;

		//remove file in file list
		for (int i = 0; i<file_system->file_num; i++) {
			if (file_system->file_list_time[i] == file_fcb) flag = TRUE;
			if (flag == TRUE && i != file_system->file_num - 1)
				file_system->file_list_time[i] = file_system->file_list_time[i + 1];
			else if (flag == TRUE && i == file_system->file_num - 1)
				file_system->file_list_time[i] = 0;
		}
		flag = FALSE;
		for (int i = 0; i<file_system->file_num; i++) {
			if (file_system->file_list_size[i] == file_fcb) flag = TRUE;
			if (flag == TRUE && i != file_system->file_num - 1)
				file_system->file_list_size[i] = file_system->file_list_size[i + 1];
			else if (flag == TRUE && i == file_system->file_num - 1)
				file_system->file_list_size[i] = 0;
		}

		//reset system info
		file_system->file_num--;
		set_bitmap(fcb_table[file_fcb].block_num, FREE);
		//reset modified time of other files
		for (int i = 0; i<MAX_FILE_NUM; i++) {
			if (fcb_table[i].valid_entry == VALID) {
				if (fcb_table[i].time>time)
					fcb_table[i].time--;
			}
		}
		//clear file content
		for (int i = 0; i<MAX_ONE_FILE_SIZE; i++)
			volume_d[FILE_STORAGE_START + file_start + i] = 0;
		//reset FCB block
		fcb_table[file_fcb].valid_entry = FREE;
		fcb_table[file_fcb].op = G_READ;
		fcb_table[file_fcb].time = 0;
		fcb_table[file_fcb].block_num = 0;
		fcb_table[file_fcb].file_size = 0;
		for (int j = 0; j<MAX_FILE_NAME; j++)
			fcb_table[file_fcb].name[j] = 0;

	}
	//if not found
	else printf("Cannot find file %s\n", fileName);
}

/* Write Function Implementation */
__device__ u32 write(const uchar *input, u32 size, u32 fp)
{
	//if file is not in write op, return error
	if (fcb_table[fp].op != G_WRITE) {
		printf("%s is not in write op\n", fcb_table[fp].name);
		return OP_ERROR;
	}
	u32 file_start = fcb_table[fp].block_num*FCB_ENTRIES;
	u32 previous_time = fcb_table[fp].time;
	u32 count;
	if (size < MAX_ONE_FILE_SIZE) count = size;
	else count = MAX_ONE_FILE_SIZE;
	//if bytes to write is more than max file size
	if (size>MAX_ONE_FILE_SIZE)
		printf("Cannot write more than 1024 bytes in a file\n");
	//write the file
	for (int i = 0; i<count; i++)
		volume_d[FILE_STORAGE_START + file_start + i] = input[i];
	//renew the file size in FCB
	fcb_table[fp].file_size = count;
	//renew modified time in FCB
	for (int i = 0; i<MAX_FILE_NUM; i++) {
		if (fcb_table[i].valid_entry == VALID && fcb_table[i].time <= previous_time) {
			fcb_table[i].time++;
		}
	}
	fcb_table[fp].time = 0;

	//renew file lists
	for (int i = 0; i<file_system->file_num - 1; i++) {
		if (fcb_table[file_system->file_list_time[i]].time <= previous_time)
			file_system->file_list_time[i] = file_system->file_list_time[i + 1];
	}
	file_system->file_list_time[file_system->file_num - 1] = fp;

	u32 start_idx = -1;
	u32 end_idx = file_system->file_num;
	u32 flag = FALSE;
	for (int i = 0; i<file_system->file_num; i++) {
		if (file_system->file_list_size[i] == fp) start_idx = i;
		if (fcb_table[file_system->file_list_size[i]].file_size <= count && flag == FALSE){
			flag = TRUE;
			end_idx = i;
		}
	}
	//if we don't find the final position, set it the tail of list
	if (end_idx>start_idx) {
		u32 temp = file_system->file_list_size[start_idx];
		for (int i = start_idx; i<end_idx - 1; i++)
			file_system->file_list_size[i] = file_system->file_list_size[i + 1];
		file_system->file_list_size[end_idx - 1] = temp;
	}
	else if (end_idx<start_idx) {
		u32 temp = file_system->file_list_size[start_idx];
		for (int i = start_idx; i>end_idx; i--)
			file_system->file_list_size[i] = file_system->file_list_size[i - 1];
		file_system->file_list_size[end_idx] = temp;
	}

	//return number of bytes written
	return count;
}

/* Read Function Implementation */
__device__ u32 read(uchar *output, u32 size, u32 fp)
{
	//if file is not in read op, return error
	if (fcb_table[fp].op != G_READ) {
		printf("%s is not in read op\n", fcb_table[fp].name);
		return OP_ERROR;
	}

	u32 file_start = fcb_table[fp].block_num*FCB_ENTRIES;
	u32 count;
	if (size < fcb_table[fp].file_size) count = size;
	else count = fcb_table[fp].file_size;
	//if bytes to read is more than max file size
	if (size>fcb_table[fp].file_size)
		printf("Cannot read more than file size\n");
	//read the file
	for (int i = 0; i<count; i++)
		output[i] = volume_d[FILE_STORAGE_START + file_start + i];
	//return number of bytes read
	return count;
}


/* LS_D and LS_S Implementation */
__device__ void gsys(int op)
{
char *name;
u32 size=0;
/* Implement LS_D and LS_S operation here */
//LS_D Operation
if (op==LS_D){
printf("===sort by modified time===\n");

for(int i=file_system->file_num-1;i>=0;i--){
name = fcb_table[file_system->file_list_time[i]].name;
printf("%s\n",name);
}
}
//LS_S Operation
else if(op==LS_S){
printf("===sort by file size===\n");

for(int i=0;i<file_system->file_num;i++){
name=fcb_table[file_system->file_list_size[i]].name;
size=fcb_table[file_system->file_list_size[i]].file_size;
printf("%s %d\n",name,size);
}

}
else printf("The command is invalid\n");
}

/* RM Implementation */
__device__ void gsys(int op, char *s)
{
/* Implement rm operation here */
if (op==RM){
rm(s);
}
else printf("The command is invalid\n");
}


__host__ void write_binaryFile(char *fileName, void *buffer, int bufferSize)
{

FILE *fp;
fp = fopen(fileName, "wb");
fwrite(buffer, 1, bufferSize, fp);
fclose(fp);
}

__host__ int load_binaryFile(char *fileName, void *buffer, int bufferSize)
{
FILE *fp;
fp = fopen(fileName, "rb");

if (!fp)
{
printf("***Unable to open file %s***\n", fileName);
exit(1);
}

//Get file length
fseek(fp, 0, SEEK_END);
int fileLen = ftell(fp);
fseek(fp, 0, SEEK_SET);

if (fileLen > bufferSize)
{
printf("****invalid testcase!!****\n");
printf("****software warrning: the file: %s size****\n", fileName);
printf("****is greater than buffer size****\n");
exit(1);
}

//Read file contents into buffer
fread(buffer, fileLen, 1, fp);
fclose(fp);
return fileLen;
}


__device__ void init_volume()
{
	file_system = (FileSystem *)volume_d;
	fcb_table = (FCB *)(volume_d + sizeof(*file_system));

	for (int i = 0; i<MAX_FILE_NUM / BITS_IN_BYTE; i++)
		file_system->bitmap[i] = 0;
	file_system->file_num = 0;
	for (int i = 0; i<MAX_FILE_NUM; i++) {
		file_system->file_list_time[i] = 0;
		file_system->file_list_size[i] = 0;
	}

	for (int i = 0; i<MAX_FILE_NUM; i++) {
		fcb_table[i].valid_entry = FREE;
		fcb_table[i].op = G_READ;
		fcb_table[i].time = 0;
		fcb_table[i].block_num = 0;
		fcb_table[i].file_size = 0;
		for (int j = 0; j<MAX_FILE_NAME; j++) {
			fcb_table[i].name[j] = 0;
		}
	}
}

__global__ void mykernel(uchar *input, uchar *output)
{
	init_volume();
	/**************************************
	* Test Case 1
	***************************************/
	// kernel test start  
	u32 fp = open("t.txt\0", G_WRITE);
	write(input, 64, fp);

	fp = open("b.txt\0", G_WRITE);
	write(input + 32, 32, fp);

	fp = open("t.txt\0", G_WRITE);
	write(input + 32, 32, fp);

	fp = open("t.txt\0", G_READ);
	read(output, 32, fp);

	gsys(LS_D);
	gsys(LS_S);

	fp = open("b.txt\0", G_WRITE);
	write(input + 64, 12, fp);

	gsys(LS_S);
	gsys(LS_D);
	gsys(RM, "t.txt\0");
	gsys(LS_S);
	// kernel test end
	/*/
	/**************************************
	* Test Case 2
	**************************************
	//kernel test start

	u32 fp = open("t.txt\0", G_WRITE);
	write(input, 64, fp);

	fp = open("b.txt\0", G_WRITE);
	write(input+32, 32, fp);

	fp = open("t.txt\0", G_WRITE);
	write(input+32, 32, fp);

	fp = open("t.txt\0", G_READ);
	read(output, 32, fp);

	gsys(LS_D);
	gsys(LS_S);

	fp = open("b.txt\0", G_WRITE);
	write(input+64, 12, fp);

	gsys(LS_S);
	gsys(LS_D);
	gsys(RM, "t.txt\0");
	gsys(LS_S);

	char fname[10][20];
	for(int i = 0; i < 10; i++)
	{
	fname[i][0] = i+33;
	for(int j = 1; j < 19; j++)
	fname[i][j] = 64+j;
	fname[i][19] = '\0';
	}
	for(int i = 0; i < 10; i++)
	{
	fp = open(fname[i], G_WRITE);
	write(input+i, 24+i, fp);
	}
	gsys(LS_S);
	for(int i = 0; i < 5; i++)
	gsys(RM, fname[i]);
	gsys(LS_D);
	// kernel test end
	*/
	/**************************************
	* Test Case 3
	**************************************
	//kernel test start
	u32 fp = open("t.txt\0", G_WRITE);
	write(input, 64, fp);
	fp = open("b.txt\0", G_WRITE);
	write(input+32, 32, fp);
	fp = open("t.txt\0", G_WRITE);
	write(input+32, 32, fp);
	fp = open("t.txt\0", G_READ);
	read(output, 32, fp);
	gsys(LS_D);
	gsys(LS_S);
	fp = open("b.txt\0", G_WRITE);
	write(input+64, 12, fp);
	gsys(LS_S);
	gsys(LS_D);
	gsys(RM, "t.txt\0");
	gsys(LS_S);

	char fname[10][20];
	for(int i = 0; i < 10; i++)
	{
	fname[i][0] = i+33;
	for(int j = 1; j < 19; j++)
	fname[i][j] = 64+j;
	fname[i][19] = '\0';
	}
	for(int i = 0; i < 10; i++)
	{
	fp = open(fname[i], G_WRITE);
	write(input+i, 24+i, fp);
	}
	gsys(LS_S);
	for(int i = 0; i < 5; i++)
	gsys(RM, fname[i]);
	gsys(LS_D);
	char fname2[1018][20];
	int p = 0;
	for(int k = 2; k < 15; k++)
	for(int i = 50; i <= 126; i++, p++)
	{
	fname2[p][0] = i;
	for(int j = 1; j < k; j++)
	fname2[p][j] = 64+j;
	fname2[p][k] = '\0';

	}
	for(int i = 0 ; i < 1001; i++)
	{
	fp = open(fname2[i], G_WRITE);
	write(input+i, 24+i, fp);
	}
	gsys(LS_S);

	fp = open(fname2[1000], G_READ);
	read(output+1000, 1024, fp);

	char fname3[17][3];
	for(int i = 0; i < 17; i++)

	{
	fname3[i][0] = 97+i;
	fname3[i][1] = 97+i;
	fname3[i][2] = '\0';
	fp = open(fname3[i], G_WRITE);
	write(input+1024*i, 1024, fp);

	}
	fp = open("EA\0", G_WRITE);
	write(input+1024*100, 1024, fp);
	gsys(LS_S);
	//kernel test end
	*/
}

/************************************************************************************
*
* Main function
*
************************************************************************************/
int main()
{
	uchar *input_h;
	uchar *input;

	uchar *output_h;
	uchar *output;

	input_h = (uchar *)malloc(sizeof(uchar)* MAX_FILE_SIZE);
	output_h = (uchar *)malloc(sizeof(uchar)* MAX_FILE_SIZE);


	cudaMalloc(&input, sizeof(uchar)* MAX_FILE_SIZE);
	cudaMalloc(&output, sizeof(uchar)* MAX_FILE_SIZE);

	// load binary file from data.bin
	load_binaryFile(DATAFILE, input_h, MAX_FILE_SIZE);


	cudaMemcpy(input, input_h, sizeof(uchar)* MAX_FILE_SIZE, cudaMemcpyHostToDevice);
	cudaMemcpy(output, output_h, sizeof(uchar)* MAX_FILE_SIZE, cudaMemcpyHostToDevice);

	mykernel << <1, 1 >> >(input, output);

	cudaMemcpy(output_h, output, sizeof(uchar)* MAX_FILE_SIZE, cudaMemcpyDeviceToHost);

	// dump output array to snapshot.bin 
	write_binaryFile(OUTFILE, output_h, MAX_FILE_SIZE);

	cudaDeviceSynchronize();
	cudaDeviceReset();

	return 0;

}
