#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int bigEndian = 0; // msb comes in the lowest memory address
char* fileName = NULL;

typedef struct virus {
    unsigned short SigSize;
    char virusName[16];
    unsigned char* sig;
} virus;

typedef struct link {
    struct link *nextVirus;
    virus *vir;
} link;
link* virus_list = NULL;

typedef struct {
    char* name;
    void (*func)();
} fun_desc;

// Declarations - virus functions
virus* readVirus(FILE* file);
void printVirus(virus* virus, FILE* output);

// Declarations - help functions
int read(void* buffer, int size, FILE* file, virus* v);
void getMagicNumber(FILE* file);

// Declarations - link functions
void list_print(link *virus_list, FILE* file);
link* list_append(link* virus_list, virus* data);
void list_free(link *virus_list);

// Declerations - menu functions
void menu();
void load_signatures();
void print_signatures();
void detect_viruses();
void fix_file();
void quit();
FILE* getFile();
void detect_virus(char *buffer, unsigned int size, link *virus_list);
void neutralize_virus(char *fileName, int signatureOffset);

// open file, get magic number and print all the viruses
int main(int argc, char **argv){
    menu();
    return 0;
}

// Reads the next virus in the file
virus* readVirus(FILE* file){
    // Allocate memory
    virus* v = malloc(sizeof(virus));
    if(v == NULL){
        perror("unable to allocate memory");
        fclose(file);
        exit(1);
    }

    // Parse the file:
    // SigSize & VirusName - 2 byte(short) + 16 bytes(name)
    if(!read(v,18,file,v)) return NULL;
    if(bigEndian) v->SigSize = (v->SigSize >> 8) | (v -> SigSize << 8); 

    // Signature
    v->sig = (unsigned char *) malloc(v->SigSize);
    if(v->sig == NULL) {
        perror("Unable to allocate memory for signature");
        free(v);
        fclose(file);
        free(fileName);
        exit(1);
    }
    if(!read(v->sig, v->SigSize,file,v)) return NULL;

    return v;
}

// Prints the the virus name (in ASCII),
//            the virus signature length (in decimal),
//            and the virus signature (in hexadecimal).
void printVirus(virus* virus, FILE* output){
    fprintf(output,"Virus name: %s\nVirus size: %u\nsignature:\n", virus->virusName, virus->SigSize);
    for(int i = 0; i < virus->SigSize; i++)
        fprintf(output, "%02x ", virus->sig[i]); 

    fprintf(output, "\n\n");      
}


int read(void* buffer, int size, FILE* file, virus* v){
    if(fread(buffer,size,1,file) != 1){
        free(v);
        return 0;
    }
    return 1;
}

void getMagicNumber(FILE* file){
    // read the magic number
    char buffer[5] = {0}; // initialize buffer to 0 because strcmp - compare 2 null terminated strings
    if(!read(buffer,4,file,NULL)) {fclose(file); exit(1);}
    // check for error and save the right endian 
    if(strcmp(buffer, "VIRB") == 0)
        bigEndian = 1;
    else if(strcmp(buffer, "VIRL") != 0){
        fprintf(stderr, "Invalid magic number: %s\n", buffer);
        fclose(file);
        exit(1);
    }
}

// link functions:

// Print the data of every link in list to the given stream. Each item followed by a newline character
// If no file is loaded, nothing is printed
void list_print(link *virus_list, FILE*){
    link* pointer = virus_list;
    while(pointer != NULL){
        printVirus(pointer->vir, stdout);
        pointer = pointer->nextVirus;
    }
}

/* Add a new link with the given data to the list (at the end CAN ALSO AT BEGINNING), 
*  and return a pointer to the list (i.e., the first link in the list). 
*  If the list is null - create a new entry and return a pointer to the entry.*/
link* list_append(link* virus_list, virus* data){
    // Allocate memory
    link* head = (link*)malloc(sizeof(link));
    if(head == NULL){
        perror("memory allocation error");
        list_free(virus_list);
        exit(1);
    }
    // Add to head
    head->vir = data;
    head->nextVirus = virus_list;
    return head;
    
}

/* Free the memory allocated by the list. */
void list_free(link *virus_list){
    link* temp;
    while(virus_list != NULL){
        free(virus_list->vir->sig);
        free(virus_list->vir);
        temp = virus_list;
        virus_list = virus_list->nextVirus;
        free(temp);
    }
}

