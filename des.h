#ifndef __DES_H__
#define __DES_H__
#include <stdint.h>
#include <time.h>
#include <stdbool.h>
#include <memory.h>
#include "constant.h"
#include "buffer.h"

// ============ DES Functions ============
void encode(uint8_t*, uint8_t*);
void BtoW(uint32_t, uint32_t, uint8_t);
void WtoB(uint32_t, uint32_t, uint8_t);
void permute(uint8_t*, bool);
uint32_t permute_f(uint32_t);
uint32_t f(uint32_t, uint8_t*);
uint32_t transfer_sbox(uint8_t*);
void expansion(uint32_t, uint8_t*);
// ============ Key Generation Functions ============
uint8_t make_parity_byte(uint8_t);
uint8_t** generate_round_keys(uint8_t*);
uint8_t* generate_key();
uint32_t shift(uint32_t, int);
void compress(uint8_t*);
void permute_choice1(uint8_t*, uint8_t*);
void permute_choice2(uint32_t, uint32_t, uint8_t*); 
void spreate_28bit(uint32_t*, uint32_t*, uint8_t*);

// input : 버퍼 속 데이터, init : init permutation 테이블인지 아닌지(아니면 final)
void permute(uint8_t* input, bool init) {
    // 1000 0000 값을 가지는 1바이트로 초기화
    uint8_t index, bit, mask = 0x80;
    // 8바이트 임시 결과 저장용 생성
    uint8_t temp[8] = { 0, };
    // table 전체를 순회하며 값을 바꾼다.
    for (int i = 0; i < 64; i++) {
        // table 속 인덱스에 대한 좌표를 구한다.
        //  - 몇 번째 행인지(1행당 한 바이트씩 포함), index
        //  - 행 안에서 몇 번째 열인지, bit
        if(init) {
            index = (initial_permutation_table[i] - 1) / 8;
            bit = (initial_permutation_table[i] - 1) % 8;
        } else {
            index = (final_permutation_table[i] - 1) / 8;
            bit = (final_permutation_table[i] - 1) % 8;
        } 
        // 마스킹된 값(bit번 오른쪽 쉬프트한 결과 ex) bit=3, 0001 0000)과 
        // input 64비트 중 해당 인덱스를 가지는 8바이트와 AND연산
        // table 속 인덱스의 값에 위치한 input의 값이 1이라면 실행
        if (input[index] & (mask >> bit)) {
            // 1이라면 출력도 똑같이 표현
            temp[i / 8] |= mask >> (i % 8);
        }
    }
    // 변경된 값 input에 반영
    memcpy(input, temp, 8);
}

void encode(uint8_t* input, uint8_t* key) {
    uint32_t left, right = 0;
    // 64비트 키값을 바탕으로 라운드마다 사용할 키를 먼저 생성
    uint8_t** round_key = generate_round_keys(key);
    // 초기 전치
    permute(input, true);
    // 입력된 값을 32비트씩 left, right로 분할
    BtoW(input, &left, &right);
    // 총 16번의 라운드 잔행
    for(int r = 0; r < 16; r++) {
        // F 함수 진행 + 매 라운드마다 32비트 단위로 바뀐다.
        left = left ^ f(right, round_key[r]);
        // 라운드가 끝날 때마다 스왑.
        swap(&left, &right);
    }
    // 왼쪽 오른쪽 32비트를 64비트 input으로 병합(엄밀히 말하면 8*8)
    WtoB(left, right, input);
    // 마지막 전치
    permute(input, false);

    // 동적 할당 해제
    for(int i = 0; i < 16; i++) {
        free(round_key[i]);
    }
    free(round_key);
}

uint32_t f(uint32_t right, uint8_t* round_key) {
	int i;
	uint8_t data[6] = { 0, };
	uint32_t out;

	expansion(right, data);
    // 8bit 씩 총 6번 라운드 키와 xor 연산
	for (i = 0;i<6;i++) {
		data[i] = data[i] ^ round_key[i];
    }
    out = transfer_sbox(data);
    return permute_f(out);
}

uint32_t permute_f(uint32_t out) {
    uint32_t mask = 0x8000;
    uint32_t temp = 0;
    for (int i = 0; i < 32; i++) {
        if (out & (mask >> (f_permutation_table[i]-1))) {
            temp |= mask >> (f_permutation_table[i]-1);
        }
    }
    return temp;
}

// 32비트를 48비트로 확장
void expansion(uint32_t right, uint8_t* data) {
	int i;
	uint32_t mask = 0x8000;

	for (i = 0; i < 48; i++) {
		if (right & (mask >> (e_permuation_table[i] - 1))) {
			data[i / 8] |= (uint8_t)(0x80 >> (i % 8));
		}
	}
}

uint32_t transfer_sbox(uint8_t* data) {
	int i, row, column, shift = 28;
	uint32_t temp = 0, result = 0, mask = 0x80;

	for (i = 0;i<48;i++) {
         // 마스크를 씌워 확인 후 temp에 해당 비트 1로 함
		if (data[i / 8] & (uint8_t)(mask >> (i % 8))) {
            temp |= 0x20 >> (i % 6);
        }

		if ((i + 1) % 6 == 0) {
			row = ((temp & 0x20) >> 4) + (temp & 0x01);           // 행 값
			column = (temp & 0x1E) >> 1;                               // 열 값
			result += ((uint32_t)S_BOX[i / 6][row][column] << shift);    // 값 더하고 쉬프트(4비트씩)
			shift -= 4;
			temp = 0;
		}
	}
	return result;
}

