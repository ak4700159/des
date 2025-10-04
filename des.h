#ifndef __DES_H__
#define __DES_H__
#include <stdlib.h> 
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <stdbool.h>
#include <memory.h>
#include "constant.h"
#include "buffer.h"

// ============ DES Functions ============
void convert_byte_to_word(uint8_t*, uint32_t*, uint32_t*) ;
void convert_word_to_byte(uint32_t, uint32_t, uint8_t*);
void swap(uint32_t *x, uint32_t *y);
void expansion(uint32_t, uint8_t*);
void execute_des(uint8_t*, uint8_t*, bool);
void permute(uint8_t*, bool);
uint32_t f(uint32_t, uint8_t*);
uint32_t permute_f(uint32_t);
uint32_t transfer_sbox(uint8_t*);
// ============ Key Generation Functions ============
uint32_t shift(uint32_t, int);
uint8_t make_parity_byte(uint8_t);
uint8_t** generate_round_keys(uint8_t*);
uint8_t* generate_key();
void compress(uint8_t*);
void permute_choice1(uint8_t*, uint8_t*);
void permute_choice2(uint32_t, uint32_t, uint8_t*); 
void spreate_28bit(uint32_t*, uint32_t*, uint8_t*);


// DES 알고리즘 실행
//  input   : 버퍼 속 데이터
//  key     : 사용자로부터 입력받은 대칭키
//  encode  : 인코딩 모드 여부, false라면 디코딩 
void execute_des(uint8_t* input, uint8_t* key, bool encode) {
    uint32_t left = 0, right = 0;
    // 64비트 키값을 바탕으로 라운드마다 사용할 키를 먼저 생성
    uint8_t** round_key = generate_round_keys(key);
    // 디코딩 시 키를 역순으로 정렬, 페이스텔 구조 때문에 가능
    if(!encode) {
        uint8_t* temp;
        for(int i = 0; i < 8; i++) {
            temp = round_key[15-i];
            round_key[15-i] = round_key[i];
            round_key[i] = temp;
        }
    }
    // 초기 전치
    permute(input, true);
    // 입력된 값을 32비트씩 left, right로 분할
    convert_byte_to_word(input, &left, &right);
    // 총 16번의 라운드 잔행
    for(int r = 0; r < 16; r++) {
        // F 함수 진행 + 매 라운드마다 32비트 단위로 바뀐다.
        left = left ^ f(right, round_key[r]);
        // 라운드가 끝날 때마다 스왑, 마지막 라운드에선 스왑 X
        if (r != 15) {
            swap(&left, &right);
        }
    }
    // 왼쪽 오른쪽 32비트를 64비트 input으로 병합(엄밀히 말하면 8*8)
    convert_word_to_byte(left, right, input);
    // 마지막 전치
    permute(input, false);

    // 동적 할당 해제
    for(int i = 0; i < 16; i++) {
        free(round_key[i]);
    }
    free(round_key);
}

// input    : 버퍼 속 데이터
// init     : init permutation 테이블인지 아닌지(아니면 final)
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

// 라운드 내 f함수
// right    : 32비트 오른쪽 데이터
// round_key: 32비트 라운드키
uint32_t f(uint32_t right, uint8_t* round_key) {
	int i;
	uint8_t data[6] = { 0, };
	uint32_t out;

	expansion(right, data);
    // 48bit 라운드 키와 48bit 오른쪽 데이터 XOR
	for (i = 0;i<6;i++) {
		data[i] = data[i] ^ round_key[i];
    }
    out = transfer_sbox(data);
    return permute_f(out);
}

uint32_t permute_f(uint32_t out) {
    uint32_t temp = 0;
    for (int i = 0; i < 32; i++) {
        uint32_t pos = 32 - f_permutation_table[i]; 
        if (out & ((uint32_t)1 << pos)) {
            temp |= ((uint32_t)1 << (31 - i));
        }
    }
    return temp;
}

// right 32비트를 data 48비트로 확장
//  data    : uint8_t 6개를 가지는 배열(전부 0으로 초기화되어 있는 상태)
//  right   : 오른쪽 32비트 데이터 
void expansion(uint32_t right, uint8_t* data) {
    for (int i = 0; i < 48; i++) {
        uint32_t pos = 32 - e_permuation_table[i];
        // 해당 위치의 값이 1이라면 data에도 똑같이 1로 활성화
        if (right & ((uint32_t)1 << pos)) {
            // 0x80 = 0b 0100 0000
            data[i / 8] |= (uint8_t)(0x80 >> (i % 8));
        }
    }
}

// 48bit(8 * 6bit)를 32bit(8 * 4)로 축약.  
uint32_t transfer_sbox(uint8_t* data) {
	int i, row, column, shift = 28;
	uint32_t temp = 0, result = 0, mask = 0x80;

	for (i = 0; i < 48; i++) {
         // 마스크를 씌워 확인 후 temp에 해당 비트 1로 함당
         // 즉, temp에 담긴 6비트를 이용해 transfer 
		if (data[i / 8] & (uint8_t)(mask >> (i % 8))) {
            // 0x20 == 0b 0010 0000
            temp |= 0x20 >> (i % 6);
        }

        // temp에 6비트가 전부 담긴 경우.
		if ((i + 1) % 6 == 0) {
            //R----R, 양끝 비트를 추출해 행 인덱스로 사용
			row = ((temp & 0x20) >> 4) + (temp & 0x01);
            //-CCCC-, 중간 4비트 추출해 열 인덱스로 사용(0x1E == 0b 0001 1110)
			column = (temp & 0x1E) >> 1;
            // i/6 S_BOX의 (row, column) 자리의 값을 shift해 더해준다(사실 OR 연산을 해도 같은 결과)
			result += ((uint32_t)S_BOX[i / 6][row][column] << shift);
            // result 끝에서부터 값을 채운다.
			shift -= 4;
            // temp 초기화
			temp = 0;
		}
	}
	return result;
}

