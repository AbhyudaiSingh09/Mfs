
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#define WHITESPACE " \t\n"	// Whitespace is used as the delimiter//
#define MAXCOMMAND 255	// The maximum size of the command-line
#define MAXCHARARGU 10	// Mav shell will support only  five arguments

FILE * filepointer;
struct __attribute__((__packed__)) DirectoryEntry	//attribute struct
{
	char DIR_Name[11];	//Retriving directory name
	uint8_t DIR_Attr;	//Retriving directory attribute count 
	uint8_t Unused1[8];
	uint16_t DIR_FirstClusterHigh;
	uint8_t Unused2[4];
	uint16_t DIR_FirstClusterLow;
	uint32_t DIR_FileSize;	//Directory Size 
};
struct DirectoryEntry dir[16];	//Directed st

// To tokenize input by the user using cmd and token
char *token[MAXCHARARGU];	//input string separated by white space
char cmd[MAXCOMMAND];	// string inputed  will be parsed into  tokens 

char BS_OEMName[8];	//saved file 
int16_t BPB_BytesPerSec;	//Number of byter per sector 
int8_t BPB_SecPerClus;	//Number of sectors per cluster 
int16_t BPB_RsvdSecCnt;	//Reserved sectors count 
int8_t BPB_NumFATs;
int16_t BPB_RootEntCnt;	//Count of root entry 
int32_t BPB_FATSz32;
int32_t BPB_RootClus;	//Location of Rootcluster  in fat32 image
int32_t RootDirSectors = 0;	//Number of seperators in root directiory
int32_t FirstDataSector = 0;	//Location of first datasector
int32_t FirstSectorofCluster = 0;	//Location of th first sector of data cluster
int32_t currentDirectory;	//CWD
char formattedDirectory[12];	//Fullly foramted String is contained here
int file_directory_exist(struct DirectoryEntry dir[], char token[]);

int LBAToOffset(int32_t sector)
{
	if (sector == 0)
		sector = 2;
	return ((sector - 2) *BPB_BytesPerSec) + (BPB_BytesPerSec *BPB_RsvdSecCnt) + (BPB_NumFATs *BPB_FATSz32 *BPB_BytesPerSec);
}

int16_t NextLB(uint32_t sector)
{
	uint32_t FATAddress = (BPB_BytesPerSec *BPB_RsvdSecCnt) + (sector *4);
	int16_t val;
	fseek(filepointer, FATAddress, SEEK_SET);
	fread(&val, 2, 1, filepointer);
	return val;
}

void executingfunction();
void getInput();
void functionOpImage(char fileopen[]);
void functionCleanDirectory(char *directoryname);
void functionClImage();	//file closes function
void functionprintdirectory_ls();	//To print out the working directory (ls)
void functiondirectorychange_cd(int32_t sector);	//To changes directory as per user (cd)
void functionDirInfo();	//To print out the directory info from the struct above 
int32_t getCluster(char *directoryname);	//Gather cluster information to be used in executingfunction
int32_t getSizeOfCluster(int32_t cluster);	//Gather the attribute size of the cluster 
void get(char *directoryname);	//TO gte the file from the file system 
void functiondecimalToHexadecimal(int decimal);	// decimal numbers to hex conversion 
void functionprintstat(char *directoryname);	//Prints directory attributes 
void functionread_userFile(char *directoryname, int position, int numberofBytes);	//user specified bytes to be read 

int main()
{
	int CHECK = 1;
	while (CHECK)
	{
		getInput();
		executingfunction();
	}

	return 0;
}

void getInput()
{
	printf("mfs>");

	memset(cmd, '\0', MAXCOMMAND);
	while (!fgets(cmd, MAXCOMMAND, stdin))
	;
	char *argument;
	int count = 0;
	char *ptr_tocheck = strdup(cmd);
	char *working_root = ptr_tocheck;
	memset(&token, '\0', MAXCHARARGU);
	memset(&token, '\0', sizeof(MAXCHARARGU));
	while (((argument = strsep(&ptr_tocheck, WHITESPACE)) != NULL) && (count < MAXCHARARGU))
	{
		token[count] = strndup(argument, MAXCOMMAND);
		if (strlen(token[count]) == 0)
		{
			token[count] = NULL;
		}

		count++;
	}

	free(working_root);
}

