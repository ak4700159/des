#ifndef __BUFFER_H__
#define __BUFFER_H__
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

// 파일로부터 읽은 데이터를 남은 구조체
typedef struct Buffer{
    int real_size;
	int size;
	int capacity;
	uint8_t* data;
} Buffer;

void init_buffer(Buffer* buffer) {
    buffer->size = 0;
    buffer->real_size = 0;
    buffer->capacity = 1;
    buffer->data = (uint8_t*)malloc(sizeof(uint8_t));
}

void push_buffer(Buffer* buffer, uint8_t data) {
    if(buffer->size == buffer->capacity) {
        buffer->capacity *= 8;
        buffer->data = (uint8_t*)realloc(buffer->data, sizeof(uint8_t) * buffer->capacity);
        if(buffer->data == NULL) {
            printf("realloc buffer data error");
            exit(1);
        }
    }
    buffer->data[buffer->size++] = data;
}   

void print_buffer(Buffer* buffer) {
    for(int i = 0; i < buffer->size; i++) {
        printf("%c", buffer->data[i]);
    }
    printf("\n\n");
}

void close_buffer(Buffer* buffer) {
    free(buffer->data);
    free(buffer);
}

#endif