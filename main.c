/*
1.  DES 암호 알고리즘을 완성하시오.

2. 완성한 알고리즘을 활용하며 문서 암호 프로그램을 만드시오. (ECB mode)
	2-1. text 파일을 읽어와서 암호문 파일 만들기 (plaintext -> ciphertext)
	2-2. 암호문 파일을 읽어와서 복호화 파일 만들기 (ciphertext -> plaintext2)
	2-3. 복호화한 파일과 최초의 text 파일이 일치하는지 확인하기 (plaintext2 == plaintext)

	* 사용하는 데이터타입 : uint8_t 사용 -> 1바이트 크기의 양의 정수 = unsigned char형과 동일
		기본적으로 아스키코드로 인코딩된 파일 속 문자 하나는 0과 255 사이 값을 가진다.
		그래서 파일 출력값에 대해 char형(-128과 127 사이값)을 사용하는 것이 아니라 unsigned char형을 사용해 안정적인 형변환.

	* 알고리즘 진행 과정에서 비트를 확장하고 축소하는 과정 때문에 uint8_t 타입만 사용하기 불편한 시점이 있음
		그래서 uint32_t = unsigned int 형으로 한꺼번에 값을 담아 비트 단위 연산을 진행한다.

	* 암호 프로그램 설명
		1번(encrypt plaintext) 입력 시. 사용자로부터 입력 받은 평문 파일의 절대경로를 참조해 암호 파일 생성
		2번(decrypt ciphertext) 입력 시. 사용자로부터 입력 받은 암호 파일의 절대경로를 참조해 복호 파일 생성
		3번(compare two file) 입력 시. 두 개의 절대 경로 파일을 읽어들여 동일한 파일인지 확인
		4번(auto) 입력 시. 사용자로부터 평문 파일의 절대 경로만 입력 받아 위 세 가지 기능에 대한 파이프라인 실행
		모든 예외 상황에 대해 별도 조치 없이 프로그램이 종료되는 방향으로 진행 

3. 알고리즘 추가 참고 자료 : https://434official.tistory.com/1, https://434official.tistory.com/2

4. 구현 참고 자료 : 
		- https://mypage-dream.tistory.com/11
		- https://nobilitycat.tistory.com/entry/c-DES-%EA%B5%AC%EC%A1%B0-%EB%B0%8F-%EC%BD%94%EB%93%9C 
		- ChatGPT 활용
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h> 
#include <memory.h>
#include <stdbool.h>
#include "buffer.h"
#include "des.h"

#define MAX_BUFFER_SIZE 100

// 사용자로부터 입력받을 수 있는 이벤트 리스트
typedef enum SELECT {
	ENCRYPT = 1,
	DECRYPT,
	COMPARE,
	PIPELINE,
	EXIT
} SELECT;

//  ============ Program Functions ============
void execute_des_pipeline(const char*, const char*, const char*);
void encrypt_plaintext_by_ECB(const char*, const char*);
void decrypt_ciphertext_by_ECB(const char*, const char*);
void compare(const char*, const char*);
void save_file(const char*, Buffer*, bool);
void pinrt_uint8(uint8_t*, bool);
void program(); 
void menu();
Buffer* set_buffer(const char*, bool);
uint8_t* read_key();

int main(int argc, char* argv[]) {
	if(argc != 1) {
		printf("Wrong input format");
		exit(0);
	}
	// 프로그램 시작
	program();
	return 0;
}

// 프로그램 시작 함수
void program() {
	char file_path1[MAX_BUFFER_SIZE];
	char file_path2[MAX_BUFFER_SIZE];
	char file_path3[MAX_BUFFER_SIZE];
	while(1) {
		SELECT num = 5;
		menu(); scanf("%d", &num);
		if(num == EXIT) {
			printf("Exit program. bye bye");
			exit(1);
		}

		switch (num) {
			case ENCRYPT:
				printf("Insert plain file path : ");
				scanf("%s", file_path1); getchar();
				printf("Insert chiper file path : ");
				scanf("%s", file_path2); getchar();
				encrypt_plaintext_by_ECB(file_path1, file_path2); break;

			case DECRYPT:
				printf("Insert chiper file path : ");
				scanf("%s", file_path1); getchar();
				printf("Insert recovery file path : ");
				scanf("%s", file_path2); getchar();
				decrypt_ciphertext_by_ECB(file_path1, file_path2); break;

			case COMPARE:
				printf("Input plain file path : ");
				scanf("%s", file_path1); getchar();
				printf("Input recovery file path : ");
				scanf("%s", file_path2); getchar();
				compare(file_path1, file_path2); break;

			case PIPELINE:
				printf("Input plain file path : ");
				scanf("%s", file_path1); getchar();
				printf("Input chiper file path : ");
				scanf("%s", file_path2); getchar();
				printf("Input recovery file path : ");
				scanf("%s", file_path3); getchar();
				execute_des_pipeline(file_path1, file_path2, file_path3); break;
				
			default:
				printf("Wrong Number, please select 1~5\n"); break;
		}
		memset(file_path1, 0, sizeof(char) * MAX_BUFFER_SIZE);
		memset(file_path2, 0, sizeof(char) * MAX_BUFFER_SIZE);
		memset(file_path3, 0, sizeof(char) * MAX_BUFFER_SIZE);
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

// in : 저장되어 있는 평문 파일 경로, out : 저장될 암호 파일 경로 
void encrypt_plaintext_by_ECB(const char* in, const char* out) {
	// 파일로부터 
	Buffer* buffer = set_buffer(in, true);
	uint8_t* key = generate_key();
	printf("Record your key : "); pinrt_uint8(key, false);
	// 버퍼에 담긴 데이터를 알고리즘에 전달
	// 8바이트씩 디코딩 함수에 전달해야된다.(패딩처리를 했기 때문에 가능)
	for(int i = 0; i < buffer->size; i+=8) {
		execute_des(&(buffer->data[i]), key, true);
	}	
	save_file(out, buffer, true);	
	// 동적 할당 정리
	close_buffer(buffer);
	free(key);
}

// in : 저장되어 있는 암호 파일 경로, out : 저장될 평문 파일 경로 
void decrypt_ciphertext_by_ECB(const char* in, const char* out) {
	// 버퍼설정, 저장되어 있는 암호 파일 읽기
	Buffer* buffer = set_buffer(in, false);
	// 사용자로부터 대칭키 읽기
	uint8_t* key = read_key();
	// 암호화된 블록 복호화
	for(int i = 0; i < buffer->size; i+=8) {
		execute_des(&(buffer->data[i]), key, false);
	}	
	// 복화화한 데이터 저장
	save_file(out, buffer, false);
	// 동적 할당 정리	
	close_buffer(buffer);
	free(key);
}

// 파일 데이터를 버퍼에 저장
Buffer* set_buffer(const char* file_path, bool encode) {
	// 파일 읽기
	FILE* fp = fopen(file_path, "rb");
	if(fp == NULL) {
		printf("No file path exists\n");
		exit(1);
	}

	// 파일의 정보를 담는 버퍼 생성
	Buffer* buffer = (Buffer*)malloc(sizeof(Buffer)); 
	init_buffer(buffer); int byte;
	// 파일 암호화 시
	if (encode) {
		// 파일 길이에 대한 정보를 작성
		while((byte = fgetc(fp)) != EOF) {
			push_buffer(buffer, (uint8_t)byte);
		}
		buffer->real_size = buffer->size;
		// 나머지 바이트는 0으로 패딩
		int r = buffer->size % 8; 
		if(r != 0) {
			for(int j = 0; j < 8 - r; j++) {
				push_buffer(buffer, 0);
			}
		}
	// 파일 복호화 시 
	} else {
		// 암호화할 때 입력해놓은 실제 평문의 크기를 출력
		fread(&buffer->real_size, sizeof(int), 1, fp);
		// 암호화된 데이터를 버퍼에 삽입
		while((byte = fgetc(fp)) != EOF) {
			push_buffer(buffer, (uint8_t)byte);
		}
	}
	fclose(fp);
	return buffer;
}

// 버퍼에 있는 데이터를 파일에 저장
void save_file(const char* file_path, Buffer* buffer, bool encode) {
	FILE* fp = fopen(file_path, "wb");
	if (fp == NULL) {
		printf("No file path exists\n");
		exit(1);
	}

	// 암호화한 파일에 실제 파일 크기를 작성
	if(encode) {
		// 인코딩 시점의 원본 파일 평문의 길이를 파일에 입력
		int written = fwrite(&buffer->real_size, sizeof(int), 1, fp);
		if (written != 1) {
			exit(1);
		}
		// 버퍼
		written = fwrite(buffer->data, 1, buffer->size, fp);
		if (written != buffer->size) {
			exit(1);
		}
	// 복호화한 경우, 실제 파일 크기만큼 해독한 파일 저장
	} else {
		// 실제 버퍼 사이즈만큼 복호화한 파일에 버퍼 속에 데이터를 입력
		int written = fwrite(buffer->data, 1, buffer->real_size, fp);
		if (written != buffer->real_size) {
			exit(1);
		}
	}
	fclose(fp);
}

// 주어진 두 파일을 비교해 일치한지 확인
void compare(const char* plain_file_path, const char* recovery_file_path) {
	// 원본 파일과 복호화한 파일을 연다
    FILE *fp1 = fopen(plain_file_path, "rb");
    FILE *fp2 = fopen(recovery_file_path, "rb");
    if (!fp1 || !fp2) {
        printf("File open error\n");
        if (fp1) fclose(fp1);
        if (fp2) fclose(fp2);
        exit(1);
    }

    bool same = true;
    unsigned char buf1[MAX_BUFFER_SIZE], buf2[MAX_BUFFER_SIZE];
    int r1, r2;
    while (1) {
        r1 = fread(buf1, 1, sizeof(buf1), fp1);
        r2 = fread(buf2, 1, sizeof(buf2), fp2);

		// 방금 읽어들인 파일의 내용이 서로 상이하면 같지 않음
        if (memcmp(buf1, buf2, r1) != 0) {
            same = false;
            break;
        }
        if (r1 == 0) {
            break;
        }
    }
    fclose(fp1);
    fclose(fp2);

	// 결과 출력
    if (same) {
        printf("Files are Identical\n");
    } else {
        printf("Files are Different\n");
    }
}

// 입력으로 주어진 평문 파일(경로)와 암호문이 저장될 파일 경로를 입력으로 받음
// 암호화 + 복호화 + 결과비교까지 전부 한꺼번에 실행
void execute_des_pipeline(const char* plain_file_path, const char* chiper_file_path, const char* recovery_file_path) {
	encrypt_plaintext_by_ECB(plain_file_path, chiper_file_path);
	decrypt_ciphertext_by_ECB(chiper_file_path, recovery_file_path);
	compare(plain_file_path, recovery_file_path);
}

void pinrt_uint8(uint8_t* data, bool hex) {
	printf("\"");
	for(int i = 0; i < 8; i++) {
		if(hex) {
			if(i == 7) {
				printf("%x", data[i]);
			} else {
				printf("%x ", data[i]);
			}
		} else {
			if(i == 7) {
				printf("%d", data[i]);
			} else {
				printf("%d ", data[i]);
			}			
		}

	}
	printf("\"\n");
}

// 사용자로부터 키를 입력
uint8_t* read_key() {
	char ckey[MAX_BUFFER_SIZE] = {0, };
	uint8_t* key = (uint8_t*)malloc(sizeof(uint8_t) * 8);
	printf("Insert your key : ");
	fgets(ckey, MAX_BUFFER_SIZE, stdin);
	char* temp = strtok(ckey, " "); int i = 0;
	while(temp) {
		if(temp == NULL) {
			break;
		}
		key[i++] = atoi(temp);
		temp = strtok(NULL, " ");
	}
	return key;
}