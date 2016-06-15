#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define DEBUG(A...)// printf("<DEBUG> "); printf(A)
#define RECENT_DELTA_TIME 100

typedef struct stTableElement {
	short lastAccess;
  char isReferenced;
  char isModified;
  char isPresent;
  char isReferencedSeg;
} tpPage;

typedef struct circularQueue {
    tpPage *pages[3000];
    int begin;
    int end;
} queue;

typedef struct stListEl {
    struct stListEl *next;
    tpPage *page;
} tpListEl;

typedef struct stList {
    tpListEl *head;
} tpList;

char *substAlg;
int pageSize, memSize;
int pageSizeBitsBoundery;
tpPage pageTable[600000];
queue Q;
int presentCount = 0;
int time = 0;
int writingCount = 0;
int pageFaultCount = 0;
tpList presentPages;

tpPage *lruAlgorithm() {
	tpPage *leastRecent = NULL;
	tpListEl *element = presentPages.head;
	while(element != NULL) {
		tpPage *page = element->page;
		if(leastRecent == NULL ||
				 page->lastAccess < leastRecent->lastAccess) {
			leastRecent = page;
		}
		element = element->next;
	}
	return leastRecent;
}

void append(tpPage *page) {
    Q.end = (Q.end + 1) % 3000;
    Q.pages[Q.end] = page;
}

void addPresentPage(tpPage *page) {
	if(page->isPresent) return;

	tpListEl *element = (tpListEl*) malloc(sizeof(tpListEl));
	element->page = page;
	element->next = presentPages.head;
	presentPages.head = element;
}

void removePresentPage(tpPage *page) {
	tpListEl *prev, *curr;
	prev = presentPages.head;
	curr = presentPages.head->next;
	if(curr == NULL || prev->page == page) {
		presentPages.head = prev->next;
		free(prev);
	} else {
		while(curr->page != page) {
			prev = curr;
			curr = curr->next;
		}
		prev->next = curr->next;
		free(curr);
	}
}

tpPage *nruAlgorithm() {
	int minPoints = 5;
	tpPage *choosed = NULL;
	tpListEl *pageEl = presentPages.head;
	while(pageEl != NULL) {
		int points;
		tpPage *page = pageEl->page;

		points = (page->isReferenced ? 2 : 0) + (page->isModified ? 1 : 0);
		if(points == 0) return page;
		if(points <= minPoints) {
			points = minPoints;
			choosed = page;
		}
		pageEl = pageEl->next;
	}
	return choosed;
}

tpPage *secAlgorithm() {
    tpPage* first = Q.pages[Q.begin];
    Q.begin = (Q.begin + 1) % 3000;
    if(first->isReferencedSeg == 1) {
        first->isReferencedSeg = 0;
        append(first);
        return secAlgorithm();
    }
    return first;
}

tpPage *nextPageToBeReplaced() {
	if(strcmp(substAlg, "lru") == 0) {
		return lruAlgorithm();
	} else if(strcmp(substAlg, "nru") == 0) {
		return nruAlgorithm();
	} else if(strcmp(substAlg, "seg") == 0) {
    return secAlgorithm();
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
	tpListEl *element = presentPages.head;
	while(element != NULL) {
		tpPage *page = element->page;
		page->isReferenced = 0;
		element = element->next;
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
			removePresentPage(pageToBeReplaced);
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
  pageTable[pageAddress].isReferencedSeg = 1;
	if(pageTable[pageAddress].isPresent) {
		DEBUG("Page %x are already in memory\n", pageAddress);
	} else {
		DEBUG("Page %x are not in memory\n", pageAddress);

		verifyAndTreatPageFault(pageAddress);
    append(&pageTable[pageAddress]);
    addPresentPage(&pageTable[pageAddress]);
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
	unsigned int address;
	char operation;
	Q.begin = 0;
	Q.end = -1;
	presentPages.head = NULL;

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
