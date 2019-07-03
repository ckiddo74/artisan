#include <stdio.h>

int a () {   
    return 0;
}

int b () {
    return 0;
}


int main() {
    int x[10];
    while (b() != 0) {
        a();
    }
    if (b() != 0) {
        a();
    }
    return 0;
}