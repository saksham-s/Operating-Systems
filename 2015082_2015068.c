#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define BLOCK 1024 // 1KB block size
#define INODE_ENTRIES 10 // only 100 entries allowed
#define DATA_ENTRIES 1023 // max allowable data entries
#define SUPER_BLOCK 2*1024 // 2KB super block
#define INODE_BITMAP 1*1024 // 1KB inode bitmap block
#define DATA_BITMAP 1*1024 // 1KB data bitmap block

typedef struct _superBlock {
	char *fileName;
	int fileSizeKB;

} superBlock;

typedef struct _inodeBlock {
	char *fileName;
	int timesFetched;
	int start[DATA_ENTRIES];
	int end[DATA_ENTRIES];
} inodeBlock;

typedef struct _inodeBitmap {
	char iBitmap[INODE_ENTRIES];
} inodeBitmap;

typedef struct _dataBitmap{
	char dBitmap[DATA_ENTRIES];
} dataBitmap;

int noOfFiles = 0;

superBlock *super;
inodeBlock *inode[INODE_ENTRIES];
inodeBitmap *inodeBit;
dataBitmap *dataBit;

void showMenu() {
	printf("\n1. Make a new file\n2. Add entry to already created file\n3. Read an already created file\n4. Print Inode Bitmaps\n5. Print Data Bitmaps\n6. Print File List\n7. Exit\n");
}

int strLength(char *string) {
	int counter = 0, moveBack;
	char *myString = string;
	while(*myString != '\0') {
		counter += 1;
		*myString += 1;
	}
	moveBack = counter-1;
	while( moveBack >= 0 ) {
		*string -= 1;
		moveBack -= 1;
	}
	return counter;
}

