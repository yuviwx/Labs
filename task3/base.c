#include <stdlib.h>
#include <stdio.h>
#include <string.h>
 
/* Ignores c, reads and returns a character from stdin using fgetc. */
char my_get(char c){
  return fgetc(stdin);
}

/* If c is a number between 0x20 and 0x7E, cprt prints the character of ASCII value c followed by a new line. 
   Otherwise, cprt prints the dot ('.') character. After printing, cprt returns the value of c unchanged. */
char cprt(char c){
  (c >= 0x20 && c <= 0x7E) ? printf("%c\n", c) : printf(".");
  return c;
}

/* Gets a char c and returns its encrypted form by adding 1 to its value. If c is not between 0x1F and 0x7E it is returned unchanged */
char encrypt(char c){
  return (c >= 0x1F && c <= 0x7E) ? c + 1 : c;
}

/* Gets a char c and returns its decrypted form by reducing 1 from its value. If c is not between 0x21 and 0x7F it is returned unchanged */
char decrypt(char c){
  return (c >= 0x21 && c <= 0x7F) ? c - 1 : c;
}

/* xprt prints the value of c in a hexadecimal representation followed by a new line, and returns c unchanged. */
char xprt(char c){
  printf("%x\n", c);
  return c;
}

/* dprt prints the value of c in a decimal representation followed by a new line, and returns c unchanged. */
char dprt(char c){
  printf("%c\n", c);
  return c;
}


char* map(char *array, int array_length, char (*f) (char)){
  char* mapped_array = (char*)(malloc(array_length*sizeof(char))); // the new array
  /* TODO: Complete during task 2.a */
  for(int i = 0; i < array_length; i++){
    mapped_array[i] = f(array[i]); // apply f on each cell
  }
  return mapped_array;
}

