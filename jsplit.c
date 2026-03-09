/*
=============================


PROGRAMMED BY JAN TESKEREDZIC

A custom implementation of the split POSIX utility

Programmed for educational purposes.


=============================
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>


//MACRO DEFINITIONS
#define BUFFER_SIZE 1024
#define DEFAULT_WRITE_AMOUNT 200


//GLOBAL VARIABLES
long size = 0;
long limitWriteSize = 0;
int writeAmount = 0;
int limitFileAmount = 0;

//ARGUMENT FLAGS
int splitByBytes = 1;
int splitByKb = 0;
int endWithNl = 0;
int outputFileName = 0;
int yesFlag = 0;
int limitWrite = 0;
int limitFile = 0;


//NAMING CONVENTION GLOBALS
char* 		batchingName = "autobatch";
char* 		inputFile = NULL;
size_t  	_batchingNameLength = -1;


int main(int argc, char* argv[])
{
	if(argc < 2)//If argumens are less than 2, that means there are no arguments. Program call is counted as arg
	{
		printf("jsplit: use as jsplit -options <input_file> -o <output_batching_name>\n");
		return 1;
	}
	

	{//CREATING SCOPE FOR ARGUMENT CHECKING

	int _nextIsBatchingName = 0;//Temporary for loop conditional
	int _nextIsSizeInput = 0;
	int _nextIsLimit = 0;
	int _nextIsFileLimit = 0;
	for(int i = 0; i < argc; i++)
	{
		if(i == 0) continue; //Skip first argument because its program call
		if(_nextIsBatchingName)
		{
			if(*(argv[i]) == '-')
			{
				printf("jsplit: <err> output batching name should be typed without \'-\' prefix\n");
				return 2;
			}
			if((_batchingNameLength = strlen(argv[i])) > 20)
			{
				printf("jsplit: <err> output batching name is longer than 20 characters\n");
				return 2;
			}
			batchingName = argv[i];
			_nextIsBatchingName = 0;
			continue;
		}
		else if(inputFile == NULL)
		{
			if(*(argv[i]) != '-')
			{
				inputFile = argv[i];
				continue;
			}
		}
		if(_nextIsSizeInput)
		{
			if(writeAmount != 0)
			{
				printf("jsplit: can not define split size more than once\n");
				return 2;
			}
			#if 0
			if(splitByKb && splitByBytes) // LEGACY FLAG COMPARISON, NOT NEEDED
			{
				printf("jsplit: can not use both -b and -kb flag\n");
				return 2;
			}
			#endif

			if(*(argv[i]) == '-')
			{
				printf("jsplit: write size should be tpyed without \'-\' prefix\n");
				return 2; 
			}
			_nextIsSizeInput = 0;
			char* tt = argv[i];
			while(*(tt) != '\0')
			{
				if(*(tt) >= '0' && *(tt) <= '9')
				{
					++tt;
					continue;
				}
				printf("jsplit: numeric values must be used for split size\n");
			}
			int t = atoi(argv[i]);
			writeAmount = t;
			if(splitByKb) writeAmount *= 1024;
			if(writeAmount < 64)
			{
				printf("jsplit: write size should be at least 64 bytes\n");
				return 2;
			}
			
		}
		if(_nextIsLimit)
		{
			if(*(argv[i]) == '-')
			{
				printf("jsplit: limit size should be typed without \'-\' prefix\n");
				return 2; 
			}
			_nextIsLimit = 0;
			char* tt = argv[i];
			while(*(tt) != '\0')
			{
				if(*(tt) >= '0' && *(tt) <= '9')
				{
					++tt;
					continue;
				}
				printf("jsplit: numeric values must be used for limit size\n");
			}
			int t = atoi(argv[i]);
			if(t < 16)
			{
				printf("jsplit: limit size should be at least 64 bytes\n");
				return 2;
			}
			limitWriteSize = t;
		}
		if(_nextIsFileLimit)
		{
			if(*(argv[i]) == '-')
			{
				printf("jsplit: file limit amount should be typed without \'-\' prefix\n");
				return 2; 
			}
			_nextIsFileLimit = 0;
			char* tt = argv[i];
			while(*(tt) != '\0')
			{
				if(*(tt) >= '0' && *(tt) <= '9')
				{
					++tt;
					continue;
				}
				printf("jsplit: numeric values must be used for file limit amount\n");
			}
			int t = atoi(argv[i]);
			if(t < 1)
			{
				printf("jsplit: file limit must be at least: 1\n");
				return 2; 
			}
			limitFileAmount = t;
		}
		if(strcmp(argv[i], "-b") == 0)
		{
			_nextIsSizeInput = 1;
			continue;
		}
		if(strcmp(argv[i], "-kb") == 0)
		{
			splitByKb = 1;
			_nextIsSizeInput = 1;
			continue;
		}
		if(strcmp(argv[i], "-n") == 0)
		{
			endWithNl = 1;
			continue;
		}
		if(strcmp(argv[i], "-o") == 0)
		{
			if(argc < 4)
			{
				printf("jsplit: no output batching name provided\n");
				return 2;
			}
			outputFileName = 1;
			_nextIsBatchingName = 1;
			continue;
		}
		if(strcmp(argv[i], "-l") == 0)
		{
			_nextIsLimit = 1;
			limitWrite = 1;
			continue;
		}
		if(strcmp(argv[i], "-lf") == 0)
		{
			_nextIsFileLimit = 1;
			limitFile = 1;
			continue;
		}
		if((strcmp(argv[i], "-y") == 0) || (strcmp(argv[i], "-Y") == 0))
		{
			yesFlag = 1;
			continue;
		}
	}

	}// CLOSING SCOPE FOR ARGUMENT CHECKING

	if(inputFile == NULL)
	{
		printf("jsplit: no input file provided\n");
		return 1;
	}

	if(outputFileName && batchingName == NULL)
	{
		printf("jsplit: No output batching name provided\n");
		return 2;
	}

	int fileptr = open(inputFile, O_RDONLY);
	if(fileptr == -1)
	{
		printf("jsplit: Could not find file: \'%s\'\n", inputFile);
		return 2;
	}

	if(!writeAmount) writeAmount = DEFAULT_WRITE_AMOUNT;

	if(!yesFlag)
	{
		char inputC;
		printf("Are you sure that you want to split '%s' by %d bytes? y/n:", inputFile, writeAmount);
		inputC = getchar();
		if(inputC != 'y' && inputC != 'Y')
		{
			printf("jsplit: program stopped by user\n");
			return 5;
		}
	}


	off_t 	fileSize = lseek(fileptr, 0, SEEK_END);
	lseek(fileptr, 0, SEEK_SET);

	if(fileSize <= writeAmount)
	{
		printf("jsplit: file size[%i bytes] is smaller than splitting unit[%i bytes]\n", fileSize, writeAmount);
		return 3;
	}

	if(limitWrite)
	{
		if(writeAmount > limitWriteSize)
		{
			printf("jsplit: splitting unit[%i bytes] is larger than write limit[%i bytes]\n",writeAmount, limitWriteSize);
			return 3;
		}
	}

	char 	charBuffer[BUFFER_SIZE];
	long 	amountWritten = 0;
	long 	__TOTAL_WRITTEN = 0;
	int	local_write = 0;
	off_t 	dataLoaded = 0;

	char str[25] = {0};
	
	if(!outputFileName) _batchingNameLength = 9;
	strcpy(str, batchingName);
	str[_batchingNameLength] = '1';

	int 	filesCreated = 1;
	int 	fileWrittenTo = open(str, O_CREAT | O_WRONLY, 0644);


	if(fileWrittenTo == -1)
	{
		printf("jsplit: <err> could not create batch file: make sure naming convention is correct\n");
		return 4;
	}

	while((dataLoaded = read(fileptr, charBuffer, BUFFER_SIZE)) > 0)
	{
		int __index = 0;
		local_write = writeAmount;
		while(dataLoaded > 0)
		{
			if(dataLoaded < writeAmount) local_write = dataLoaded;
			if(limitWrite)
			{
				if((__TOTAL_WRITTEN + local_write) > limitWriteSize)
				{
					goto FINISH;
				}
			}

			write(fileWrittenTo, charBuffer + __index, local_write);
			
			amountWritten += local_write;
			__TOTAL_WRITTEN += local_write;
			dataLoaded -= local_write;

			if(amountWritten >= writeAmount && __TOTAL_WRITTEN < fileSize)
			{
				if(endWithNl)
				{
					write(fileWrittenTo, "\n", 1);
				}
				close(fileWrittenTo);
				if(limitFile)
				{
					if((filesCreated + 1) > limitFileAmount)
					{
						goto FINISH;
					}
				}
				__index = amountWritten;
				amountWritten = 0;

				#if 0

				char *ptr = str; //UNUSED LEGACY POSTFIX INCREMENT
				(*(ptr + _batchingNameLength))++;

				#endif

				++filesCreated;
				sprintf(str + _batchingNameLength, "%d", filesCreated);
				fileWrittenTo = open(str, O_CREAT | O_WRONLY, 0644);
				if(fileWrittenTo == -1)
				{
					--filesCreated;
					printf("jsplit: <err> could not create batch file: make sure naming convention is correct\n");
					printf("jsplit: Created %d files[total bytes written %d]\n", filesCreated, __TOTAL_WRITTEN);
					return 4;
				}
			}
		}
	}

	FINISH:
	printf("jsplit: Created %d files[total bytes written %d]\n", filesCreated, __TOTAL_WRITTEN);
	close(fileptr);
	close(fileWrittenTo);
	return 0;
}
