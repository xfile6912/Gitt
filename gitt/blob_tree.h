#ifndef BLOB_TREE_H
#define BLOB_TREE_H

#include "list.h"

#define MAX_LINE 2048

//blob 정보 저장
struct blob_item
{
    char hashed_str[MAX_LINE];//BLOB파일의 hash
    char file_name[MAX_LINE];//BLOB파일이 나타내는 파일이름

    struct list_elem elem;  //list의 원소로 사용할 때 사용
};

//tree 자료구조
struct tree_item
{
    struct list blobs;//하나의 tree파일에 속한 blob파일들
    struct list trees;//하나의 tree파일에 속한 tree파일들
    char hashed_str[MAX_LINE];//하나의 tree파일에 해당하는 hashed_str
    char folder_name[MAX_LINE];//하나의 tree파일이 나타내는 폴더 이름

    struct list_elem elem;//list 원소로 사용할 때 사용
};

//blob item을 비교할 때 사용하는 함수
bool blob_item_less_func(const struct list_elem* a,const struct list_elem* b, void* aux);

//tree item을 비교할 때 사용하는 함수
bool tree_item_less_func(const struct list_elem* a,const struct list_elem* b, void* aux);

//hashed_str을 이용해 object 폴더 생성하고, object 경로를 추출
void make_object_path(char *object_path, char *hashed_str);

//byte를 hex형태로 바꾸어줌
void byte_to_hex(char *hex, unsigned char *byte);
//특정 file_path와 file의 size를 받아 blob파일로 만들어주고, blob_file의 hashed string을 반환, 파일의 상대주소 이용
void create_blob_file(char *hashed_str, char *file_path, off_t file_size);


//파일 path를 타고 재귀적으로 탐색하여 tree 자료구조를 생성
void dfs_path_and_make_tree(struct tree_item *t, char *hashed_str, char *file_path);

//tree item을 재귀적으로 순회하며 hashed_str을 부여
void give_hashed_str_to_tree(struct tree_item *t);

//tree_item t를 받아 tree_file을 생성해줌
void create_tree_file(struct tree_item *t);

//tree_item t와 commit message를 받아 커밋 파일을 만듦, commit_file의 hashed string을 반환
void create_commit_file(char * hashed_str, struct tree_item *t, char * commit_msg, char * parent);

//index파일의 내용을 읽어 tree 자료구조로 만듦, 만약 index파일이 없으면 0 반환, 있으면 1반환
int read_index_file_to_tree(struct tree_item *t);

//tree item t에 속해있는 모든 sub tree 및 blob구조를 free 해줌
void free_all_sub_trees_and_blobs(struct tree_item *t);
#endif