void swap(uint32_t *x, uint32_t *y) {
    uint32_t t = *x; *x = *y; *y = t;
}

// data[8] -> left(32bit), right(32bit), Byte -> Word
// 8 사이즈의 바이너리 배열을 두 개의 32bit인 unsinged int로 치환 
void convert_byte_to_word(uint8_t *data, uint32_t *left, uint32_t *right) {
    int i;
    *left = 0;
    *right = 0;

    // data[0] ~ data[3], 앞 4바이트 -> left
    for (i = 0; i < 4; i++) {
        *left = (*left << 8) | data[i];
    }

    // data[4] ~ data[7], 뒤 4바이트 -> right
    for (i = 4; i < 8; i++) {
        *right = (*right << 8) | data[i];
    }
}

// left(32bit), right(32bit) -> data[8], Word -> Byte
// 두 개의 unsinged int를 8사이즈의 바이너리 배열로 치환
void convert_word_to_byte(uint32_t left, uint32_t right, uint8_t *data) {
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

// 16개의 48비트 라운드키를 생성, 이 과정을 key expanssion이라고도 부름
uint8_t** generate_round_keys(uint8_t* key){
    uint8_t** round_keys = (uint8_t**)malloc(sizeof(uint8_t*) * 16);
    for (int i = 0; i < 16; i++) {
        round_keys[i] = (uint8_t*)calloc(6, 1);
    }
    // 첫번째 permuted choice를 지난 결과
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
        // 0 ~ 127 사이의 값. 실제로 rand 함수 사용을 권장 X
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

// 56bit -> a(28bit), b(28bit) 분리
void spreate_28bit(uint32_t* a, uint32_t* b, uint8_t* choice1_result) {
    uint8_t mask = 0x80;
    *a = 0; *b = 0; 
    // a 값 채우기, 맨 앞의 비트부터 읽으면서 진행한다.
    for (int i = 0; i < 28; i++) {
        // 비트 자리를 한자리씩 뒤로
        *a <<= 1;
        if (choice1_result[i / 8] & (mask >> (i % 8))) {
            *a |= (uint32_t)1;
        }
    }
    // b 값 채우기
    for (int i = 28; i < 56; i++) {
        *b <<= 1;
        if (choice1_result[i / 8] & (mask >> (i % 8))) {
            *b |= (uint32_t)1;
        }
    }
    // 상위 4비트 쓰레기값으로 채우기(사용 X)
    *a &= 0x0FFFFFFF; *b &= 0x0FFFFFFF;
}

// 64bit key -> 56bit로 전치, 이 과정에서 패리티 비트를 제외되고 섞임
// 위제 있는 전치 코드와 유사
void permute_choice1(uint8_t* key, uint8_t* choice1_result) {
    uint8_t mask = 0x80;
    memset(choice1_result, 0, 7);
    for (int i = 0; i < 56; i++) {
        uint8_t index = (key_permutation_table1[i] - 1) / 8; 
        uint8_t bit  = (key_permutation_table1[i] - 1) % 8; 
        if (key[index] & (mask >> bit)) {
            choice1_result[i / 8] |= (mask >> (i % 8));
        }
    }
}

// a+b(56bit) -> 48bit round key (6 bytes) 생성
void permute_choice2(uint32_t a, uint32_t b, uint8_t* round_key) {
    uint8_t mask = 0x80;
    // 56bit 버퍼
    uint8_t temp[7] = {0};

    // 56bit 버퍼에 값을 채우는 작업
    // a 맨마지막 비트 자리부터(MSB) -> tmep 0..27 순서로 채운다
    for (int i = 0; i < 28; i++) {
        uint8_t bit = ( (a >> (27 - i)) & (uint32_t)1 ) ? 1 : 0;
        if (bit) {
            temp[i / 8] |= (mask >> (i % 8));
        }
    }
    // b 맨마지막 비트 자리부터(MSB) -> temp 28..55
    for (int i = 0; i < 28; i++) {
        uint8_t bit = ( (b >> (27 - i)) & (uint32_t)1 ) ? 1 : 0;
        int pos = 28 + i;
        if (bit) {
            temp[pos / 8] |= (mask >> (pos % 8));
        }
    }

    // 위에서 채워놓은 temp를 바탕으로 round_key 구성
    memset(round_key, 0, 6);
    for (int i = 0; i < 48; i++) {
        // 위의 permute 코드와 동일한 과정
        uint8_t index = (key_permutation_table2[i] - 1) / 8;  // 0..6
        uint8_t bit  = (key_permutation_table2[i] - 1) % 8;  // 0..7
        if (temp[index] & (mask >> bit)) {
            round_key[i / 8] |= (mask >> (i % 8));
        }
    }
}

#endif