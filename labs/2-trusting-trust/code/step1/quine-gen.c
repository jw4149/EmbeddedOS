// convert the contents of stdin to their ASCII values (e.g., 
// '\n' = 10) and spit out the <prog> array used in Figure 1 in
// Thompson's paper.
#include <stdio.h>

int main(void) { 
    // put your code here.
    char ch;
    char prog[10000];
    int k = 0;
    while ((ch = getchar()) != EOF) {
        prog[k] = ch;
        k += 1;
    }

    prog[k] = 0;

    int i;
    printf("char prog[] = {\n");
	for(i = 0; prog[i]; i++)
		printf("\t%d,%c", prog[i], (i+1)%8==0 ? '\n' : ' ');
	printf("0 };\n");
	printf("%s", prog);
    
	return 0;
}