int createSFS(char* fileName, int nBytes) {
	// create File System
	int writeError, fileSize = nBytes*BLOCK, seekError, i = 0;
	if( sizeof(char*)*strLength(fileName)+1 > fileSize ) {
		printf("\nError -> File System was not initialised. File Name size is more than size provided\n");
		goto done;
	}
	int fileSystemID = open(fileName, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	char empty = ' ';
	if( fileSystemID < 0 ) {
		printf("\nError -> File System was not initialised. File not formed\n");
	}
	else {
		for( ; i < fileSize; i++ ) {
			writeError = write(fileSystemID, (void*)(&empty), 1	);
			if( writeError == -1 ) {
				printf("\nError -> File System was not initialised. File was not written correctly\n");
				break;
			}
		}
		// create Super Block
		super = (superBlock*)malloc(sizeof(superBlock));
		super->fileName = (char*)malloc(sizeof(char)*(strLength(fileName)+1));
		strcpy(super->fileName,fileName);
		super->fileSizeKB = fileSize;
		for( i = 0; i < 2; i++ ) {
			if( i == 0 ) {
				seekError = lseek(fileSystemID, 0, 0);
				if( seekError == -1 ) {
					printf("\nError -> File System was not initialised. File seeking caused the error\n");
					break;
				}
				char *myFile = (char*)malloc(sizeof(char)*(strLength(super->fileName)+1));
				strcpy(myFile, super->fileName);
				writeError = write(fileSystemID, (void*)(myFile), BLOCK);
			}
			else {
				seekError = lseek(fileSystemID, 0, 1);
				if( seekError == -1 ) {
					printf("\nError -> File System was not initialised. File seeking caused the error\n");
					break;
				}
				char size[4]; 
				sprintf(size, "%d", super->fileSizeKB);
				writeError = write(fileSystemID, (void*)(size), BLOCK);
			}
			if( writeError == -1 ) {
				printf("\nError -> File System was not initialised. File was not written correctly\n");
				break;
			}
		}
	}
	// initialize inode and data bitmaps
	inodeBit = (inodeBitmap*)malloc(sizeof(inodeBitmap));
	dataBit = (dataBitmap*)malloc(sizeof(dataBitmap));
	for( i = 0; i < INODE_ENTRIES; i++ ) {
		inodeBit->iBitmap[i] = '0';
	}
	for( i = 0; i < DATA_ENTRIES; i++ ) {
		dataBit->dBitmap[i] = '0';
	}
	seekError = lseek(fileSystemID, 0, 1);
	if( seekError == -1 ) {
		printf("\nError -> File System was not initialised. File seeking caused the error\n");
	}
	else {
		writeError = write(fileSystemID, (void*)(inodeBit->iBitmap), BLOCK);
		if( writeError == -1 ) {
			printf("\nError -> File System was not initialised. File was not written correctly\n");
		}
	}
	seekError = lseek(fileSystemID, 0, 1);
	if( seekError == -1 ) {
		printf("\nError -> File System was not initialised. File seeking caused the error\n");
	}
	else {
		writeError = write(fileSystemID, (void*)(dataBit->dBitmap), BLOCK);
		if( writeError == -1 ) {
			printf("\nError -> File System was not initialised. File was not written correctly\n");
		}
	}
	return fileSystemID;
	done:
		close(fileSystemID);
		return 0;	
}

int readData(int disk, int blockNum, void* block) {
	// read data block by block
	int readError;
	int seekError = lseek(disk, blockNum*BLOCK, 0);
	if( seekError == -1 ) {
		printf("\nError -> File not read. File seeking caused the error\n");
	}
	else {
		readError = read(disk, block, BLOCK);
		if( readError == -1 ) {
			printf("\nError -> File not read. File reading caused the error\n");
		}
	}
	return readError;
}

int writeData(int disk, int blockNum, void* block) {
	// write data block by block
	int writeError;
	int seekError = lseek(disk, blockNum*BLOCK, 0);
	if( seekError == -1 ) {
		printf("\nError -> File not read. File seeking caused the error\n");		
	}
	else {
		writeError = write(disk, block, BLOCK);
		if( writeError == -1 ) {
			printf("\nError -> File not read. File reading caused the error\n");
		}
	}
	return writeError;	
}

char* inodeToString(inodeBlock *inode) {
	int extraBuffer = 3;
	char *string = (char*)malloc(BLOCK);
	strcpy(string, inode->fileName);
	int times = 0;
	while( times <= inode->timesFetched ) {
		char start[4], end[4];
		strcat(string, " ");
		sprintf(start, "%d", inode->start[times]);
		sprintf(end, "%d", inode->end[times]);
		strcat(string, start);
		strcat(string, " ");
		strcat(string, end);
		times += 1;
	}
	return string;
}

int appendFile(int disk, char* fileName, void* block) {
	int i = 0;
	for( ; i < noOfFiles; i++ ) {
		if( strcmp(inode[i]->fileName, fileName) == 0 ) {
			inode[i]->timesFetched += 1;
			int sizeOfBlock = sizeof(char)*strLength((char*)block);
			int blocksRequired = 0;
			if( (sizeOfBlock % BLOCK) == 0 ) {
				blocksRequired = sizeOfBlock/BLOCK;
			}
			else {
				blocksRequired = (sizeOfBlock/BLOCK) + 1;	
			}
			inode[i]->start[inode[i]->timesFetched] = -1;
			int contiguous = 0;
			int j = 0;
			for( ; j < DATA_ENTRIES; j++ ) {
				if( dataBit->dBitmap[j] == '0' ) {
					contiguous += 1;
					if( contiguous == blocksRequired ) {
						// start tells data block position from 104 blocks w.r.t file system
						inode[i]->start[inode[i]->timesFetched] = j - blocksRequired + 1;
						inode[i]->end[inode[i]->timesFetched] = j;
						break;
					}
				}
				else {
					contiguous = 0;
				}
			}
			if( inode[i]->start[inode[i]->timesFetched] == -1 ) {
				printf("\nError -> File was not written correctly. Data entries are full\n");
			}
			for( j = inode[i]->start[inode[i]->timesFetched]; j <= inode[i]->end[inode[i]->timesFetched]; j++ ) {
				dataBit->dBitmap[j] = '1';
			}
			// copy niche se
			// data bitmap save
			int dataBitmapOffset = 0 + SUPER_BLOCK + INODE_BITMAP;
			int blockNumber = dataBitmapOffset/BLOCK;
			int writeError = writeData(disk, blockNumber, (void*)(dataBit->dBitmap));
			if( writeError == -1 ) {
				printf("\nError -> File was not written correctly\n");
			}
			// inode save
			int inodeOffset = 0 + SUPER_BLOCK + INODE_BITMAP + DATA_BITMAP + (i*BLOCK);
			char *string = inodeToString(inode[i]);
			blockNumber = inodeOffset/BLOCK;
			writeError = writeData(disk, blockNumber, (void*)(string));
			if( writeError == -1 ) {
				printf("\nError -> File was not written correctly\n");
			}
			// data save
			int dataOffset = 0 + SUPER_BLOCK + INODE_BITMAP + DATA_BITMAP + (INODE_ENTRIES*BLOCK) + (inode[i]->start[inode[i]->timesFetched]*BLOCK);
			blockNumber = dataOffset/BLOCK;
			writeError = writeData(disk, blockNumber, block);
			if( writeError == -1 ) {
				printf("\nError -> File was not written correctly\n");
			}
			return 1;
		}
	}
	printf("\nError -> No file found with the given fileName. Try again!\n");
	return -1;
}

int writeFile(int disk, char* fileName, void* block) {
	if( noOfFiles > INODE_ENTRIES ) {
		printf("\nError -> File System Crash. No more space for the file specified.\n");
		return -1;
	}
	else {
		if( noOfFiles != 0 ) {
			int i = 0;
			for( ; i < noOfFiles; i++ ) {
				if( strcmp(inode[i]->fileName, fileName) == 0 ) {
					printf("\nError -> File found with the same given fileName. Try again!\n");
					return -1;
				}
			}
		}
		inode[noOfFiles] = (inodeBlock*)malloc(sizeof(inodeBlock));
		inode[noOfFiles]->timesFetched = 0;
		inode[noOfFiles]->fileName = (char*)malloc(sizeof(char)*(strLength(fileName)+1));
		strcpy(inode[noOfFiles]->fileName, fileName);
		int sizeOfBlock = sizeof(char)*strLength((char*)block);
		int blocksRequired = 0;
		if( (sizeOfBlock % BLOCK) == 0 ) {
			blocksRequired = sizeOfBlock/BLOCK;
		}
		else {
			blocksRequired = (sizeOfBlock/BLOCK) + 1;	
		}
		// assign inode bitmap
		inodeBit->iBitmap[noOfFiles] = '1';
		
		// assign data bitmap

		// contiguous block check
		inode[noOfFiles]->start[0] = -1;
		int contiguous = 0;
		int i = 0;
		for( ; i < DATA_ENTRIES; i++ ) {
			if( dataBit->dBitmap[i] == '0' ) {
				contiguous += 1;
				if( contiguous == blocksRequired ) {
					// start tells data block position from 104 blocks w.r.t file system
					inode[noOfFiles]->start[0] = i - blocksRequired + 1;
					inode[noOfFiles]->end[0] = i;
					break;
				}
			}
			else {
				contiguous = 0;
			}
		}
		if( inode[noOfFiles]->start[0] == -1 ) {
			printf("\nError -> Filewas not written. Data Entries are full.\n");
			return -1;
		}
		for( i = inode[noOfFiles]->start[0]; i <= inode[noOfFiles]->end[0]; i++ ) {
			dataBit->dBitmap[i] = '1';
		}
		// saving to file system
		
		// inode bitmap save
		int inodeBitmapOffset = 0 + SUPER_BLOCK, writeError;
		int blockNumber = inodeBitmapOffset/BLOCK;
		writeError = writeData(disk, blockNumber, (void*)(inodeBit->iBitmap));
		if( writeError == -1 ) {
			printf("\nError -> File was not written correctly\n");
		}
		// data bitmap save
		int dataBitmapOffset = 0 + SUPER_BLOCK + INODE_BITMAP;
		blockNumber = dataBitmapOffset/BLOCK;
		writeError = writeData(disk, blockNumber, (void*)(dataBit->dBitmap));
		if( writeError == -1 ) {
			printf("\nError -> File was not written correctly\n");
		}
		// inode save
		int inodeOffset = 0 + SUPER_BLOCK + INODE_BITMAP + DATA_BITMAP + (noOfFiles*BLOCK);
		char *string = inodeToString(inode[noOfFiles]);
		blockNumber = inodeOffset/BLOCK;
		writeError = writeData(disk, blockNumber, (void*)(string));
		if( writeError == -1 ) {
			printf("\nError -> File was not written correctly\n");
		}
		// data save
		int dataOffset = 0 + SUPER_BLOCK + INODE_BITMAP + DATA_BITMAP + (INODE_ENTRIES*BLOCK) + (inode[noOfFiles]->start[0]*BLOCK);
		blockNumber = dataOffset/BLOCK;
		writeError = writeData(disk, blockNumber, block);
		if( writeError == -1 ) {
			printf("\nError -> File was not written correctly\n");
		}
		noOfFiles += 1;
		return 1;		
	}
}

int readFile(int disk, char* fileName, void* block) {
	int i = 0;
	for( ; i < noOfFiles; i++ ) {
		if( strcmp(inode[i]->fileName,fileName) == 0 ) {
			int times = 0;
			while( times <= inode[i]->timesFetched ) {
				int start = inode[i]->start[times];
				int dataOffset = 0 + SUPER_BLOCK + INODE_BITMAP + DATA_BITMAP + (INODE_ENTRIES)*BLOCK + (inode[i]->start[times]*BLOCK);
				while( start <= inode[i]->end[times] ) {
					char *blockData = (char*)malloc(sizeof(char)*BLOCK);
					int blockNumber = dataOffset/BLOCK;
					int readError = readData(disk, blockNumber, (void*)blockData);
					strcat((char*)block, blockData);
					strcat((char*)block, " ");
					dataOffset += 1;
					start += 1;
				}
				times += 1;
			}
			printf("\nContent of the file %s is as follows -\n\t%s\n", fileName, (char*)block);
			return 1;
		}
	}
	printf("\nError -> No file found with the given fileName. Try again!\n");
	return -1;
}

void print_inodeBitmaps(int fileSystemID) {
	int readError, inodeBitmapOffset = 0 + SUPER_BLOCK;
	int seekError = lseek(fileSystemID, inodeBitmapOffset, 0);
	if( seekError == -1 ) {
		printf("\nError -> File not read. File seeking caused the error\n");	
	}
	else {
		char *block = (char*)malloc(sizeof(char)*BLOCK);
		int blockNumber = (inodeBitmapOffset/BLOCK);
		readError = readData(fileSystemID, blockNumber, (void*)block);
		printf("\nInode Bitmap Status is as follows -\n%s\n", block);
		if( readError == -1 ) {
			printf("\nError -> File not read. File reading caused the error\n");
		}
	}
}

void print_dataBitmaps(int fileSystemID) {
	int readError, dataBitmapOffset = 0 + SUPER_BLOCK + INODE_BITMAP;
	int seekError = lseek(fileSystemID, dataBitmapOffset, 0);
	if( seekError == -1 ) {
		printf("\nError -> File not read. File seeking caused the error\n");	
	}
	else {
		char *block = (char*)malloc(sizeof(char)*BLOCK);
		int blockNumber = (dataBitmapOffset/BLOCK);
		readError = readData(fileSystemID, blockNumber, (void*)block);
		printf("\nData Bitmap Status is as follows -\n%s\n", block);
		if( readError == -1 ) {
			printf("\nError -> File not read. File reading caused the error\n");
		}
	}
}

void print_FileList(int fileSystemID) {
	int readError, inodeOffset = 0 + SUPER_BLOCK + INODE_BITMAP + DATA_BITMAP;
	int seekError = lseek(fileSystemID, inodeOffset, 0);
	if( seekError == -1 ) {
		printf("\nError -> File not read. File seeking caused the error\n");	
	}
	else {
		int files = noOfFiles, blockNumber = (inodeOffset/BLOCK), count = 1;
		char *block = (char*)malloc(sizeof(char)*BLOCK);
		while( files != 0 ) {
			readError = readData(fileSystemID, blockNumber, (void*)block);
			printf("\nInode Entry %d -\t%s",count, block);
			if( readError == -1 ) {
				printf("\nError -> File not read. File reading caused the error\n");
			}
			files -= 1;
			blockNumber += 1;
			count += 1;
		}
	}
	printf("\n");
}

int main() {
	// call createSFS once
	char *fileName = (char*)malloc(sizeof(char)*BLOCK);
	int nBytes;
	printf("Enter the name of the File System :- ");
	scanf("%s",fileName);
	strcat(fileName, ".txt");
	printf("Enter the size of the File System in KB:- ");
	scanf("%d",&nBytes);
	int fileSystemID = createSFS(fileName, nBytes);
	char *newFileName = (char*)malloc(sizeof(char)*BLOCK);
	char *newFileData = (char*)malloc(sizeof(char)*BLOCK);
	if( fileSystemID ) {
		printf("\nWelcome to File System %s\n", fileName);
		while(true) {
			showMenu();
			int input;
			scanf("%d",&input);
			switch(input) {
				case 1: // make a new file -> writeFile
						printf("\nEnter the name of the file you want to create :- ");
						scanf("%s",newFileName);
						strcat(newFileName, ".txt");
						scanf("%*c");
						printf("\nEnter the content of the file you want to create :- ");
						if( fgets(newFileData, BLOCK, stdin) ) {
							newFileData[strlen(newFileData)-1] = '\0';
						}
						int success = writeFile(fileSystemID, newFileName, (void*)newFileData);
						if( success == -1 ) {
							printf("\nError -> File was not created.\n");
						}
						else {
							printf("\n%s File was created successfully\n", newFileName);
						}
						break;
				case 2: // write to already created file -> writeData
						printf("\nEnter the name of the file you want to edit :- ");
						scanf("%s",newFileName);
						strcat(newFileName, ".txt");
						scanf("%*c");
						printf("\nEnter the content of the file you want to create :- ");
						if( fgets(newFileData, BLOCK, stdin) ) {
							newFileData[strlen(newFileData)-1] = '\0';
						}
						success = appendFile(fileSystemID, newFileName, (void*)newFileData);
						if( success == -1 ) {
							printf("\nError -> File did not exist.\n");
						}
						else {
							printf("\n%s File was edited successfully\n", newFileName);
						}
						break;
				case 3: // read an already created file -> readFile
						printf("\nEnter the name of the file you want to read :- ");
						scanf("%s",newFileName);
						strcat(newFileName, ".txt");
						strcpy(newFileData, "");
						success = readFile(fileSystemID, newFileName, (void*)newFileData);
						if( success == -1 ) {
							printf("\nError -> File was not found.\n");
						}
						else {
							printf("\n%s File was read successfully\n", newFileName);
						}
						break;
				case 4: // print inode_bitmaps
						print_inodeBitmaps(fileSystemID);
						break;
				case 5: // print data_bitmaps
						print_dataBitmaps(fileSystemID);
						break;
				case 6: // print file list
						print_FileList(fileSystemID);
						break;
				case 7: // exit
						exit(0);
						break;
				default: printf("\nSorry, Wrong input provided\n");
						 exit(0);
			}
		}
	}
	else {
		exit(0);
	}
	return 0;
}