void menu(){
    char buffer[3];
    fun_desc menu[] = {
        {"Load signatures", load_signatures},
        {"Print signatures", print_signatures}, 
        {"Detect viruses", detect_viruses}, 
        {"Fix file", fix_file}, 
        {"Quit", quit}, 
        {NULL, NULL}
    };

    int result, option;
    while(1){
        printf("Select operation from the following menu:\n");
        // 3. Display a menu
        for(int i = 0; menu[i].name != NULL; i++){
        printf("%d) %s\n", i+1, menu[i].name);
        }
        // Choose from the menu
        printf("Enter your choice: ");
        if(fgets(buffer,3,stdin) == NULL) exit(0);
        result = sscanf(buffer, "%d",&option);
        // check bounds
        if(result == EOF || option < 1 || option > 5){
            printf("Not within bounds\n");
            continue;
        }
        // Evaluate the appropriate function
        menu[option-1].func();
    }
    free(fileName);
}

void load_signatures(){ 

    FILE* file = getFile();
    if(file == NULL) return;

    // Load the signatures - 
    getMagicNumber(file);
    
    virus* v;
    list_free(virus_list); // free the old list
    virus_list = NULL;

    while((v = readVirus(file)) != NULL){
        virus_list = list_append(virus_list,v);
    }

    printf("File loaded successfully\n\n");
    fclose(file);
}

// Print the viruses
void print_signatures(){
    list_print(virus_list,stdout); 
}

void detect_viruses(){
    FILE* file = getFile();
    if(file == NULL) return;

    char buffer[10000] = {0};
    int size = fread(buffer,1,10000,file);
    if(size == 0) {
        perror("couldn't read file");
        return;
    }
    detect_virus(buffer, size, virus_list);
    printf("\n");
    fclose(file);
}

void fix_file(){
    FILE* file = getFile();
    if(file == NULL) return;

    char buffer[10000] = {0};
    int size = fread(buffer,1,10000,file);
    if(size == 0) {
        perror("couldn't read file");
        return;
    }

    link* curr = virus_list;
    virus* v = NULL;
    // for every virus go through the all file byte-by-byte
    while(curr){
        v = curr->vir;
        for(int i = 0; i <= (size - v->SigSize); i++){
            if(memcmp(&buffer[i],v->sig,v->SigSize) == 0){
                neutralize_virus(fileName, i);
            }
        }
        curr = curr->nextVirus;
    }
    fclose(file);
}

void neutralize_virus(char *fileName, int signatureOffset){
    // open file
    FILE* file = fopen(fileName, "rb+");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    // move the cruser
    int result = fseek(file, signatureOffset, SEEK_SET);

    // neutralize the virus
    if(result == 0){
        unsigned char retIns = 0xC3; // ret instruction
        result = fwrite(&retIns,1,1,file);
        if(result != 1) perror("An error has occurred rewriting the virus");
        fclose(file);
    }
}

void quit(){
    list_free(virus_list);
    printf("Exiting program...\n");
    exit(0);    
}

// Read and open file
FILE* getFile(){
    free(fileName);
    fileName = NULL;
    
    printf("Enter the file name: ");
    fileName = calloc(100,1);
    if(fgets(fileName,100,stdin) == NULL) {
        perror("error reading input"); 
        exit(0);
    }
    fileName[strcspn(fileName, "\n")] = '\0'; 
    FILE* file = fopen(fileName, "rb");
    if (file == NULL) {
        perror("Error opening file");
        return NULL;
    }
    printf("\n");
    return file;
}

// The detect_virus function compares the content of the buffer byte-by-byte with the virus signatures stored in the virus_list linked list. 
void detect_virus(char *buffer, unsigned int size, link *virus_list){
    link* curr = virus_list;
    virus* v = NULL;
    // for every virus go through the all file byte-by-byte
    while(curr){
        v = curr->vir;
        for(int i = 0; i <= (size - v->SigSize); i++){
            if(memcmp(&buffer[i],v->sig,v->SigSize) == 0){
                printf("Starting byte location: %d\n", i);
                printf("Virus name: %s\n", v->virusName);
                printf("Virus sig_size: %d\n", v->SigSize);
            }
        }
        curr = curr->nextVirus;
    }
}