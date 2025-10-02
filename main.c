/*
1.  DES 암호 알고리즘을 완성하시오.

2. 완성한 알고리즘을 활용하며 문서 암호 프로그램을 만드시오. (ECB mode)
	2-1. text 파일을 읽어와서 암호문 파일 만들기 (plaintext -> ciphertext)
	2-2. 암호문 파일을 읽어와서 복호화 파일 만들기 (ciphertext -> plaintext2)
	2-3. 복호화한 파일과 최초의 text 파일이 일치하는지 확인하기 (plaintext2 == plaintext)

	* 사용하는 데이터타입 : uint8_t 사용 -> 1바이트 크기의 양의 정수 = unsigned char형과 동일
		기본적으로 아스키코드로 인코딩된 파일 속 문자 하나는 0과 255 사이 값을 가진다.
		그래서 우리는 char형(-128과 127 사이값)을 사용하는 것이 아니라 unsigned char형을 사용해 안정적인 형변환을 시도한다.

	* 암호 프로그램 설명
		1번(encrypt plaintext) 입력 시. 사용자로부터 입력 받은 평문 파일의 절대경로를 참조해 암호 파일 생성
		2번(decrypt ciphertext) 입력 시. 사용자로부터 입력 받은 암호 파일의 절대경로를 참조해 복호 파일 생성
		3번(compare two file) 입력 시. 두 개의 절대 경로 파일을 읽어들여 동일한 파일인지 확인
		4번(auto) 입력 시. 사용자로부터 평문 파일의 절대 경로만 입력 받아 위 세 가지 기능에 대한 파이프라인 실행
		모든 예외 상황에 대해 별도 조치 없이 프로그램이 종료되는 방향으로 진행 

3. 알고리즘 추가 참고 자료 : https://434official.tistory.com/1

4. 구현 참고 자료 : 
		- https://mypage-dream.tistory.com/11
		- https://nobilitycat.tistory.com/entry/c-DES-%EA%B5%AC%EC%A1%B0-%EB%B0%8F-%EC%BD%94%EB%93%9C 
		- ChatGPT
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <memory.h>
#include "buffer.h"
#include "des.h"

#define MAX_BUFFER_SIZE 100

enum SELECT {
	ENCRYPT = 1,
	DECRYPT,
	COMPARE,
	PIPELINE,
	EXIT
};

void program(); 
void menu();
void encrypt_plaintext_by_ECB(const char*, const char*);
void decrypt_ciphertext_by_ECB(const char*, const char*);
void compare(const char*, const char*);
void execute_des_pipeline(const char*, const char*);

Buffer* set_buffer(const char*);
void save_file(const char*, Buffer*);
void pinrt_hex_from_uint8(uint8_t*);
uint8_t* read_key();

int main(int argc, char* argv[]) {
	if(argc != 1) {
		printf("Wrong input format");
		exit(0);
	}
	program();
	return 0;
}

// 프로그램 시작 함수
void program() {
	char file_path1[MAX_BUFFER_SIZE];
	char file_path2[MAX_BUFFER_SIZE];
	while(1) {
		enum SELECT num = 5;
		menu(); scanf("%d", &num);
		if(num == EXIT) {
			printf("Exit program. bye bye");
			exit(1);
		}

		switch (num) {
			case ENCRYPT:
				printf("Insert input file path : ");
				scanf("%s", file_path1); getchar();
				printf("Insert output file path : ");
				scanf("%s", file_path2); getchar();
				encrypt_plaintext_by_ECB(file_path1, file_path2); break;

			case DECRYPT:
				printf("Insert input file path : ");
				scanf("%s", file_path1); getchar();
				printf("Insert output file path : ");
				scanf("%s", file_path2); getchar();
				decrypt_ciphertext_by_ECB(file_path1, file_path2); break;

			case COMPARE:
				printf("Input encrypt file path : ");
				scanf("%s", file_path1); getchar();
				printf("Input decrypt file path : ");
				scanf("%s", file_path2); getchar();
				compare(file_path1, file_path2); break;

			case PIPELINE:
				printf("Input encrypt file path : ");
				scanf("%s", file_path1); getchar();
				printf("Input decrypt file path : ");
				scanf("%s", file_path2); getchar();
				execute_des_pipeline(file_path1, file_path2); break;
				
			default:
				printf("Wrong number, please select 1~5\n"); break;
		}
		memset(file_path1, 0, sizeof(char) * MAX_BUFFER_SIZE);
		memset(file_path2, 0, sizeof(char) * MAX_BUFFER_SIZE);
		printf("\n\n");
	}
}

// 메뉴 출력
void menu() {
	printf("====== MENU ======\n");
	printf("1. Encrypt plaintext(ECB Mode)\n");
	printf("2. Decrypt ciphertext(ECB Mode)\n");
	printf("3. Compare files\n");
	printf("4. Execute pipeline\n");
	printf("5. Exit program\n");
	printf("==================\n");
	printf("Input num : ");
}

// in : 평문 파일 경로
// out : 암호 파일 경로 
void encrypt_plaintext_by_ECB(const char* in, const char* out) {
	Buffer* buffer = set_buffer(in);
	uint8_t* key = generate_key();
	printf("Record your key :"); pinrt_hex_from_uint8(key);
	// 버퍼에 담긴 데이터를 알고리즘에 전달
	// 8바이트씩 디코딩 함수에 전달해야된다.(패딩처리를 했기 때문에 가능)
	// for(int i = 0; i < buffer->size; i+=8) {
	// 	encode_by_des(&(buffer->data[i]), key);
	// }	
	save_file(out, buffer);	
	close_buffer(buffer);
}

void decrypt_ciphertext_by_ECB(const char* in, const char* out) {

}

Buffer* set_buffer(const char* file_path) {
	// 파일 읽기
	FILE* fp = fopen(file_path, "rb");
	if(fp == NULL) {
		printf("No file path exists\n");
		exit(1);
	}

	// 읽어온 파일을 바탕으로 버퍼 초기화
	Buffer* buffer = (Buffer*)malloc(sizeof(Buffer)); 
	init_buffer(buffer); int byte;
	while((byte = fgetc(fp)) != EOF) {
		push_buffer(buffer, (uint8_t)byte);
	}

	// 나머지 바이트는 0으로 패딩
	if(buffer->size % 8 != 0) {
		for(int j = 0; j < 8; j++) {
			if(j > buffer->size % 8) {
				push_buffer(buffer, 0);
			} 
		}
	}
	fclose(fp);
	return buffer;
}

void save_file(const char* file_path, Buffer* buffer) {
	FILE* fp = fopen(file_path, "wb");
	if(fp == NULL) {
		printf("No file path exists\n");
		exit(1);
	}
    int written = fwrite(buffer->data, 1, buffer->size, fp);
    if (written != buffer->size) {
        exit(1);
    }
	fclose(fp);
}

// 주어진 두 파일을 비교해 일치한지 확인
void compare(const char* encrypt_file_path, const char* decrypt_file_path) {

}

// 입력으로 주어진 평문 파일(경로)와 암호문이 저장될 파일 경로를 입력으로 받음
// 암호화 + 복호화 + 결과비교까지 전부 한꺼번에 실행
void execute_des_pipeline(const char* encrypt_file_path, const char* decrypt_file_path) {

}


void pinrt_hex_from_uint8(uint8_t* data) {
	for(int i = 0; i < 8; i++) {
		printf(" %x", data[i]);
	}
	printf("\n");
}

uint8_t* read_key() {

}
