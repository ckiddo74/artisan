#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
 
void run_cpu(int N){
   for (int q = 0; q < N; q++) {
       printf("Loop b\n");
       for (int j = 0; j < N; j++) {
           printf("Loop b_c\n");
           sleep(1);
       }
   }
   for (int i = 0; i < N; i++) {
       printf("Loop d\n");
       sleep(1);
   }
}
 
int main(int argc, char **argv) {
   int N = 2;
   for (int i = 0; i < N; i++){
       printf("Loop a\n");
       sleep(1);
   }
   run_cpu(N);
}


