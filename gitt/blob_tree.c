#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "blob_tree.h"
#include "gitt.h"
#include "sha1.h"

bool blob_item_less_func(const struct list_elem *a, const struct list_elem *b, void *aux)
{
    struct blob_item *blb_item1 = list_entry(a, struct blob_item, elem);
    struct blob_item *blb_item2 = list_entry(b, struct blob_item, elem);
    if (strcmp(blb_item1->file_name, blb_item2->file_name) < 0)
        return true;
    return false;
}

bool tree_item_less_func(const struct list_elem *a, const struct list_elem *b, void *aux)
{
    struct tree_item *tre_item1 = list_entry(a, struct tree_item, elem);
    struct tree_item *tre_item2 = list_entry(b, struct tree_item, elem);
    if (strcmp(tre_item1->folder_name, tre_item2->folder_name) < 0)
        return true;
    return false;
}

void byte_to_hex(char *hex, unsigned char *byte)
{
    int i;
    char *ptr = hex;
    for (i = 0; i < 20; i++)
        sprintf(ptr + 2 * i, "%02x", byte[i]);
}

void dfs_path_and_make_tree(struct tree_item *t, char *hashed_str, char *file_path)
{
    char *next_path = strchr(file_path, '/'); //해당 파일의 경로가 폴더 경로를 포함하고 있는지 ex)file_path=foo/bar.c, next_path=/bar.c
    if (next_path)                            //폴더경로가 있는 경우
    {
        struct tree_item *tre_item;
        char folder_name[MAX_LINE];

        //폴더 경로에서 가장 상위 폴더의 이름을 추출하여 사용
        struct tree_item *search_item = (struct tree_item *)malloc(sizeof(struct tree_item));
        strncpy(search_item->folder_name, file_path, next_path - file_path);

        //폴더 경로에서 가장 상위 폴더에 대한 tree 정보가, 현재 트리 t의 트리 list에 있는지 체크
        struct list_elem *e = list_find(&t->trees, &search_item->elem, tree_item_less_func);
        if (e) //이미 해당 폴더에 대한 tree가 있는 경우, 따로 작업 안해줌
        {
            tre_item = list_entry(e, struct tree_item, elem);
            free(search_item);
        }
        else //해당 폴더에 대한 tree가 없는 경우 새로은 tre_item을 추가해줌
        {
            tre_item = search_item;
            //추가될 tree_item안의 blobs, trees list 초기화
            list_init(&tre_item->blobs);
            list_init(&tre_item->trees);
            //새로운 tree_item 추가
            list_insert_ordered(&t->trees, &tre_item->elem, tree_item_less_func, NULL);
        }

        dfs_path_and_make_tree(tre_item, hashed_str, next_path + 1); //남은 경로에 대해 계속 트리 구조를 만들어줌, '/'을 제외해야하므로 +1을 해줌   //ex)next_path+1=bar.c
    }
    else //파일인 경우
    {
        //읽어들인 내용을 기반으로 blob_item 생성하여 blobs list에 넣음
        struct blob_item *blb_item = (struct blob_item *)malloc(sizeof(struct blob_item));
        strcpy(blb_item->hashed_str, hashed_str);
        strcpy(blb_item->file_name, file_path);
        list_insert_ordered(&t->blobs, &blb_item->elem, blob_item_less_func, NULL);
    }
}

void give_hashed_str_to_tree(struct tree_item *t)
{
   
    char *buffer = (char *)malloc(2048*(list_size(&t->trees) + list_size(&t->blobs)));
    strcpy(buffer, "\0");

    //t->trees는 ordered로 들어가기 때문에 들어가는 순서에 관계없이 항상 동일
    //t안에 속해있는 모든 tree_item에 hashed_str을 부여하고 이를 버퍼에 추가
    struct list_elem *e;
    for (e = list_begin (&t->trees); e != list_end (&t->trees); e = list_next (e)) 
    {
        struct tree_item *tre_item = list_entry(e, struct tree_item, elem);
        //해당 tree_item에 hashed_str을 부여
        give_hashed_str_to_tree(tre_item);

        strcat(buffer, "tree ");
        strcat(buffer, tre_item->hashed_str);
        strcat(buffer, " ");
        strcat(buffer, tre_item->folder_name);
        strcat(buffer, "\n");
    }

    //t->blobs는 ordered로 들어가기 때문에 들어가는 순서에 관계없이 항상 동일
    //t안에 속해있는 모든 blob_item을 돌며 buffer에 추가
    for (e = list_begin (&t->blobs); e != list_end (&t->blobs); e = list_next (e)) 
    {
        struct blob_item *blb_item = list_entry(e, struct blob_item, elem);

        strcat(buffer, "blob ");
        strcat(buffer, blb_item->hashed_str);
        strcat(buffer, " ");
        strcat(buffer, blb_item->file_name);
        strcat(buffer, "\n");
    }

    //버퍼의 내용을 sha1을  통해 해시
    unsigned char hashed_byte[MAX_LINE];
    SHA1(hashed_byte, (unsigned char*)buffer, strlen(buffer));
    byte_to_hex(t->hashed_str, hashed_byte);
    free(buffer);
}

