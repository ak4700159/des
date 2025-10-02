#include <stdio.h>
#include <stdlib.h>



void test_func2(int* arr) {
    arr[0] = 20;
}

void test_func(int* arr) {
    arr[0] = 10;
    test_func2(arr+2);
}

int* local_variabe() {
    int arr[5] = {1, 2, 3, 4, 5};
    return arr;
}

void print_arr(int* arr) {
    printf("arr : ");
    for(int i = 0; i < 10; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n\n");
}

int main(void) {
    // int* arr = (int*)malloc(sizeof(int) * 10);
    // for(int i = 0; i < 10; i++) {
    //     arr[i] = i;
    // }
    // print_arr(arr);
    // test_func(arr);
    // print_arr(arr);
    print_arr(local_variabe());
    return 0;
}



