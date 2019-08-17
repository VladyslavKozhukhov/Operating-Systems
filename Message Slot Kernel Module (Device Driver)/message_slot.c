#undef __KERNEL__
#define __KERNEL__
#undef MODULE
#define MODULE
#include  "message_slot.h"
#include <linux/kernel.h>  
#include <linux/module.h>   
#include <linux/fs.h>       
 #include <asm/uaccess.h>   
#include <linux/string.h> 
#include <linux/slab.h> 
MODULE_LICENSE("GPL");




//#############Structs#######################

struct message_slot{
	int channel_id;
	int  minor_id;
	struct buffer_Node * buffer_list; 
	struct message_slot* next;
};

struct buffer_Node{
	int id;//channel_id == id
	int flag;
	int sizeOfbufferMessage;
	char buffer[MAX_BUFFER];
	struct buffer_Node *next;	
};
static struct message_slot *first_message_slot = NULL;
//##########################################

static int deleteAllBuffersInMessagePort(struct message_slot *messageSlot);


/*
static char* getBufferChanelByIdFromMessageSlot(struct message_slot * messageSlot){
	struct buffer_Node * first_run= messageSlot->buffer_list;
	while(first_run!= NULL)
	{
		if(first_run->id ==messageSlot->channel_id){
			return first_run->buffer;
		}
		first_run = first_run->next;
	}
	return NULL;
}
*/
static int createMessageSlot(int minor)
{
	struct message_slot *messageSlot = (struct message_slot*) kmalloc(sizeof(struct message_slot), GFP_KERNEL);

	if(messageSlot == NULL)
	{
		//printk(KERN_EMERG "Failed to kalloc new message slot\n");
		return -1;
	}
	
	messageSlot->minor_id = minor;
	messageSlot->buffer_list = NULL;
	messageSlot->channel_id=-1;	//	if(swapNodes((&(messageSlot->next)))
	messageSlot->next = first_message_slot;
				

	first_message_slot = messageSlot;
	//messageSlot->buffer_list->id =minor;
	//printk(KERN_EMERG "Create Slot Number: %d\n",messageSlot->next->minor_id);

	return SUCCESS;
}
/*
static  int  swapNodes(message_slot**  ms,message_slot** first){
	if(ms ==NULL || first == NULL ){
		return  -1;
	}
	message_slot * tmp = *ms;
	*(ms)=*first;
	*first =tmp;
	return SUCCESS
	
}*/

static int deleteMessageSlot(struct message_slot *messageSlot)
{	
	struct message_slot *tmpMS = NULL;
	if(messageSlot != NULL){
		if(first_message_slot == messageSlot)//MS is the first Node in Linked list
			first_message_slot = messageSlot->next;
		else
		{
			tmpMS = first_message_slot;
			while(tmpMS->next != messageSlot)
			{
				if(tmpMS->next == NULL)
				{
					//printk(KERN_EMERG "Message slot is not found!\n");
					return -EINVAL;
				}
				tmpMS = tmpMS->next;
			}
			tmpMS->next = tmpMS->next->next;
		}
		if(tmpMS!=NULL){messageSlot=tmpMS;}
		if(deleteAllBuffersInMessagePort(messageSlot)>-1){
			//printk(KERN_EMERG "Buffer in MessageSlot is not found!\n");
			kfree(messageSlot);
			return -EINVAL;
		}
		kfree(messageSlot);
		printk(KERN_EMERG "free\n");

		return SUCCESS;
	}
	kfree(messageSlot);
	return -1;

}

static int createBufferForChannel(struct message_slot *messageSlot){
		if(messageSlot != NULL){
			struct buffer_Node *bufferNode = (struct buffer_Node*) kmalloc(sizeof(struct buffer_Node), GFP_KERNEL);
			if(bufferNode == NULL)
			{
				//printk(KERN_EMERG "Failed to kalloc new buffer node\n");
				return -1;
			}
			bufferNode->id = messageSlot->channel_id;
			//printk(KERN_EMERG "buffer_id: % d\n",bufferNode->id);//***
			bufferNode->flag=-1;
			bufferNode->next = messageSlot->buffer_list;
			messageSlot->buffer_list = bufferNode;		
			return SUCCESS;			
		}

	return -1;
}

