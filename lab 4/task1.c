#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  char debug_mode;
  char file_name[128];
  int unit_size;
  unsigned char mem_buf[10000];
  size_t mem_count;
  char display_mode;
  /*
   .
   .
   Any additional fields you deem necessary
  */
} state;

typedef struct {
    char* name;
    void (*func)(state*);
} fun_desc;

// Declarations
void Toggle_Debug_Mode(state* s);
void Set_File_Name(state* s);
void Set_Unit_Size(state* s);
void Load_Into_Memory(state* s);
void Toggle_Display_Mode(state* s);
void File_Display(state* s);
void Memory_Display(state* s);
void Save_Into_File(state* s);
void Memory_Modify(state* s);
void Quit(state* s);

//my functions
state* constructor();
FILE* my_fopen(state* s, char* mode);
char get2params(unsigned int* location, unsigned int* length, char* formats);
void display(state* s, FILE* file, unsigned int location, unsigned int length);
char* unit_to_format2(int unit_size, char hex);
void print_units(state* s, char* buffer, int length);
char get3params(unsigned int* addr, unsigned int* offset, unsigned int* length, char* formats);

int main(int argc, char ** argv){
    //char* carray = calloc(5, sizeof(char)); // 1. Define a pointer 'carray' to a char array of length 5, initialized to the empty string.
    char* buffer = calloc(3, sizeof(char)); // A buffer for the input number
    fun_desc menu[] = {{"Toggle Debug Mode", Toggle_Debug_Mode}, {"Set File Name", Set_File_Name}, {"Set Unit Size", Set_Unit_Size}, {"Load Into Memory", Load_Into_Memory}, {"Toggle Display Mode", Toggle_Display_Mode}, {"File Display", File_Display}, {"Memory Display", Memory_Display}, {"Save Into File", Save_Into_File}, {"Memory Modify", Memory_Modify}, {"Quit", Quit}, {NULL, NULL}};
    int input;
    state* s = constructor();

    while(1){
        printf("Choose action:\n");
        // 3. Display a menu
        for(int i = 0; menu[i].name != NULL; i++){
        printf("%d-%s\n", i, menu[i].name);
        }
        // 4. Choose from the menu
        if(fgets(buffer,3,stdin) == NULL) exit(0);
        input = atoi(buffer);
        // check bounds
        if(input < 0 || input > 9){
        printf("Not within bounds\n");
        free(buffer);
        free(s);
        exit(0);
        }
        // 5.Evaluate the appropriate function
        printf("\n");
        menu[input].func(s);
        printf("\n");
        //char* temp = map(carray, 5, menu[input].func);
    }
    return 0;
}

state* constructor(){
    state* s = (state*)malloc(sizeof(state));
    s->debug_mode = 0;
    strcpy(s->file_name, "abc"); // Assume that the user has already set the file name to "abc".
    s->unit_size = 1;
    s->mem_count = 0;
    s->display_mode = 0; // decimal representation
    return s;
}

void Toggle_Debug_Mode(state* s){
    s->debug_mode = s->debug_mode == 0 ? 1 : 0;
    printf("Debug flag now %s\n", s->debug_mode ? "on" : "off");
}

/*
// @brief: sets the file name
// @param: state* s - the state of the program
*/
void Set_File_Name(state* s){
    //file name doesn't need to be cleared because fgets will add null terminator after the input to determine the end of the string.
    printf("Please enter a file name:\n");
    fgets(s->file_name, 100, stdin);
    s->file_name[strlen(s->file_name)-1] = '\0'; // instead of the new line
    if(s->debug_mode == 1) printf("Debug: file name set to %s", s->file_name);
} 

void Set_Unit_Size(state* s){
    char buffer[10];
    printf("Please choose a unit size: (the options are: 1,2,4)\n");
    fgets(buffer,10,stdin);
    int input = atoi(buffer);
    if(input != 1 && input != 2 && input != 4){
        perror("unvalid input, unit size can be 1, 2 or 4.");
    }
    else{
        s->unit_size = input;
        if(s->debug_mode) printf("Debug: set size to %d\n", s->unit_size);
    }

}

void Quit(state* s){
    if(s->debug_mode) printf("quitting");
    free(s);
    exit(0);
}