int read_index_file_to_tree(struct tree_item *t)
{
    FILE *index = fopen(".gitt/index", "r");
    if (!index) // index 파일이 없는 경우
    {
        return 0;
    }
    // index 파일이 있는 경우
    list_init(&t->blobs);
    list_init(&t->trees);

    char hashed_str[MAX_LINE]; // staged area 안의 파일의 hashed str
    char file_path[MAX_LINE];  // staged area 안의 파일의 경로
    while (1)
    {
        // index파일에서 각 파일의 내용을 읽음
        int res = fscanf(index, "%s %s", hashed_str, file_path);
        if (res == EOF)
            break;

        //해당 파일을 트리 구조로 만들어냄
        dfs_path_and_make_tree(t, hashed_str, file_path);
    }

    //여기까지, 이제 모든 tree 구조를 만들었지만, tree_item 자료구조의 hashed_str은 비어있게 됨
    //따라서 tree 구조 안의 내용을 바탕으로 hashed_str을 채워주어야 함
    give_hashed_str_to_tree(t);

    fclose(index);
    return 1;
}

void make_object_path(char *object_path, char *hashed_str)
{
    strcpy(object_path, ".gitt/objects/");
    strncat(object_path, hashed_str, 2);
    mkdir(object_path, 0755);
    strcat(object_path, "/");
    strcat(object_path, hashed_str + 2);
}

void create_blob_file(char *hashed_str, char *file_path, off_t file_size)
{
    FILE *fp;
    unsigned char *buffer;

    char blob_path[MAX_LINE];
    unsigned char hashed_byte[MAX_LINE];

    memset(blob_path, '\0', MAX_LINE);
    memset(hashed_byte, '\0', MAX_LINE);

    //파일 오픈
    fp = fopen(file_path, "r");

    //파일로부터 내용으ㄹ 버퍼로 읽음
    buffer = (unsigned char *)malloc(sizeof(unsigned char) * file_size);
    memset(buffer, '\0', file_size);
    fread(buffer, file_size, sizeof(unsigned char), fp);
    fclose(fp);

    //파일을 sha1 해시함수를 통해 해시
    SHA1(hashed_byte, buffer, file_size);
    byte_to_hex(hashed_str, hashed_byte);

    // hashed_str을 기반으로 폴더 생성하고 blob path 생성
    make_object_path(blob_path, hashed_str);

    //해당 blob file이 없으면 blob 파일 생성
    if (!is_file_exist(blob_path))
    {
        file_copy(file_path, blob_path);
    }

    free(buffer);
}

void free_all_sub_trees_and_blobs(struct tree_item *t)
{

    //t 안에 속한 모든 tree_item 순회하며 동적할당 해제
    struct list_elem *e;
    for (e = list_begin (&t->trees); e != list_end (&t->trees);) 
    {
        struct tree_item *tre_item = list_entry(e, struct tree_item, elem);
        //t 안의 모든 tree item을 돌며 동적할당 해제
        free_all_sub_trees_and_blobs(tre_item);

        e = list_remove(e);
        free(tre_item);
    }
    
    //t에 속한 blobs의 동적할당 해제
    for (e = list_begin (&t->blobs); e != list_end (&t->blobs);)
    {
        struct blob_item *blb_item = list_entry(e, struct blob_item, elem);
        e = list_remove(e);
        free(blb_item);
    }

}

void create_tree_file(struct tree_item *t)
{
    
    //t의 hashed str을 통해 tree_path를 생성
    char tree_path[MAX_LINE];
    make_object_path(tree_path, t->hashed_str);

    if(is_file_exist(tree_path))//해당 tree path에 대해 파일이 이미 있으면 만들지 않아도 됨
        return;

    //해당 파일이 없으면 파일을 쓰기 모드로 생성
    FILE *fp=fopen(tree_path, "w");

    //t에 속한 모든 tree_item 순회
    struct list_elem *e;
    for (e = list_begin (&t->trees); e != list_end (&t->trees); e = list_next (e)) 
    {
        struct tree_item *tre_item = list_entry(e, struct tree_item, elem);
        //해당 tree_item에 대해 또 tree_file 생성
        create_tree_file(tre_item);

        //tree_file에 해당 tree_item 내용 기록
        fprintf(fp, "tree %s %s\n", tre_item->hashed_str, tre_item->folder_name);
    }

    //t에 속한 모든 blob_item 순회
    for (e = list_begin (&t->blobs); e != list_end (&t->blobs); e = list_next (e)) 
    {
        struct blob_item *blb_item = list_entry(e, struct blob_item, elem);
        
        //tree_file에 해당 blob_item 내용 기록
        fprintf(fp, "blob %s %s\n", blb_item->hashed_str, blb_item->file_name);
    }

    fclose(fp);
}

void create_commit_file(char * hashed_str, struct tree_item *t, char * commit_msg, char * parent)
{
    //t에 대한 tree file들을 생성
    create_tree_file(t);

    char *buffer = (char *)malloc(sizeof(char) * MAX_LINE * 4);
    strcpy(buffer, "\0");

    //tree에 대한 정보 입력
    strcat(buffer, "tree ");
    strcat(buffer, t->hashed_str);
    strcat(buffer, "\n");
    
    //parent에 대한 정보 입력
    if(parent != NULL)
    {
        strcat(buffer, "parent ");
        strcat(buffer, parent);
        strcat(buffer, "\n");
    }

    strcat(buffer, "\n");
    strcat(buffer, commit_msg);


    //버퍼의 내용을 sha1을  통해 해시
    unsigned char hashed_byte[MAX_LINE];
    SHA1(hashed_byte, (unsigned char*)buffer, strlen(buffer));
    byte_to_hex(hashed_str, hashed_byte);

    // hashed_str을 기반으로 폴더 생성하고 blob path 생성
    char commit_path[MAX_LINE];
    memset(commit_path, '\0', MAX_LINE);

    //hashed_str을 이용하여 object path를 만들어냄
    make_object_path(commit_path, hashed_str);

    //object path를 이용해 commit_file 생성
    FILE *fp = fopen(commit_path, "w");
    fprintf(fp, "%s\n", buffer);
    
    fclose(fp);
    free(buffer);
}