static int deleteBufferByChannelId(struct message_slot *messageSlot,int channel_id){
	struct buffer_Node *delNode ;
		if(messageSlot != NULL){
			if(messageSlot->buffer_list->id ==channel_id){//first
			messageSlot->buffer_list = messageSlot->buffer_list->next;
			kfree(messageSlot->buffer_list);
			return SUCCESS;
			}
			else{
				struct buffer_Node *tmpNode;
				tmpNode=messageSlot->buffer_list ;
				while(tmpNode->next ->id != messageSlot->channel_id)
				{
					if(tmpNode->next == NULL)
					{
						//printk(KERN_EMERG "Message slot is not found!\n");
						return -1;
					}
				tmpNode = tmpNode->next;
			}
			delNode = NULL;
			delNode=tmpNode->next;
			tmpNode->next = tmpNode->next->next;
			kfree(delNode);
			}
		
		return SUCCESS;
				
		}
	return -1;
}


static int deleteAllBuffersInMessagePort(struct message_slot *messageSlot){

	if(messageSlot != NULL){
		while(messageSlot->buffer_list!=NULL){
			if(deleteBufferByChannelId(messageSlot, messageSlot->buffer_list->id)<0){
			return -EINVAL;
			}
		}
		return SUCCESS;
	}
			return -EINVAL;
	
}

static struct message_slot* getFileMessageSlotByMinorId(int minor_id){
	struct message_slot *messageS = first_message_slot;
	while(messageS != NULL)
	{
		if(messageS->minor_id == minor_id)
			return messageS;
		messageS = messageS->next;
	}
	return NULL;	
}


static struct buffer_Node* getBufferbyId(struct message_slot *messageSlot,int index){
	struct buffer_Node * nodeList = messageSlot->buffer_list;
		//printk(KERN_EMERG "index : %d",index);


		while(nodeList != NULL)
	{	
		if(nodeList->id == index)
			return nodeList;
		nodeList = nodeList->next;
	}
		//	printk(KERN_EMERG "index : %d",index);

	return NULL;	
}




//---- DEVICE _FUNCTIONS----//

//____OPEN___//
static int device_open( struct inode* inode, struct file*  file )
{ 		

	struct message_slot *messageSlot;	
	//printk(KERN_EMERG "Enter to device_open");
	
	messageSlot =  getFileMessageSlotByMinorId(iminor(file->f_path.dentry->d_inode));//Check if MS exist//		

	if(messageSlot ==NULL){
		//printk(KERN_EMERG "here to device_open");
		if(createMessageSlot(iminor(file->f_path.dentry->d_inode))<0){
			return -EINVAL;
		}
		//printk(KERN_EMERG "created new message slot , minnor %d",iminor(file->f_path.dentry->d_inode) );
	}
	//printk(KERN_EMERG "OPENED %d",iminor(file->f_path.dentry->d_inode));
  return SUCCESS;
}

//__***************************RELEASE__********************************//
static int device_release( struct inode* inode, struct file*  file)
{
	//Think about to Lock
	//If yes so here close open the lock
  return SUCCESS;
}

