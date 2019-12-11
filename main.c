#include <stdio.h>
#include <stdlib.h>
#include "options.h"

int main(int argc, char *argv[]) {
    char *file = argv[1];
    int section = atoi(argv[2]);

    if (section != 1 && section != 2 && section != 3 && section != 4) {
        printf("Invalid section number!");
        exit(0);
    }

    if(section == 1) {
        option1(file);
    } else if(section == 2) {
        option2(file);
    } else if(section == 3) {
        option3(file);
    } else if(section == 4) {
        option4(file);
    }

    

    return 0;
}