void Load_Into_Memory(state* s){
    FILE* file = my_fopen(s, "rb");
    if(file == NULL) return;

    printf("please enter location(in hexadecimal) and length(in decimal)\n");
    unsigned int location;
    unsigned int length;
    if(get2params(&location, &length, "%x %d") == 0){
        fclose(file);
        return;
    }

    if(s->debug_mode) printf("file name: %s\n location: %x\n length: %d\n", s->file_name, location, length);
    if(fseek(file, location, SEEK_SET) != 0){
        perror("error seeking location");
        fclose(file);
        return;
    }
    if(fread(s->mem_buf, s->unit_size, length, file) != length){
        perror("error reading file");
        fclose(file);
        return;
    }
    s->mem_count = length * s->unit_size; // in bytes
    printf("Loaded %d units into memory successfully!\n", length);
    fclose(file);

}

void Toggle_Display_Mode(state* s){
    s->display_mode = 1 - s->display_mode;
    printf("Display flag now %s representation\n", s->display_mode ? "on, hexadecimal" : "off, decimal");
}

/*
* @brief: displays u units of size unit_size starting at a given offset in the file
*/
void File_Display(state* s){
    FILE* file = my_fopen(s, "rb");
    if(file == NULL) return;
    printf("please enter offset and length(both in decimal)\n");
    unsigned int offset; // location
    unsigned int u; // length
    if(get2params(&offset, &u, "%d %d") == 0){
        fclose(file);
        return;
    }
    if(s->debug_mode) printf("offset: %x\nlength: %d\n", offset, u);
    display(s, file, offset, u);
    fclose(file);
}

/*
* @brief Displays u units of size unit_size starting at a given address in memory
*/
void Memory_Display(state* s){
    printf("please enter address(in hexadecimal) and length(in decimal)\n");
    unsigned int addr; // location
    unsigned int u; // length
    if(get2params(&addr, &u, "%x %d") == 0){
        return;
    }
    if(s->debug_mode) printf("address: %x\nlength: %d\n", addr, u);
    
    // check bounds
    if(addr + u * s->unit_size > s->mem_count){
        perror("error: trying to read more units than the memory contains");
        return;
    }
    print_units(s, (char*)(s->mem_buf + addr), u);    
}
/*
* @brief: replaces <length> units from the memory buffer into a file
* @param: state* s - the state of the program - contains the memory buffer and the file name
*/
void Save_Into_File(state* s){
    // open the file
    FILE* file = my_fopen(s, "r+b");
    if(file == NULL) return;

    // get the location and length in mem_buf
    printf("please enter source_address(hex) location(hex) and length(dec)\n");
    unsigned int source_address;  // virual memory address if != 0, mem_buf otherwise
    unsigned int target_location; // offset in file
    unsigned int length;          // number of units
    if(get3params(&source_address, &target_location, &length, "%x %x %d") == 0){
        return;
    }
    // debug message
    if(s->debug_mode) printf("source_address: %x\ntarget_location: %x\nlength: %d", source_address, target_location, length);

    // Check bounds:
    // Check if the source has enough units
    if(length * s->unit_size > s->mem_count){
        perror("error: trying to read more units than the memory contains");
        return;
    }
    // set the source address
    char* source_buffer = (source_address == 0) ? (char*)s->mem_buf : (char*)source_address;
    // Check if the target location is within the file size
    size_t total_bytes = length * s->unit_size;
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    if (target_location + total_bytes > file_size) {
        fprintf(stderr, "Error: Target location exceeds file size.\n");
        fclose(file);
        return;
    }

    fseek(file, target_location, SEEK_SET);
    // Write data to the file
    size_t bytes_written = fwrite(source_buffer, s->unit_size, length, file);
    if (bytes_written != length) {
        fprintf(stderr, "Error: Only %zu units written out of %zu requested.\n", bytes_written, length);
        fclose(file);
        return;
    }
    fclose(file);
}
/*
* @brief: replaces a unit in the memory buffer at a given location with val.
* @param: state* s - the state of the program - contains the memory buffer
*/
void Memory_Modify(state* s) {
     printf("please enter location(hex) and val(hex)\n");
    unsigned int location;
    unsigned int val;
    if(get2params(&location, &val, "%x %x") == 0){
        return;
    }

    if(s->debug_mode) {
        fprintf(stderr, "Debug: location: %x, val: %x\n", location, val);
    }
    printf("location: %x\nlocation: %d\n", location, location);
    if(location + s->unit_size > 10000) {
        printf("Error: Invalid location\n");
        return;
    }

    memcpy(&s->mem_buf[location], &val, s->unit_size);
}

