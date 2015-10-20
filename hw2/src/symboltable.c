#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include<math.h>
#include"header.h"

#define TABLE_SIZE	256

symtab * hash_table[TABLE_SIZE];
extern int linenumber;
extern int idCounter;
extern char* comments;

int HASH(char * str){
	int idx=0;
	while(*str){
		idx = idx << 1;
		idx+=*str;
		str++;
	}	
	return (idx & (TABLE_SIZE-1));
}

/*returns the symbol table entry if found else NULL*/

symtab * lookup(char *name){
	int hash_key;
	symtab* symptr;
	if(!name)
		return NULL;
	hash_key=HASH(name);
	symptr=hash_table[hash_key];

	while(symptr){
		if(!(strcmp(name,symptr->lexeme)))
			return symptr;
		symptr=symptr->front;
	}
	return NULL;
}


void insertID(char *name){
	int hash_key;
	symtab* ptr;
	symtab* symptr=(symtab*)malloc(sizeof(symtab));	
	
	hash_key=HASH(name);
	ptr=hash_table[hash_key];
	
	if(ptr==NULL){
		/*first entry for this hash_key*/
		hash_table[hash_key]=symptr;
		symptr->front=NULL;
		symptr->back=symptr;
	}
	else{
		symptr->front=ptr;
		ptr->back=symptr;
		symptr->back=symptr;
		hash_table[hash_key]=symptr;	
	}
	
	strcpy(symptr->lexeme,name);
	symptr->line=linenumber;
	symptr->counter=1;
    symptr->reserved = isReserved(name);
    //printf("%d\n",symptr->reserved);
}

void printSym(symtab* ptr) 
{
    /*if(ptr->reserved){
        printf("reserved: %s\n",ptr->lexeme);
    }*/
	    printf(" Name = %s \n", ptr->lexeme);
	    printf(" References = %d \n", ptr->counter);
}

void printSymTab()
{
    int i;
    int n = idCounter;
    int j = 0;
    printElem p[n];
    printf("----- Symbol Table ---------\n");
    for (i=0; i<TABLE_SIZE; i++)
    {
        symtab* symptr;
        symptr = hash_table[i];
        while (symptr != NULL)
        {
            printf("====>  index = %d \n", i);
            if(symptr->reserved == 0){
                p[j++] = copyPrintElem(symptr);
            }
            printSym(symptr);
            symptr=symptr->front;
        }
    }
    printf("----- comments -----\n");
    printf("%s", comments);
    qsort(p, j, sizeof(printElem), cmp);
    //printf("number is: %d\n",j);
    printf("----- sorted result ------\n");
    for(i = 0; i < j ; i++){
        printf("%-20s%d\n", p[i].lexeme, p[i].counter);
    }
}
printElem copyPrintElem(symtab* ptr){
    printElem* temp = (printElem*)malloc(sizeof(printElem));
    strcpy(temp->lexeme, ptr->lexeme);
    temp->counter = ptr->counter;
    return *temp;
}
int isReserved(char* name){
    if(strcmp(name, "while") == 0){
        return 1;
    }
    if(strcmp(name, "for") == 0){
        return 1;
    }
    if(strcmp(name, "return") == 0){
        return 1;
    }
    if(strcmp(name, "typedef") == 0){
        return 1;
    }
    if(strcmp(name, "void") == 0){
        return 1;
    }
    if(strcmp(name, "int") == 0){
        return 1;
    }
    if(strcmp(name, "float") == 0){
        return 1;
    }
    if(strcmp(name, "if") == 0){
        return 1;
    }
    if(strcmp(name, "else") == 0){
        return 1;
    }
    return 0;

}
int cmp(const void* a, const void* b){
    printElem elem1 = *(printElem*)a;
    printElem elem2 = *(printElem*)b;
    return strcmp(elem1.lexeme, elem2.lexeme);
}
