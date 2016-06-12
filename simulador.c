#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define DEBUG(A) //A

typedef struct stTableElement {
	short lastAccess;
  char isReferenced;
  char isModified;
} tpPage;

char *substAlg;
int pageSize, memSize;
int pageSizeBitsBoundery;
unsigned int instructions[500][2]; // [adress, operation (0 - read 1 - write]
tpPage pageTable[600000];
int referencedCount = 0;
int time = 0;
int writingCount = 0;
int pageFaultCount = 0;

void readInstructions(char *path) {
	char operation;
	memset(instructions, 0x00, 500*2*sizeof(int));
	freopen(path, "r", stdin);
	DEBUG(printf("===> Reading instructions\n");)
	while(scanf("%x %c", &instructions[instructions[0][0]+1][0], &operation) != EOF) {
		instructions[instructions[0][0]+1][1] = (operation == 'R' ? 0 : 1);
		instructions[0][0]++;
		DEBUG(printf("%x %d\n", instructions[instructions[0][0]][0], instructions[instructions[0][0]][1]);)
	}
	DEBUG(printf("%d instructions readed\n<===\n\n\n", instructions[0][0]);)
}

tpPage *nextPageToBeReplaced() {
	unsigned int i;
	if(strcmp(substAlg, "lru") == 0) {
		tpPage *leastRecent = NULL;
		for(i = 0; i < 600000; i++) {
			if(pageTable[i].isReferenced &&
					(leastRecent == NULL ||
					 pageTable[i].lastAccess < leastRecent->lastAccess)) {
				leastRecent = pageTable + i;
			}
		}
		return leastRecent;
	} else if(strcmp(substAlg, "nru") == 0) {
	} else if(strcmp(substAlg, "seg") == 0) {
	}
	printf("Choose one of those page replace algorithm: LRU, NRE, SEG");
	exit(1);
}

char *uniformizeAlgorithmName(char *name) {
	int i;
	for(i = 0; i < strlen(name); i++) {
		name[i] = tolower(name[i]);
	}
	return name;
}

void resetPage(tpPage *page) {
	memset(page, 0x00, sizeof(tpPage));
}

int main(int argc, char *argv[])
{
	int i;	
	substAlg = uniformizeAlgorithmName(argv[1]);
	readInstructions(argv[2]);
	pageSize = atoi(argv[3]);
	memSize = atoi(argv[4]);
	pageSizeBitsBoundery = (int) ceil(log2(pageSize*1024));

	memset(pageTable, 0x00, 600000*sizeof(tpPage));
	for(i = 1; i <= instructions[0][0]; i++) {
		unsigned int address = instructions[i][0];
		unsigned int isWriting = instructions[i][2];
		unsigned int logicalAddress = address >> pageSizeBitsBoundery;

		time++;

		// Page fault?
		if((referencedCount+1)*pageSize > memSize) {
			// Use one of the methods to replace page
			pageFaultCount++;
			
			tpPage *pageToBeReplaced = nextPageToBeReplaced();
			if(pageToBeReplaced != NULL) {
				resetPage(pageToBeReplaced);
				referencedCount--;
			}
			
		}

		pageTable[logicalAddress].lastAccess = time;
		if(pageTable[logicalAddress].isReferenced) {
			// address are already referenced
		} else {
			pageTable[logicalAddress].isReferenced = 1;
			referencedCount++;
		}

		if(isWriting) {
			pageTable[logicalAddress].isModified = 1;
			writingCount++;
		} else {
			// is reading
		}
	}

	printf("Executando o simulador...\n");
	printf("Arquivo de entrada: %s\n", argv[2]);
	printf("Tamanho da memoria fisica: %d KB\n", memSize);
	printf("Tamanho das páginas: %d KB\n", pageSize);
	printf("Alg de substituição: %s\n", substAlg);
	printf("Numero de Faltas de Páginas: %d\n", pageFaultCount);
	printf("Numero de Paginas escritas: %d\n", writingCount);

  return 0;
}