void functionOpImage(char fileopen[])
{
	filepointer = fopen(fileopen, "r");
	if (filepointer == NULL)
	{
		printf("No Image found\n");	//Invalid image name provided 
		return;
	}

	printf("%s opened.\n", fileopen);

	fseek(filepointer, 3, SEEK_SET);
	fread(&BS_OEMName, 8, 1, filepointer);

	fseek(filepointer, 11, SEEK_SET);
	fread(&BPB_BytesPerSec, 2, 1, filepointer);
	fread(&BPB_SecPerClus, 1, 1, filepointer);
	fread(&BPB_RsvdSecCnt, 2, 1, filepointer);
	fread(&BPB_NumFATs, 1, 1, filepointer);
	fread(&BPB_RootEntCnt, 2, 1, filepointer);

	fseek(filepointer, 36, SEEK_SET);
	fread(&BPB_FATSz32, 4, 1, filepointer);

	fseek(filepointer, 44, SEEK_SET);
	fread(&BPB_RootClus, 4, 1, filepointer);
	currentDirectory = BPB_RootClus;

	int offset = LBAToOffset(currentDirectory);
	fseek(filepointer, offset, SEEK_SET);
	fread(&dir[0], 32, 16, filepointer);

}

int32_t getCluster(char *directoryname) //Obtain cluster data for use in the execution function
{
	int32_t cluster = 0; //Initialize cluster to 0
	int i; //Initialize i to 0
	for (i = 0; i < 16; i++) //for each entry in directory
	{
		if (strcmp(dir[i].DIR_Name, directoryname) == 0) //if the name of the directory matches with name of the entry
		{
			cluster = dir[i].DIR_FirstClusterLow; //set cluster to the first cluster of the entry
		}
	}

	return cluster;
}

int32_t getSizeOfCluster(int32_t cluster) //Obtain the size of the cluster
{
	int32_t size = 0; //Intialize size to 0
	size = BPB_SecPerClus * BPB_BytesPerSec; //get the size of the cluster
	return size;
}

void functionCleanDirectory(char *directoryname) //clean the directory
{
	char exp_name[12];
	//const char s[2]=".";

	memset(exp_name, ' ', 12);
	char *token = strtok(directoryname, "."); //tokenize the directory name

	if (token)
	{
		strncpy(exp_name, token, strlen(token));	//copy token to exp_name 
		token = strtok(NULL, ".");
		if (token) 
		{
			strncpy((char*)(exp_name), token, strlen(token)); //copy token to exp_name
		}

		exp_name[11] = '\0'; //set the last character to null

		int i = 0; //initialize i to 0
		for (i = 0; i < 11; i++) 
		{
			exp_name[i] = toupper(exp_name[i]);	//to uppercase the directory 
		}
	}
	else
	{
		strncpy(exp_name, directoryname, strlen(directoryname)); //copy directoryname to exp_name
		exp_name[15] = '\0'; //set the last character to null

	}

	strncpy(formattedDirectory, exp_name, 12); //copy exp_name to formattedDirectory
}

