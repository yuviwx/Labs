#include <stdio.h>
#include <string.h>

int count_digits(char* str) {
   int count = 0;
   while(*str) {
       if(*str >= '0' && *str <= '9') count++;
       str++;
   }
   return count;
}

int main(int argc, char ** argv) {
    if(argc != 2){
        perror("error: invalid number of arguments");
        return 1;
    }
    printf("String contains %d digits\n", count_digits(argv[1]));
    return 0;
}

/*
ntsc file:
    count_digit:
        virtual address: 0804847d
        size: 93
        secion number: 13
    
    section 13:
        virtual address: 08048380
        offset: 000380

    offset_count_digit_ntsc = 000380 + (0804847d - 08048380) = 47d
    size_count_digit_ntsc = 93

task4 file:
    count_digit:
        virtual address: 000011ad
        size: 58
        section number: 14
    
    section 14:
        virtual address: 00001080
        offset: 001080
    
    offset_count_digit_task4 = 001090 + (000011bd - 00001090) = 11AD
    size_count_digit_task4 = 58
*/