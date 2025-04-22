#include <stdio.h>
#include "base.c"

typedef struct {
    char* name;
    char (*func)(char);
} fun_desc;


int main(int argc, char ** argv){
    char* carray = calloc(5, sizeof(char)); // 1. Define a pointer 'carray' to a char array of length 5, initialized to the empty string.
    char* buffer = calloc(3, sizeof(char)); // A buffer for the input number
    fun_desc menu[] = {{"cprt", cprt}, {"my_get", my_get}, {"encrypt", encrypt}, {"decrypt", decrypt}, {"xprt", xprt}, {"dprt", dprt}, {NULL, NULL}};
    int input;

    while(1){
        printf("Select operation from the following menu:\n");
        // 3. Display a menu
        for(int i = 0; menu[i].name != NULL; i++){
        printf("%d) %s\n", i+1, menu[i].name);
        }
        // 4. Choose from the menu
        if(fgets(buffer,3,stdin) == NULL) exit(0);
        input = atoi(buffer);
        // check bounds
        if(input < 1 || input > 6){
        printf("Not within bounds\n");
        free(carray);
        free(buffer);
        exit(0);
        }
        printf("Within bounds\n");
        // 5.Evaluate the appropriate function
        char* temp = map(carray, 5, menu[input-1].func);
        free(carray);
        carray = temp;
    }
    return 0;
}