void executingfunction()
{
	if (token[0] == NULL)	//if no value has enterned return without opening file
	{
		return;
	}

	if (strcmp(token[0], "open") == 0)	//if the token has open command in it to open the file proceed 
	{
		if (filepointer != NULL)
		{
			printf("Error system image open already\n"); //if the file is already open return error
			return;
		}

		if (token[1] != NULL && filepointer == NULL)  
		{
			functionOpImage(token[1]);	// Open Image after comparing token and file pointer 
		}
		else if (token[1] = NULL)
		{
			printf("Error:Please give arguments of file to open");
		}

		return;
	}
	else if (strcmp(token[0], "info") == 0) //if the token has info in it
	{
		printf("BPB_RsvdSecCnt:%d=\n", BPB_RsvdSecCnt);
		functiondecimalToHexadecimal(BPB_RsvdSecCnt); //convert decimal to hexadecimal
		printf("BPB_FATSz32:%d=\n", BPB_FATSz32);
		functiondecimalToHexadecimal(BPB_FATSz32); //convert decimal to hexadecimal
		printf("BPB_BytesPerSec:%d=\n", BPB_BytesPerSec);
		functiondecimalToHexadecimal(BPB_BytesPerSec); //convert decimal to hexadecimal
		printf("BPB_SecPerClus:%d=\n", BPB_SecPerClus);
		functiondecimalToHexadecimal(BPB_SecPerClus); //convert decimal to hexadecimal
		printf("BPB_NumFATs:%d=\n", BPB_NumFATs);
		functiondecimalToHexadecimal(BPB_NumFATs); //convert decimal to hexadecimal
	}
	else if (filepointer == NULL)
	{
		printf("ERROR: Please open an Image first\n");
	}
	else if (strcmp(token[0], "ls") == 0) //if the token has ls in it
	{
		functionprintdirectory_ls();	//print the directory 
	}
	else if (strcmp(token[0], "cd") == 0) //if the token has cd in it
	{
		if (token[1] == NULL)
		{
			printf("ERROR: Please provide directory name\n");
			return;
		}

		functiondirectorychange_cd(getCluster(token[1])); //change the directory
	}
	else if (strcmp(token[0], "get") == 0) //if the token has get in it
	{
		get(token[1]);
	}
	else if (strcmp(token[0], "read") == 0) //if the token has read in it
	{
		if (token[1] == NULL || token[2] == NULL || token[3] == NULL) //if the token is empty
		{
			printf("Input argument");
			return;
		}

		functionread_userFile(token[1], atoi(token[2]), atoi(token[3])); //read the file
	}
	else if (strcmp(token[0], "stat") == 0) //if the token has stat in it.
	{
		functionprintstat(token[1]); //print the stat
	}
	else if (strcmp(token[0], "close") == 0) //if the token has close in it.
	{
		functionClImage(); //close the image
	}

	/*else if (strcmp("delete",token[0])==0) 
	    {
	        if(token[1]==NULL)
	            {
	                printf("Error:Provide file name\n ");
	                return;
	            }

	        char *directoryname;
	        int index= 0;
	        index= file_directory_exist(dir,token[1]);
	        if(index=-1)
	            {
	               printf("FIle does not exist \n");
	            }    
	         else 
	            {
	                strncpy(BS_OEMName,dir[index].DIR_Name,11);
	                dir[index].DIR_Name[0]=0xe5;
	            }    
	    }*/
	else if (strcmp("undel", token[0]) == 0) //if the token has undel in it.
	{
		if (token[1] == NULL)
		{
			printf("Give filename for undelete");
		}
		else
		{
			int a = 0;
			for (a = 0; a < 16; a++)
			{
				if (dir[a].DIR_Name[0] == 'F')
				{
					strncpy(dir[a].DIR_Name, BS_OEMName, strlen(BS_OEMName)); //copy BS_OEMName to dir[a].DIR_Name
					dir[a].DIR_Attr = 0x1; //set the attribute to 1
				}
			}
		}
	}
}

void get(char *directoryname) //get the file
{
	char *dir_string = (char*) malloc(strlen(directoryname));
	strncpy(dir_string, directoryname, strlen(directoryname)); //copy directoryname to dir_string
	int cluster = getCluster(dir_string); //get the cluster number
	int size = getSizeOfCluster(cluster); //get the size of the cluster
	FILE *new_filepointer = fopen(token[1], "w"); //open the file
	fseek(filepointer, LBAToOffset(cluster), SEEK_SET); //seek the filepointer to the cluster
	char *pointer; //pointer to the file
	pointer = (char*) malloc(size); //allocate memory to pointer
	fread(pointer, size, 1, filepointer); //read the file
	fwrite(pointer, size, 1, new_filepointer); //write the file
	fclose(new_filepointer); //close the file

}

