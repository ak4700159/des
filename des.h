#ifndef __DES_H__
#define __DES_H__
#include <stdint.h>
#include <time.h>
#include "constant.h"
#include "buffer.h"

/*
DES 알고리즘 구현
    0. 키생성
    1. IP
    2. 총 16번의 라운드 진행
    3. FP
*/

// input : 버퍼 속 데이터
void permute_init(uint8_t* input) {
    int i;
    // 1000 0000 값을 가지는 1바이트로 초기화
    uint8_t index, bit, mask = 0x80;
    // 8바이트 임시 결과 저장용 생성
    uint8_t* temp = (uint8_t*)malloc(sizeof(uint8_t) * 8);
    memset(temp, 0, sizeof(uint8_t) * 8);
    // table 전체를 순회하며 값을 바꾼다.
    for (i = 0; i < 64; i++) {
        // table 속 인덱스에 대한 좌표를 구한다.
        // 몇 번째 행인지(1행당 한 바이트씩 포함)
        index = (initial_permutation_table[i] - 1) / 8;
        // 행 안에서 몇 번째 열인지
        bit = (initial_permutation_table[i] - 1) % 8;
        // 마스킹된 값(bit번 오른쪽 쉬프트한 결과 ex) bit=3, 0001 0000)과 
        // input 64비트 중 해당 인덱스를 가지는 8바이트와 AND연산
        // table 속 인덱스의 값이 1인지 0인지 확인
        if (input[index] & (mask >> bit)) {
            // 1이라면 출력도 똑같이 표현
            temp[i / 8] |= mask >> (i % 8);
        }
    }
    // 변경된 값 input에 반영
    memcpy(input, temp, 8);
    free(temp);
}

void permute_final(uint8_t* input) {

}


void encode_by_des(uint8_t* input, uint8_t* key) {
    permute_init(input);

    // 총 16번의 라운드
    for(int r = 0; r < 16; r++) {

    }

    permute_final(input);
}

uint8_t* decode_by_des(uint8_t* input) {
    uint8_t result = 0x00;
}



void des_round() {};

void xor_key() {};

void expansion() {};

void straight() {};

// 홀수 패리티 비트 생성
// data = (0XXX XXXX), 0 ~ 127 사이의 값을 가짐
uint8_t make_parity_byte(uint8_t data) {
    uint8_t count = 0;
    for (int i = 0; i < 7; i++) {
        // 0000 0001 부터 시작하여 왼쪽으로 하나씩 쉬프트 하면서
        // data7과 AND 연산을 실행 = 1이 위치한 비트와 AND 연산
        // 해당 위치값이 1이면 count는 ++, 0이면 동작 X 
        if (data & (1 << i)) count++;
    }

    // 홀수 패리티 비트 생성
    uint8_t parity = (count % 2 == 0) ? 1 : 0;
    // LSB 비트에 패리티 비트 추가
    return data << 1 | parity; 
}


// 키 생성
uint8_t* generate_key() {
    uint8_t* key = (uint8_t*)malloc(sizeof(uint8_t) * 8);
    srand((unsigned)time(NULL));
    for(int i = 0; i < 8; i++) {
        // 0 ~ 127 사이의 값
        uint8_t random = (uint8_t)(rand() % 128);
        key[i] = make_parity_byte(random);
        printf(" %d", random);
    }
    printf("\n");
    return key;
};


// 패리티 비트 제거
void drop_parity_bit() {

};

// 1, 2, 9, 16 에서만 shift 두 번 
void shift(int round) {

};


void compress_key()		{};


#endif