//_*********************************__READ___*********************************************//
static ssize_t device_read( struct file* file, char __user* buffer, size_t  length, loff_t* offset )
{
	int i=-1;
	int j=1;
	int size ;
	struct buffer_Node * bufferRunner ;
	struct buffer_Node* bufferSource ;
	struct message_slot *messageSlot = getFileMessageSlotByMinorId(iminor(file->f_path.dentry->d_inode));
	if (offset == NULL){
		        return -EINVAL;

	}
	if(length > 128){
		
		        return -EINVAL;

	}
	if(messageSlot == NULL){
		printk(KERN_EMERG "can not find fi le1\n");
        return -EINVAL;
	}
			//printk(KERN_EMERG "channel_id  is :   %d\n", messageSlot->channel_id);
			//printk(KERN_EMERG "minor id  is :   %d\n", messageSlot->minor_id);
	bufferSource = getBufferbyId(messageSlot,messageSlot->channel_id);
	if(bufferSource <0){
			//printk(KERN_EMERG "buffer doesnt exist");
        return -EINVAL;

	}
		//printk(KERN_EMERG "buffer_id  is :   %d\n", bufferSource->id);

	//printk(KERN_EMERG "Message slot  channel id %d, minor :  %d", messageSlot->channel_id,messageSlot->minor_id);
	
			
	if(messageSlot->channel_id == -1){
		return -EINVAL;
	}
	

		bufferRunner =bufferSource;
		//bufferSource =messageSlot->buffer_list;
		//printk(KERN_EMERG "buffer :  %s", bufferRunner->buffer);

	while(bufferRunner!= NULL  && j==1){
		if(bufferRunner ->id == messageSlot->channel_id){
			if(bufferSource->flag==-1){
				return -EWOULDBLOCK;
			}
					//printk(KERN_EMERG "channel_id :  %d", bufferRunner->id);
			j=2;	
			bufferSource = bufferRunner;
		}
		bufferRunner=bufferRunner->next;
	}
	
	if(j != 2){//check if this channel exits
		//printk(KERN_EMERG "can not find file2 \n");
		return -EINVAL;
	}
		
	size =bufferSource->sizeOfbufferMessage;
		//printk(KERN_EMERG "Size of bits : %d \n",size);

	for(i = 0; i < length && i < MAX_BUFFER && i<size; i++){
		
			//printk(KERN_EMERG "READ FILE %s",bufferSource->buffer);

		if(put_user((bufferSource->buffer)[i], &buffer[i]) == -EFAULT)
		{
			
			return -ENOSPC;		
		}
		//printk(KERN_EMERG "enter char :  %c\n",(bufferSource->buffer)[i]);

	}
	if(put_user((bufferSource->buffer)[size],&buffer[size])== -EFAULT)
	return -ENOSPC;	

	
  return i;
}



//__***********************_WRITE__***************************************_//

static ssize_t device_write( struct file*  file, const char __user* buffer,  size_t  length, loff_t  *  offset)
{
	int byteNum;
	struct buffer_Node * bufferRunner;
	struct buffer_Node * bufferSource;
	struct message_slot *messageSlot;
	int j=1;
	// printk(KERN_EMERG "%s , %d",buffer , length);
		if(length>MAX_BUFFER){
			return  -EINVAL;
	}
	if(offset == NULL){
			return  -EINVAL;

	}
	messageSlot = getFileMessageSlotByMinorId(iminor(file->f_path.dentry->d_inode));
 
	 //printk(KERN_EMERG "write______messageSlot___Id: %d",messageSlot-> minor_id);
	
	if(messageSlot == NULL){
		printk(KERN_EMERG "can not find file in write \n");
        return -EINVAL;
	}
	
	if(messageSlot->channel_id == -1){
		return  -EINVAL;
	}	

	bufferRunner = messageSlot->buffer_list;
	bufferSource = messageSlot->buffer_list;

	while(bufferRunner != NULL  && j==1){
		if(bufferRunner->id == messageSlot->channel_id){
			j=2;		
			bufferSource=bufferRunner;
		}
		if(bufferRunner->next == NULL){
			j=5;
		}
		else{
			bufferRunner=bufferRunner->next;		
		}
	}	
	if(j !=2 ){//check if this channel exits
		if(createBufferForChannel(messageSlot)<0){return -EINVAL;}
		//printk(KERN_EMERG "Create new Channel \n");
	}		

	bufferSource = getBufferbyId(messageSlot, messageSlot->channel_id);
	bufferSource->flag=1;
	for (byteNum = 0; byteNum < length && byteNum < MAX_BUFFER; byteNum++)
	{
		if(get_user((bufferSource->buffer)[byteNum], &buffer[byteNum]) == -EFAULT)
		{
			printk(KERN_EMERG "failed to read from user address %p\n",buffer+byteNum);
        return -EINVAL;
		}
		//printk(KERN_EMERG "enter char :  %c\n",(bufferSource->buffer)[byteNum]);

	}
	(bufferSource->buffer)[length]='\0';
	bufferSource->sizeOfbufferMessage=length;
	
			//printk(KERN_EMERG "enter strings :  %s, id = %d  \n",(bufferSource->buffer),bufferSource->id);	
	return byteNum;
}

