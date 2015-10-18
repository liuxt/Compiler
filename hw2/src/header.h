struct symtab{
	char lexeme[256];
	struct symtab *front;
	struct symtab *back;
	int line;
    int reserved;                   // 0 if not reserved, 1 reserved
	int counter;
};
struct printElem{
    char lexeme[256];
    int counter;
};

typedef struct symtab symtab;
typedef struct printElem printElem;
symtab * lookup(char *name);
void insert(char *name);
int isReserved(char* name);
printElem copyPrintElem(symtab* ptr);
int cmp(const void* a, const void* b);
