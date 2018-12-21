#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <device_launch_parameters.h>
#include <cuda_runtime.h>
#include <cuda.h>

//page size is 32bytes
#define PAGESIZE 32
//32 KB in shared memory 
#define PHYSICAL_MEM_SIZE 32768
//128 KB in global memory
#define STORAGE_SIZE 131072

#define PT_ENTRIES 1024 

#define DATAFILE "./data.bin"
#define OUTFILE "./snapshot.bin"

typedef unsigned char uchar ; 
typedef uint32_t u32 ; 

//page table entries
__device__ int PAGE_ENTRIES = 0;
//count the pagefault times
__device__ int PAGEFAULT_NUM = 0; 

__device__ int currentTime = 0; 

//secondary memory
__device__ uchar storage[STORAGE_SIZE];

//date input and output 
__device__ uchar results[STORAGE_SIZE]; 
__device__ uchar input[STORAGE_SIZE]; 

//Function Declaration
__device__ u32 paging(uchar *buffer, u32 frameNum, u32 offset);
__device__ u32 getPageNum(u32 pt_entries);

//page table
extern __shared__ u32 pt[];
const uint32_t INVALID =0;
const uint32_t VALID =1;
const uint32_t PAGENUMMASK=(1 << 13)-2;

__device__ void init_pageTable( int entries ){
	for(int i = 0; i < entries; i++){
		pt[i] = INVALID; 
	}
}

__device__ uchar Gread( uchar *buffer, u32 addr){
	/* Complete Gread function to read value from data buffer */
	u32 frameNum = addr/PAGESIZE ; 
	u32 offset = addr%PAGESIZE ; 
	
	addr = paging(buffer, frameNum, offset) ;
	return buffer[addr] ; 	
}

__device__ void Gwrite( uchar *buffer, u32 addr, uchar value){
	/* Complete Gwrite function to write value to data buffer */
	u32 frameNum = addr/PAGESIZE ; 
	u32 offset =  addr%PAGESIZE ; 

	addr = paging(buffer, frameNum , offset) ; 
	buffer[addr] = value ; 
}

__device__ void snapshot( uchar *results, uchar* buffer, int offset, int input_size ){
	/* Complete snapshot function to load elements from data to result */
	for(int i = 0; i < input_size; i++ ){
		results[i] = Gread(buffer, i + offset) ; 
	}
}

__global__ void mykernel( int input_size ){
	//take shared memory as physical memory 
	__shared__ uchar data[PHYSICAL_MEM_SIZE];

	//get page table entries 
	int pt_entries = PHYSICAL_MEM_SIZE/PAGESIZE;
	
	//before first Gwrite or Gread 
	init_pageTable(pt_entries); 

	/* Gwrite / Gread starts */
	for(int i = 0; i < input_size; i++)
		Gwrite(data, i , input[i]); 
	
	for(int i = input_size - 1; i >= input_size - 32769; i-- )
		int value = Gread(data, i) ;

	snapshot( results, data, 0, input_size);

	printf("pagefault number is %d\n", PAGEFAULT_NUM) ;  
}

__device__ u32 getPageNum(u32 pt_entries){
	return (pt_entries & PAGENUMMASK) >>1;
}

__device__ u32 paging( uchar *buffer, u32 frameNum, u32 offset){
	u32 target ; 
	int pt_entries = PT_ENTRIES ;

	//Find if the target page exists
	for(int i = 0; i < pt_entries;i++){
		u32 pageNum = getPageNum(pt[i]); //Stores the logic page number of pt[i]

		if (pt[i] & 1==VALID){
			if (pageNum==frameNum){
				u32 tempTime=currentTime++;
				//update hit time
				pt[i]=(tempTime<<13)|(frameNum<<1)|VALID;
				return i*PAGESIZE+offset;
			}
		}
	}
	
	//Find if there is an empty entry 
	for(int i = 0; i < pt_entries;i++){
		//Bitwise checking the page table and find invalid entries and mark as pagefault
		if(pt[i] & 1==INVALID){
			PAGEFAULT_NUM++ ;
			//Update page table
			u32 tempTime = currentTime++ ;
			pt[i] = (tempTime << 13 ) | ( frameNum << 1 ) | 1 ; 
			return i * PAGESIZE + offset  ; 
		}
	}

	//Find a place for swapping in by the rule of LRU
	u32 leastTime=0xFFFFFFFF;
	for(int i = 0; i < pt_entries;i++){
		u32 mask = (u32)(-1);
		u32 currentTime  = (mask & pt[i]) >> 13 ;
		if(currentTime < leastTime){
			//The entry of the potential page to be swapped out
			target = i ;
			leastTime=currentTime;
		}
	}
	
	PAGEFAULT_NUM++ ;

	//The page number of the logical page to be swapped out
	u32 tarFrame = getPageNum(pt[target]);

	//The address of the target to be swapped out in the secondary memory
	u32 beginAddress = tarFrame * PAGESIZE; 
	for(int i = beginAddress, j = 0; j < PAGESIZE; i++, j++){
		//The address of the target to be swapped in in the physical memory
		u32 sharedAddress = target * PAGESIZE + j; 
		u32 curAddress = frameNum * PAGESIZE + j; 
		
		//Swap out
		storage[i] = buffer[sharedAddress];
		//Swap in		
		buffer[sharedAddress] = storage[curAddress];	
	}
	int tempTime = currentTime++ ; 
	pt[target] = ((tempTime) << 13 ) | ( frameNum << 1 ) | 1 ;
	return target * PAGESIZE + offset ;
}

__host__ void write_binaryFile(char *fileName, uchar *results, int bufferSize){
	FILE *fp;
	fp = fopen(fileName, "wb");
 
	fwrite(results,sizeof(uchar),bufferSize,fp);
	fclose(fp) ; 
}

__host__ int load_binaryFile(char *fileName, uchar *input, int bufferSize){
	FILE *fp;
	fp = fopen(fileName, "rb");

	if (!fp){
		printf("***Unable to open file %s***\n", fileName);
		exit(1);
	}

	fseek(fp, 0, SEEK_END);
	int fileLen = ftell(fp);
	
	fseek(fp, 0, SEEK_SET);

	//Read data from input file
	fread(input,sizeof(uchar),fileLen,fp); 

	if (fileLen > bufferSize){
		printf("****invalid testcase!!****\n");
		printf("****software warrning: the file: %s size****\n", fileName);
		printf("****is greater than buffer size****\n");
		exit(1);
	}

	fclose(fp);
	
	return fileLen;  
}

int main(){
	cudaError_t cudaStatus;
	int input_size = load_binaryFile(DATAFILE, input, STORAGE_SIZE);

	mykernel<<<1, 1, 16384>>>(input_size);
	cudaStatus = cudaGetLastError();
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "mykernel launch failed: %s\n", cudaGetErrorString(cudaStatus));
		return 0;
	}

	printf("input size: %d\n", input_size);

	cudaDeviceSynchronize() ; 
	cudaDeviceReset() ;

	write_binaryFile(OUTFILE, results, input_size);
	
	return 0 ; 
}
