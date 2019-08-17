#include "message_slot.h"
#include <fcntl.h>      
#include <unistd.h>     
#include <sys/ioctl.h>  
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <errno.h>

int main( int argc, char *argv[] )  
{
	int file_d;
	int  ret_v;
	int  id;
	if(argc < 4)
	{
		printf("Invalid number of arguments\n");
		exit(-1);
	}
	if(argc == 4){
		file_d = open(argv[1], O_RDWR);
		//fprintf(stderr, "fopen() failed: %s\n", strerror(errno));
		//printf("%s %s %s %d \n",argv[1],argv[2],argv[3],file_d);
	}
	if (file_d < 0) 
	{
		printf ("Can't open device file\n");
		exit(-1);
	}
	id = atoi(argv[2]); 
		//printf("id :%d \nfile_d : %d\n",id,file_d);
		//printf("IOCTL: %d \n",IOCTL_SET_ENC);
		
	ret_v = ioctl(file_d, MSG_SLOT_CHANNEL, id);
	printf("ret_v : %d\n",ret_v);
	if (ret_v < 0) 
	{
		 printf ("ioctl set index failed: %d\n", ret_v);
		 close(file_d); 
		 exit(-1);
	}	 
	if(strlen(argv[3])>128){
		 close(file_d); 
		 exit(-1);
	}
	
	ret_v = write(file_d, argv[3], strlen(argv[3]));
	//printf("%d, %s\n",strlen(argv[3]),argv[3]);
	if (ret_v < 0) {
		printf("write to file failed:%d - %s\n", ret_v, strerror(errno));
		close(file_d); 
		exit(-1);
	}	 
	
	close(file_d); 
	printf("%d chars from the message:\n %s\nwrote to channel %d\n",ret_v,argv[3],id); 
	return 0;
}