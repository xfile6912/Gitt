#ifndef GITT_H
#define GITT_H

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include "list.h"
#include "blob_tree.h"

char cwd[MAX_LINE];

//명령을 분류하여 실행
void execute_command(int argc, char *argv[]);

//gitt help
void gitt_help();

//gitt init
void gitt_init(int argc, char *argv[]);

//gitt status
void gitt_status(int argc, char *argv[]);

//cur folder 안의 폴더 및 하위 파일들을 재귀적으로 탐색하여, blob파일로 만들고 index파일에 정보를 저장해 줌, ./gitt add . 시 사용
void recursive_add(char *cur_folder, FILE *index);

//gitt add [filename] .. or ./gitt add .
void gitt_add(int argc, char *argv[]);

//gitt commit [commit message]
void gitt_commit(int argc, char *argv[]);

//gitt branch [branch name]
void gitt_branch(int argc, char *argv[]);

//commit_hash를 받아 해당 commit에 해당하는 내용으로 working directory의 내용을 바꿈
void update_files(char *commit_hash);

//gitt checkout [branch name] or [commit hash]
void gitt_checkout(int argc, char *argv[]);

// msg에 해당하는 error 출력
void print_error(char *msg);

//해당 저장소가 git init 되었는지의 여부 체크
int is_inited();

// file_path에 해당하는 file이 존재하면 1 반환, 존재하지 않으면 0 반환
int is_file_exist(char *file_path);

//index file을 idx_list 자료구조로 읽어옴, index_file이 없어 실패하면 0반환, 있으면 1반환
int read_index_file_to_list(struct list *idx_list);

//숨김폴더를 제외한 폴더 안의 모든 파일 및 폴더를 제거
void delete_all_files_in_folder(char *cur_folder);

//tree_hash와, tree_hash가 가리키는 폴더에 대한 상대경로를 이용하여, 해당 tree_hash에 속하는 폴더 및 파일들을 재귀적으로 생성
void create_files_using_tree_hash(char *tree_hash, char *relative_path);


//from에 해당하는 파일을, to로 복사
void file_copy(char *from_path, char *to_path);
#endif