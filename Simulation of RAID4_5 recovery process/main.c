#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/syscall.h>


pthread_mutex_t lock;
#define  chunkSize_  (1024*1024)
 #define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })
	 
pthread_cond_t  cond;
int counter_= 0;
int bit=0;
int lockThreads_=0;
int finishThreadRound_=0;

int maxNumOfBytes_=-1;
int maxNumOfThreads_=-1;
char * outPutPath_=NULL;
char*  outPutChunk_;//[chunkSize_]={0};
int outIndex_=0;
int fdOutPut_=-1;
int maxLen_=-1;
int threadCounter_=0;
int NUM_THREADS;


ssize_t  pThreadRead(int fd, int size , char buffer[]){
	ssize_t amountBits=0;
	int currentPos = lseek(fd, (size_t)0, SEEK_CUR);
	while (amountBits !=size){
		amountBits = read(fd, buffer , size);
		if (amountBits < 0) {
			return -1;
		}
		if(amountBits !=size){
			lseek(fd, currentPos, SEEK_SET);
		}
	}
	return (ssize_t) amountBits;
}

void  pThreadWrite (){
	int writeBytes=-1;
	writeBytes= write(fdOutPut_, outPutChunk_, maxNumOfBytes_);
	if(writeBytes!=maxNumOfBytes_){
		printf( "An error occurred in the write.\n");
		  free(outPutChunk_);

		exit(-1);
	}		
}

int getSize(int fd){
	int size =0;
	int currentPos = lseek(fd, (size_t)0, SEEK_CUR);
	size = lseek(fd, (size_t)0, SEEK_END);
	lseek(fd, currentPos, SEEK_SET);	
	if(size <0){
		printf( "An error occurred in the size check.\n");
		perror("Size check");
		  free(outPutChunk_);

		exit(-1);
	}
	return size;
}

void finishLastThread(){
	pThreadWrite();
	maxNumOfBytes_=-1;
	for(int j = 0; j < chunkSize_; j++)
		outPutChunk_[j] = 0;		
	threadCounter_=0;//2
	maxNumOfThreads_=maxNumOfThreads_-finishThreadRound_;
	pthread_cond_broadcast(&cond);
	finishThreadRound_ = 0;				
}

void* ReadCharFromFile(void* fileName){
	int size;
	int  amountBits;
	int fd;
	char chunkInput[chunkSize_+1];
	int rc=-1;
	rc++;
	int flag;
	//OPEN FILE
    fd = open((char*)fileName, O_RDONLY);
    if(fd < 0)
    {
        printf("File could not be opened.\n");
        perror("open");
		  free(outPutChunk_);

		exit(-1);
    }
	//Thread Works
	size = getSize(fd);
			//	printf("size %d, maxLEn_ %d  1\n",size,maxLen_);

	while(size > 0){
		flag=0;
		
		amountBits = pThreadRead(fd,min(size,chunkSize_) ,chunkInput);
		//printf("Read num of bytes %d  thread = %s\n",amountBits,(char*)fileName);

		if(amountBits <0){
			printf("File could not be read.\n");
			perror("read");
			  free(outPutChunk_);

			exit(-1);
		}			
		

		rc = pthread_mutex_lock(&lock);
		if(maxLen_<size){

			maxLen_=size;		
		}
				size = size -amountBits;		

		if(amountBits>maxNumOfBytes_){
			maxNumOfBytes_=amountBits;	
		}	
		for(int i=0;  i<amountBits; i++){
			outPutChunk_ [i]= chunkInput[i]^outPutChunk_ [i];//XOR		
		}				
		threadCounter_++;				
		if(size == 0){
			finishThreadRound_++;
		}
		
		if(maxNumOfThreads_ ==	threadCounter_){//LastPointer
			finishLastThread();
			flag =1 ;
			
		}
		if(!flag){
	
			pthread_cond_wait(&cond ,&lock);//Not last
			
		}
				pthread_mutex_unlock(&lock);			

	}		
		close(fd);									
		pthread_exit( (void*) fileName);

}


//====================================================
int main ( int argc, char** argv )
{
  NUM_THREADS = argc-2;
  pthread_t thread[argc-2];
  int       rc;
  long      t;
  void*     status;
  pthread_mutex_init(&lock, NULL);  
  //thread_cond_init (&cond, NULL);
  outPutChunk_=(char *)calloc(sizeof(char),chunkSize_+1);
  if(outPutChunk_<0){
	     printf("File could not be created.\n");
        perror("create");
        return -1;
  }
  fdOutPut_  = open(argv[1], O_TRUNC|O_WRONLY|O_CREAT, 0007);//open output file
    if(fdOutPut_ < 0)
    {
        printf("File could not be opened.\n");
        perror("open");
        return -1;
    }
  
  maxNumOfThreads_=argc-2;//number  of threads at the begining
  printf( "Hello, creating %s from %d input files\n",argv[1],maxNumOfThreads_);

  // --- Launch threads ------------------------------
  for( t = 0; t < NUM_THREADS; ++t )
  {
    rc = pthread_create( &thread[t], NULL, ReadCharFromFile, (void*) argv[t+2] );
    if( rc ) 
    {
      printf("ERROR in pthread_create(): "
             "%s\n", strerror(rc));
      exit(-1);
    }
  }

  // --- Wait for threads to finish ------------------
  for( t = 0; t < NUM_THREADS; ++t ) 
  {
    rc = pthread_join(thread[t], &status);
	
    if( rc ) 
    {
      printf( "ERROR in pthread_join(): "
              "%s\n", strerror(rc));
  free(outPutChunk_);

      exit( -1 );
    }
  }  
  close(fdOutPut_);
 
  // --- Epilogue ------------------------------------
  pthread_mutex_destroy(&lock);
  //pthread_cond_destroy(&cond);
  if(maxLen_==-1){
	  maxLen_=0;
  }
  printf( "Created %s with size %d bytes\n",argv[1],maxLen_);
  free(outPutChunk_);
  pthread_exit(NULL);
  exit(0);
}