// data[8] -> left(32bit), right(32bit), Byte -> Word
// 8 사이즈의 바이너리 배열을 두 개의 32bit인 unsinged int로 치환 
void BtoW(uint32_t *data, uint32_t *left, uint8_t *right) {
    int i;

    *left = 0;
    *right = 0;

    // data[0] ~ data[3], 앞 4바이트 → left
    for (i = 0; i < 4; i++) {
        *left = (*left << 8) | data[i];
    }

    // data[4] ~ data[7], 뒤 4바이트 → right
    for (i = 4; i < 8; i++) {
        *right = (*right << 8) | data[i];
    }
}

// left(32bit), right(32bit) -> data[8], Word -> Byte
// 두 개의 unsinged int를 8사이즈의 바이너리 배열로 치환
void WtoB(uint32_t left, uint32_t right, uint8_t *data) {
    int i;

    // left → 앞 4바이트
    for (i = 3; i >= 0; i--) {
        // LEFT & 1111 1111
        // 형변환으로 인해 하위 8비트를 제외한 상위 비트는 사라진다.
        data[i] = (uint8_t)(left & 0xFF);
        // 다음 인덱스에 들어갈 데이터를 위해 시프팅
        left >>= 8;
    }

    // right → 뒤 4바이트
    for (i = 7; i >= 4; i--) {
        data[i] = (uint8_t)(right & 0xFF);
        right >>= 8;
    }
}

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

// 16개의 48비트 라운드키를 생성 
uint8_t** generate_round_keys(uint8_t* key){
    uint8_t** round_keys = (uint8_t**)malloc(sizeof(uint8_t*) * 16);
    for (int i = 0; i < 16; i++) {
        round_keys[i] = (uint8_t*)calloc(6, 1);
    }
    uint8_t choice1_result[7] = { 0, };
    // a와 b는 28비트씩 저장
    uint32_t a, b = 0;

    permute_choice1(key, choice1_result);
    spreate_28bit(&a, &b, choice1_result);
    for(int i = 0; i < 16; i++) {
        a = shift(a, i);
        b = shift(b, i);
        permute_choice2(a, b, round_keys[i]);
    }
    return round_keys;
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
}

// 좌측 순환 쉬프트
uint32_t shift(uint32_t key, int i) {
    // 1, 2, 9, 16 에서만 shift 두 번 
	int n_shift[16] = { 1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1 };
	key = (key << n_shift[i]) + (key >> (28 - n_shift[i]));
    // 상위 4비트를 제거하기 위해 AND 연산(28비트만)
	return key & 0xFFFFFFF;
}

// PC-1: 64bit key -> 56bit (parity drop)
void permute_choice1(uint8_t* key, uint8_t* choice1_result) {
    // key: 8 bytes, choice1_result: 7 bytes(out)
    const uint8_t MSB = 0x80;
    memset(choice1_result, 0, 7);

    for (int i = 0; i < 56; i++) {
        uint8_t src_byte = (key_permutation_table1[i] - 1) / 8;  // 0..7
        uint8_t src_bit  = (key_permutation_table1[i] - 1) % 8;  // 0..7 (MSB first)
        if (key[src_byte] & (MSB >> src_bit)) {
            choice1_result[i / 8] |= (MSB >> (i % 8));
        }
    }
}

// 56bit -> C(28bit), D(28bit)   (MSB-first 시프트 빌드)
void spreate_28bit(uint32_t* a, uint32_t* b, uint8_t* choice1_result) {
    const uint8_t MSB = 0x80;
    *a = 0;
    *b = 0;

    // build C (bits 0..27)
    for (int i = 0; i < 28; i++) {
        *a <<= 1;
        if (choice1_result[i / 8] & (MSB >> (i % 8))) {
            *a |= 1u;
        }
    }

    // build D (bits 28..55)
    for (int i = 28; i < 56; i++) {
        *b <<= 1;
        if (choice1_result[i / 8] & (MSB >> (i % 8))) {
            *b |= 1u;
        }
    }
    // 상위 4비트 마스킹(안전)
    *a &= 0x0FFFFFFF;
    *b &= 0x0FFFFFFF;
}

// PC-2: C||D(56bit) -> 48bit round key (6 bytes)
void permute_choice2(uint32_t a, uint32_t b, uint8_t* round_key) {
    // round_key: 6 bytes(out)
    const uint8_t MSB = 0x80;

    // 56bit 버퍼(C||D) 구성
    uint8_t cd[7] = {0};

    // put C (a) : bit 27..0 -> positions 0..27 (MSB-first)
    for (int i = 0; i < 28; i++) {
        uint8_t bit = ( (a >> (27 - i)) & 1u ) ? 1 : 0;
        if (bit) cd[i / 8] |= (MSB >> (i % 8));
    }
    // put D (b) : bit 27..0 -> positions 28..55
    for (int i = 0; i < 28; i++) {
        uint8_t bit = ( (b >> (27 - i)) & 1u ) ? 1 : 0;
        int pos = 28 + i;
        if (bit) cd[pos / 8] |= (MSB >> (pos % 8));
    }

    // PC-2 적용
    memset(round_key, 0, 6);
    for (int i = 0; i < 48; i++) {
        uint8_t src_byte = (key_permutation_table2[i] - 1) / 8;  // 0..6
        uint8_t src_bit  = (key_permutation_table2[i] - 1) % 8;  // 0..7
        if (cd[src_byte] & (MSB >> src_bit)) {
            round_key[i / 8] |= (MSB >> (i % 8));
        }
    }
}

#endif