#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Global variables
FILE* infile;
FILE* outfile;
int addition; // 0 - subtraction, 1 - addition
char* encoding = "0"; // default - no encoding
int encoding_index = 0;


// Function declarations
void handle_arguments(int argc, char **argv);
char encode(char c);
int alphanumeric(char c);
void handle_input();
int len(char* s);

// Main function - reads the input file and encodes the characters
int main(int argc, char **argv) {
    infile = stdin;
    outfile = stdout;

    handle_arguments(argc, argv);
    handle_input();
    return 0;
}

/* Handle command line arguments - 
#  D - sets the debug mode, +D - activates(1), -D - deactivates(0)
#  E - sets the encoding pattern
#  i/o - sets the input/output files*/
void handle_arguments(int argc, char **argv) {
    int debug_mode = 1; 

    for(int i=1; i<argc; i++) {
        if(strcmp(argv[i], "-D") == 0){
            debug_mode = 0; 
        }
        else if(strcmp(argv[i], "+D") == 0){
            debug_mode = 1;
        }
        else if(strncmp(argv[i], "+E",2) == 0){
            addition = 1;    
            encoding = argv[i]+2;

        }
        else if(strncmp(argv[i], "-E",2) == 0){
            addition = 0;    
            encoding = argv[i]+2;

        }
        else if(strncmp(argv[i], "-i",2) == 0){
            infile = fopen(&argv[i][2], "r");

        }
        else if(strncmp(argv[i], "-o",2) == 0){
            char *file = argv[i]+2;
            outfile = fopen(file, "a");
        }
        else if(debug_mode){
            fprintf(stderr, "%s\n", argv[i]);
        }
    }

    // check if the files are opened successfully
    if (infile == NULL || outfile == NULL) {
       fprintf(stderr, "Error opening file");
       exit(1);
   }
}

// Read the input file and encode the characters
void handle_input() {
    int encoding_len = len(encoding);
    int c;

    while((c=fgetc(infile)) != EOF) {
        if(alphanumeric(c)){
            c = encode(c);
            encoding_index = (encoding_index + 1) % encoding_len;
        }
        fputc(c, outfile);
    }  

    if (infile != stdin) fclose(infile);
    if (outfile != stdout) fclose(outfile);
}

// encode the character based on the encoding mode, remember to handle the wrap around.
char encode(char c) {
    int shift = (encoding[encoding_index] - '0') * (addition ? 1 : -1);

    if (c >= '0' && c <= '9') {
        return ((c - '0' + shift + 10) % 10) + '0'; 
    } else if (c >= 'A' && c <= 'Z') {
        return ((c - 'A' + shift + 26) % 26) + 'A'; 
    }
    return ((c - 'a' + shift + 26) % 26) + 'a';
}

// check if the character is alphanumeric - for each option an if statement with 2 conditions
int alphanumeric(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9');
}

int len(char* s) {
    int i = 0;
    while(s[i] != '\0') i++;
    return i;
}