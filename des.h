#ifndef __DES_H__
#define __DES_H__
#include <stdint.h>
#include "constant.h"
#include "buffer.h"


/*
DES 알고리즘 구현
    0. 키생성
    1. IP
    2. 총 16번의 라운드 진행
    3. FP
*/


uint8_t encode_by_des(uint8_t input) {
    input = permute_init(input);

    // 총 16번의 라운드
    for(int r = 0; r < 16; r++) {

    }

    return permute_final(input);
}

uint8_t decode_by_des(uint8_t input) {
    uint8_t result = 0x00;
}

uint8_t permute_init(uint8_t input) {
    int i;
    // 1000 0000 값을 가지는 1바이트로 초기화
    uint8_t index, bit, mask = 0x80;
    for (i = 0; i < 64; i++) {
        index = (initial_permutation_table[i] - 1) / 8;
        bit   = (initial_permutation_table[i] - 1) % 8;

        if (initial_permutation_table[index] & (mask >> bit))
            out[i / 8] |= mask >> (i % 8);
    }
}

uint8_t permute_final(uint8_t input) {

}

void des_round() {};

void xor_key() {};

void expansion() {};

void straight() {};

// KEY GENERATION FUNCTIONs
void generate_key() {};

uint8_t make_parity_byte(uint8_t data7) {
    data7 &= 0x7F; 
    uint8_t count = 0;
    for (int i = 0; i < 7; i++) {
        if (data7 & (1 << i)) count++;
    }
    uint8_t parity = (count % 2 == 0) ? 1 : 0;
    return (data7 << 1) | parity; 
}

void drop_parity_bit() {
    // 패리티 비트 제거
};

// 1, 2, 9, 16 에서만 shift 두 번 
void shift(int round) {};


void compress_key()		{};


#endif