/* 
* @brief This function will open the file saved in 's' in rb mode and return the file pointer.
* @param s The state with the file name to open.
* @return FILE* if the file is found, NULL otherwise.
*/
FILE* my_fopen(state* s, char* mode){
    if(s->file_name[0] == '\0'){
        perror("file name is empty,\n please set a file name by choosing option 1");
        return NULL;
    }
    FILE* file = fopen(s->file_name, mode);
    if(file == NULL){
        perror("file not found");
        return NULL;
    }
    return file;
}

/*
* @brief This function reads 2 parameters from the user, location and length.
* @param location: A pointer to store the location.
* @param length: A pointer to store the length.
* @return char: 1 if the input was read successfully, 0 otherwise.
*/
char get2params(unsigned int* location, unsigned int* length, char* format){
    char buffer[100];
    int result;

    if(fgets(buffer, 100, stdin) == NULL){
        perror("error reading input,\n please provide valid location and length, separated by space");
        return 0;
    }
    result = sscanf(buffer, format, location, length);

    if(result == EOF || result < 2){
        printf("result: %d\n", result);
        perror("error interpreting the input");
        printf("location & length: ");
        printf(format, *location, *length);
        return 0;
    }
    return 1;
}

/*
* @brief displays u units of size unit_size starting at a given offset in the file
* @param state* s - the state of the program
*/
void display(state* s, FILE* file, unsigned int location, unsigned int length){
    char* buffer = calloc(1, length);
    char* save = buffer;
    
    fseek(file, location, SEEK_SET);
    if(fread(buffer, s->unit_size, length, file) != length){
        perror("error reading file");
        free(buffer);
        return;
    }
    print_units(s, buffer, length);    
    free(save);
}

/*
* @brief: maps the unit size to the corresponding format string for printf
* @param: int unit_size - the size of the unit
* @param: char hex - 1 if the display mode is hexadecimal, 0 otherwise
* @return: char* - the format string
*/
char* unit_to_format2(int unit_size, char hex){
    static char* hex_formats[] = {"%#hhx\n", "%#hx\n", "No such unit", "%#x\n"};
    static char* dec_formats[] = {"%#hhd\n", "%#hd\n", "No such unit", "%#d\n"};
    return hex ? hex_formats[unit_size-1] : dec_formats[unit_size-1];
}

void print_units(state* s, char* buffer, int length){
    printf("%s\n", s->display_mode ? "Hexadecimal" : "Decimal");
    printf("%s\n", s->display_mode ? "===========" : "=======");

    char* end = buffer + (s->unit_size * length); // point to the end of the fread because it won't end with a null terminator    
    while (buffer < end) {
        //print ints
        int var = *((int*)(buffer)); 
        fprintf(stdout, unit_to_format2(s->unit_size, s->display_mode), var);
        buffer += s->unit_size;
    }
}

char get3params(unsigned int* addr, unsigned int* offset, unsigned int* length, char* format){
    char buffer[100];
    int result;

    if(fgets(buffer, 100, stdin) == NULL){
        perror("error reading input,\n please provide valid location and length, separated by space");
        return 0;
    }
    result = sscanf(buffer, format, addr, offset, length);

    if(result == EOF || result < 3){
        printf("result: %d\n", result);
        perror("error interpreting the input");
        printf("location & offset & length: ");
        printf(format, *addr, *offset, *length);
        return 0;
    }
    return 1;
}

/*
task 3:
main:
virtual address: 0804841d
size: 23
section number: 13

section:
offset: 000320
virtual address: 08048320

offset of main = sec_offset + (main_virtual_address - sec_virtual_address)
= 000320 + (0804841d - 08048320) = 00041d
*/