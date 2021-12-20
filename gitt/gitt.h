#ifndef GITT_H
#define GITT_H

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include "hash.h"

#define MAX_LINE 2048

//index 파일 내의 각 파일에 대한 정보(hashed_str, 파일 경로)를 저장
struct index_item
{
    struct hash_elem elem;
    char hashed_str[MAX_LINE];
    char file_path[MAX_LINE];
};

char cwd[MAX_LINE];


//file path가 hash의 키가 됨, index item을 hash table에 저장할 때 사용하는 함수
unsigned index_item_hash_func(const struct hash_elem* e, void* aux);
//index item을 비교할 때 사용하는 함수, 같으면 false를 반환하고, 다르면 true를 사용
bool index_item_hash_less_func(const struct hash_elem* a,const struct hash_elem* b, void* aux);
//index item에 대한 hash table을 삭제할 때 사용하는 함수
void index_item_hash_delete_func(struct hash_elem* e, void* aux);

//명령을 분류하여 실행
void execute_command(int argc, char *argv[]);
//./gitt help
void gitt_help();
//./gitt init
void gitt_init(int argc, char *argv[]);
//./gitt status
void gitt_status(int argc, char *argv[]);
//재귀적으로 gitt add dot 수행, cur folder 안의 폴더 및 하위 파일들을 재귀적으로 탐색하여, blob파일로 만들고 index파일에 정보를 저장해 줌
void gitt_add_dot(char *cur_folder, FILE *index);
//./gitt add [filename] .. or ./gitt add .
void gitt_add(int argc, char *argv[]);
//./gitt commit [commit message]
void gitt_commit(int argc, char *argv[]);
//./gitt branch [branch name]
void gitt_branch(int argc, char *argv[]);
//./gitt checkout [branch name] or [commit hash]
void gitt_checkout(int argc, char *argv[]);
// error 출력
void print_error(char *msg);

//byte를 hex형태로 바꾸어줌
void byte_to_hex(char *hex, unsigned char *byte);
//특정 file_path와 file의 size를 받아 blob파일로 만들어주고, blob_file의 hashed string을 반환, 파일의 상대주소 이용
void create_blob_file(char *hashed_str, char *file_path, off_t file_size);
void create_tree_file(char *hashed_str, char *folder_path);
//해당 저장소가 git init 되었는지의 여부 체크
int is_inited();
// file이 존재하면 1 반환, 존재하지 않으면 0 반환
int is_file_exist(char *file_path);
#endif