#include <fcntl.h>      
#include <unistd.h>     
#include <sys/ioctl.h>  
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <errno.h>

#include "message_slot.h"
int main( int argc, char *argv[] )  
{
	
	int file_d;
	int  ret_v;
	int  id;
	if(argc < 3)
	{
		printf("Invalid number of arguments\n");
		exit(-1);
	}
	if(argc == 3)

		
		file_d = open(argv[1], O_RDONLY);

		
	if (file_d < 0) 
	{
		printf ("Can't open device file\n");
		exit(-1);
	}
	
	
	id = atoi(argv[2]); 
	printf("%d , %s\n",id,argv[1]);

	ret_v = ioctl(file_d,MSG_SLOT_CHANNEL, id);
	
	if (ret_v < 0) 
	{				//printf("iotcl to file failed:%d - %s\n", ret_v, strerror(errno));

		 printf ("ioctl set index failed:%d\n", ret_v);
		 close(file_d); 
		 exit(-1);
	}	 
	
	char buffer[MAX_BUFFER];
	ret_v = read(file_d, buffer, MAX_BUFFER);

	if (ret_v < 0) {
				printf("read to file failed:%d - %s\n", ret_v, strerror(errno));

//printf ("ioctl_set_msg failed:%d\n", ret_v);
		close(file_d); 
		exit(-1);
	}	 
	
	close(file_d); 
	printf("read the message:\n%s\nfrom channel %d\n",buffer, id);
	return 0;
}