static int checkIfChannelExist(struct message_slot *messageSlot, int index){
	struct buffer_Node * bufferRunner;

	if(messageSlot->buffer_list != NULL){
		bufferRunner =messageSlot->buffer_list;
		while(bufferRunner->buffer!=NULL){
				//printk(KERN_EMERG "Im here %d\n",bufferRunner->id );
			if(bufferRunner->id == index){
				return 1;
			}
			bufferRunner=bufferRunner->next;
			if(bufferRunner == NULL){
			//printk(KERN_EMERG "Doesnt Exit Channel\n");
        return -EINVAL;
			}
		}
	}
        return -EINVAL;
}

//--------------------------------------------------------IOTCL-------------------------------
static long device_ioctl( struct   file* file, unsigned int   ioctl_command_id,unsigned long  ioctl_param )
{
	struct message_slot *messageSlot;
	int index ;
	index=iminor(file->f_path.dentry->d_inode);
	messageSlot  = getFileMessageSlotByMinorId(iminor(file->f_path.dentry->d_inode));
	if(messageSlot == NULL)
	{
		printk(KERN_EMERG "couldn't find file\n");
        return -EINVAL;
	}
			//printk(KERN_EMERG "IOCTL_SET_ENC : %d\n",IOCTL_SET_ENC);
			//	printk(KERN_EMERG "ioctl_command_id : %d\n",ioctl_command_id);
				//printk(KERN_EMERG "ioctl_param : %ld\n",ioctl_param);

	
	if(MSG_SLOT_CHANNEL == ioctl_command_id) 
	{
		if(ioctl_param<0)
		{
			printk(KERN_EMERG "invalid index %ld\n",ioctl_param);
			return -EINVAL;
		}
						//printk(KERN_EMERG "ioctl_command_id1 : %d\n",ioctl_command_id);

		messageSlot->channel_id = ioctl_param;
						//printk(KERN_EMERG "ioctl_command_id2 : %d\n",ioctl_command_id);

		if(checkIfChannelExist(messageSlot,ioctl_param)<0){
			if(createBufferForChannel(messageSlot)<0)
				return -EINVAL;
		}
	}

  return SUCCESS;
}

/************** Module Declarations *****************/

struct file_operations Fops = {
    .read = device_read,
    .write = device_write,
    .unlocked_ioctl= device_ioctl,
    .open = device_open,
    .release = device_release,  
};

static int __init simple_init(void)
{
	unsigned int rc = -1;	
		printk(KERN_EMERG "Import Module");

    rc = register_chrdev(MAJOR_NUM, DEVICE_RANGE_NAME, &Fops );
	printk(KERN_EMERG "number chrdev %d\n",rc);
	
    if (rc < 0) {
         printk( KERN_ALERT "%s registraion failed for  %d\n",DEVICE_FILE_NAME, MAJOR_NUM );
		return rc;
	
    }
	printk(KERN_EMERG "message_slot: registered major number %d\n", MAJOR_NUM);
 
    return SUCCESS;
}


static void __exit simple_cleanup(void)
{
	while(first_message_slot != NULL) 
		deleteMessageSlot(first_message_slot); 	
    unregister_chrdev(MAJOR_NUM, DEVICE_RANGE_NAME);
}


module_init(simple_init);
module_exit(simple_cleanup);