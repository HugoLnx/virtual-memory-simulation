#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define DEBUG(A...) //printf("<DEBUG> "); printf(A)
#define RECENT_DELTA_TIME 100

typedef struct stTableElement {
	short lastAccess;
  char isReferenced;
  char isModified;
  char isPresent;
} tpPage;

char *substAlg;
int pageSize, memSize;
int pageSizeBitsBoundery;
tpPage pageTable[600000];
int referencedCount = 0;
int time = 0;
int writingCount = 0;
int pageFaultCount = 0;

tpPage *lruAlgorithm() {
	unsigned int i;
	tpPage *leastRecent = NULL;
	for(i = 0; i < 600000; i++) {
		if(pageTable[i].isPresent &&
				(leastRecent == NULL ||
				 pageTable[i].lastAccess < leastRecent->lastAccess)) {
			leastRecent = pageTable + i;
		}
	}
	return leastRecent;
}

tpPage *nruAlgorithm() {
	unsigned int i;
	int maxPoints;
	tpPage *choosed = NULL;
	for(i = 0; i < 600000; i++) {
		int points = (pageTable[i].isReferenced ? 2 : 0) + (pageTable[i].isModified ? 1 : 0);
		if(points == 3) return (pageTable+i);
		if(points >= maxPoints) {
			points = maxPoints;
			choosed = pageTable+i;
		}
	}
	return choosed;
}

tpPage *nextPageToBeReplaced() {
	unsigned int i;
	if(strcmp(substAlg, "lru") == 0) {
		return lruAlgorithm();
	} else if(strcmp(substAlg, "nru") == 0) {
		return nruAlgorithm();
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

void resetRecentUsageBits() {
	int i;
	for(i = 0; i < 600000; i++) {
		pageTable[i].isReferenced = 0;
		pageTable[i].isModified = 0;
	}
}

void verifyAndTreatPageFault(unsigned int logicalAddress) {
	// Page fault?
	if((referencedCount+1)*pageSize > memSize) {
		DEBUG("Page fault!\n");
		// Use one of the methods to replace page
		pageFaultCount++;
		
		tpPage *pageToBeReplaced = nextPageToBeReplaced();
		if(pageToBeReplaced != NULL) {
			resetPage(pageToBeReplaced);
		}
		
	}
}

void executeInstruction(unsigned int address, int isWriting) {
	unsigned int logicalAddress = address >> pageSizeBitsBoundery;
	DEBUG("%s address %x (logical: %x) - time: %d\n", (isWriting ? "Writing on" : "Reading"), address, logicalAddress, time);

	pageTable[logicalAddress].lastAccess = time;
	if(pageTable[logicalAddress].isPresent) {
		DEBUG("Page %x are already in memory\n", logicalAddress);
	} else {
		DEBUG("Page %x are not in memory\n", logicalAddress);

		verifyAndTreatPageFault(logicalAddress);
		pageTable[logicalAddress].isPresent = 1;
		pageTable[logicalAddress].isReferenced = 1;
		referencedCount++;
	}

	if(isWriting) {
		pageTable[logicalAddress].isModified = 1;
		writingCount++;
	} else {
		// is reading
	}
	DEBUG("\n\n");
}

int main(int argc, char *argv[])
{
	int i;
	unsigned int address;
	char operation;

	printf("Executando o simulador...\n");

	freopen(argv[2], "r", stdin);
	memset(pageTable, 0x00, 600000*sizeof(tpPage));

	substAlg = uniformizeAlgorithmName(argv[1]);
	pageSize = atoi(argv[3]);
	memSize = atoi(argv[4]);
	pageSizeBitsBoundery = (int) ceil(log2(pageSize*1024));

	while(scanf("%x %c", &address, &operation) != EOF) {
		time++;
		if(time % RECENT_DELTA_TIME == 0) resetRecentUsageBits();
		executeInstruction(address, (operation == 'R' ? 0 : 1));
	}


	printf("Arquivo de entrada: %s\n", argv[2]);
	printf("Tamanho da memoria fisica: %d KB\n", memSize);
	printf("Tamanho das páginas: %d KB\n", pageSize);
	printf("Alg de substituição: %s\n", substAlg);
	printf("Numero de Faltas de Páginas: %d\n", pageFaultCount);
	printf("Numero de Paginas escritas: %d\n", writingCount);

  return 0;
}
