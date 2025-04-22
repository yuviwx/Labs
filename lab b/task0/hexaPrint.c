#include <stdio.h>
#include <stdlib.h>

void PrintHex(unsigned char *buffer, int length);

int main (int argc, char ** argv){
    int length;
    int index = 0;
    unsigned char* buffer = (char*)malloc(16);
    
    if(buffer == NULL){
        perror("Memory not allocated.");
        return 1;
    }

    // receive name and open file
    if(argc != 2){
        fprintf(stderr ,"Arguments error: 2 was expected, %d was entered.",argc);
        free(buffer);
        return 1;
    }
    FILE* file = fopen(argv[1],"rb");
    if(file == NULL){
        perror("error opening the file");
        free(buffer);
        return(1);
    }

    // read 
    do {
        length = fread(buffer, 1, 16, file);
        PrintHex(buffer, length);
    }while(length != 0);
    printf("\n");

    free(buffer);
    fclose(file);
    return 0;
}

// This function prints the buffer in hexadecimal value(more explanation below)
void PrintHex(unsigned char *buffer, int length){
    for(int i = 0; i < length; i++)
        printf("%02x ", buffer[i]);    
}

/* Detailed explanation of PrintHex:
* Byte is 8 bits, thus is value is between 0 and 255.
* Two digits of hexadecimal's value is also between 0 and 255,
* so printHex prints each char separately, meaning each char will be presented as 2 numbers in base 16(padding with 0 if needed).
*/