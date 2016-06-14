#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define DEBUG(A...) printf("<DEBUG> "); printf(A)
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
int presentCount = 0;
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
	int minPoints;
	tpPage *choosed = NULL;
	for(i = 0; i < 600000; i++) {
		int points = (pageTable[i].isReferenced ? 2 : 0) + (pageTable[i].isModified ? 1 : 0);
		if(points == 0) return (pageTable+i);
		if(points <= minPoints) {
			points = minPoints;
			choosed = pageTable+i;
		}
	}
	return choosed;
}

tpPage *nextPageToBeReplaced() {
	if(strcmp(substAlg, "lru") == 0) {
		return lruAlgorithm();
	} else if(strcmp(substAlg, "nru") == 0) {
		return nruAlgorithm();
	} else if(strcmp(substAlg, "seg") == 0) {
		// seg
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
	}
}

void verifyAndTreatPageFault(unsigned int pageAddress) {
	// Page fault?
	if((presentCount+1)*pageSize > memSize) {
		DEBUG("Page fault!\n");
		pageFaultCount++;
		
		tpPage *pageToBeReplaced = nextPageToBeReplaced();

		if(pageToBeReplaced->isModified) {
			DEBUG("Writing page before replacing it");
			writingCount++;
		}

		if(pageToBeReplaced == NULL) {
			DEBUG("Error, no page choosed");
			exit(1);
		} else {
			resetPage(pageToBeReplaced);
		}
	} else {
		presentCount++;
	}
}

void executeInstruction(unsigned int address, int isWriting) {
	unsigned int pageAddress = address >> pageSizeBitsBoundery;
	DEBUG("%s address %x (page: %x) - time: %d\n", (isWriting ? "Writing on" : "Reading"), address, pageAddress, time);

	pageTable[pageAddress].lastAccess = time;
	pageTable[pageAddress].isReferenced = 1;
	if(pageTable[pageAddress].isPresent) {
		DEBUG("Page %x are already in memory\n", pageAddress);
	} else {
		DEBUG("Page %x are not in memory\n", pageAddress);

		verifyAndTreatPageFault(pageAddress);
		pageTable[pageAddress].isPresent = 1;
	}

	if(isWriting) {
		pageTable[pageAddress].isModified = 1;
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
	DEBUG("Numero de Paginas em memória: %d\n", presentCount);

  return 0;
}