void functionprintstat(char *directoryname) //print stat
{
	int clster = getCluster(directoryname); //get the cluster number
	int size = getSizeOfCluster(clster); //get the size of the cluster
	printf("size=%d\n", size); 
	int i; 
	for (i = 0; i < 16; i++) 
	{
		if (clster == dir[i].DIR_FirstClusterLow)
		{
			printf("Attr=%d\n", dir[i].DIR_Attr); //print the attribute
			printf("cluster start= %d\n", clster); //print the cluster start
			printf("cluster High=%d\n", dir[i].DIR_FirstClusterHigh); //print the cluster High
		}
	}
}

void functiondirectorychange_cd(int32_t cluster) //change the directory
{
	if (cluster == 0)
	{
		printf("Error:Invalid Directory \n"); 
		return;
	}
	else
	{
		currentDirectory = cluster; //set the current directory to cluster
		int offset = LBAToOffset(currentDirectory); //get the offset
		fseek(filepointer, offset, SEEK_SET); //seek the filepointer to the offset
		fread(&dir[0], 32, 16, filepointer); //read the file
	}
}

void functionDirInfo()
{
	int i;
	for (i = 0; i < 16; i++)
	{
		fread(&dir[i], 32, 1, filepointer); //read the file
	}
}

void functionprintdirectory_ls()	//toprint the directory 
{
	if (filepointer == NULL)
	{
		printf("No image is opened\n");	//to print directory open image first 
		return;
	}

	int offset = LBAToOffset(currentDirectory); //get the offset
	fseek(filepointer, offset, SEEK_SET); //seek the filepointer to the offset
 
	int i;
	for (i = 0; i < 16; i++)
	{
		fread(&dir[i], 32, 1, filepointer); //read the file

		if ((dir[i].DIR_Name[0] != (char) 0xe5) &&
			(dir[i].DIR_Attr == 0x1 || dir[i].DIR_Attr == 0x10 || dir[i].DIR_Attr == 0x20)) //if the file is not deleted
		//check for the above state before oopening files. 
		{
			char *directory;
			directory = (char*) malloc(16);
			memset(directory, '\0', 11);
			memcpy(directory, dir[i].DIR_Name, 11);
			printf("%s\n", directory);
			/*char filename=(char)malloc(sizeof(dir[i].DIR_Name));
			strncpy(filename,dir[i].DIR_Name,11);
			printf("%s \n",filename); */
		}
	}
}

void functionClImage()
{
	if (filepointer == NULL)
	{
		printf("Error= No Open file found ");	//print error of no file is open at the moment 
		return;
	}
	else
	{
		fclose(filepointer);
		filepointer = NULL;
	}
}

void functiondecimalToHexadecimal(int decimal)	// function will be used to convert decimal to hexa decimal values  
{
	int d = 1, h, temperoary;
	char Hexadecimal[100];
	while (decimal != 0)
	{
		temperoary = decimal % 16; //get the remainder
		if (temperoary < 10) //if the remainder is less than 10
		{ 
			temperoary += 48;  //convert the remainder to ascii value
		}
		else
		{
			temperoary += 55; //if the remainder is greater than 10
		}

		Hexadecimal[d++] = temperoary; //store the remainder in the array
		decimal /= 16; //divide the decimal by 16
	}

	for (h = d - 1; h > 0; h--)
	{
		printf("%c", Hexadecimal[h]); //print the hexadecimal values
	}
}

void functionread_userFile(char *directoryname, int position, int numberofBytes)	// read the userfile 
{
	int c = getCluster(directoryname); //get the cluster number
	directoryname = (char*) malloc(16); //allocate memory to directoryname
	int Offset = LBAToOffset(c); //get the offset
	fseek(filepointer, Offset + position, SEEK_SET); //seek the filepointer to the offset
	char *bytes; //pointer to the file
	bytes = (char*) malloc(numberofBytes); //allocate memory to bytes
	fread(bytes, numberofBytes, 1, filepointer); //read the file
	printf("%s\n", bytes); //print the file
}