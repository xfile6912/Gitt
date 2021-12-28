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

//해당 저장소가 git init 되었는지의 여부 체크
int is_inited();
// file이 존재하면 1 반환, 존재하지 않으면 0 반환
int is_file_exist(char *file_path);
//index file로부터 idx_list 자료구조로 읽어옴, 없으면 0반환, 있으면 1반환
int read_index_file_to_list(struct list *idx_list);
#endif