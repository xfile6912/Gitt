#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include "sha1.h"

#define MAX_LINE 2048

char cwd[MAX_LINE];

//명령을 분류하여 실행
void execute_command(int argc, char *argv[]);
//./gitt init
void gitt_init(int argc, char *argv[]);
//./gitt status
void gitt_status(int argc, char *argv[]);
//./gitt add .
void gitt_add_dot();
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
void byte_to_hex(char *hex, unsigned char *byte);
void create_blob_file(char *hashed_str, char *file_path, off_t file_size);
void create_tree_file(char *hashed_str, char *folder_path);
int is_inited();

int is_inited()
{
    char path[MAX_LINE];
    strcpy(path, cwd);
    strcat(path, "/.gitt");

    return access(path, F_OK);
}

void execute_command(int argc, char *argv[])
{
    if (!strcmp(argv[1], "init"))
        gitt_init(argc, argv);
    else
    {
        // gitt저장소로써 초기화 되지 않았다면
        if (is_inited() == -1)
        {
            print_error("초기화된 gitt저장소가 아닙니다.");
            return;
        }
        else if (!strcmp(argv[1], "status"))
            gitt_status(argc, argv);

        else if (!strcmp(argv[1], "add"))
            gitt_add(argc, argv);

        else if (!strcmp(argv[1], "commit"))
            gitt_commit(argc, argv);

        else if (!strcmp(argv[1], "branch"))
            gitt_branch(argc, argv);

        else if (!strcmp(argv[1], "checkout"))
            gitt_checkout(argc, argv);
        else
            print_error("가용가능 명령어가 아닙니다.");
    }
}

//./gitt init
void gitt_init(int argc, char *argv[])
{
    int result;
    FILE *fp;

    //.gitt폴더를 생성
    result = mkdir(".gitt", 0755);
    if (result && errno == EEXIST)
    {
        print_error("이미 git 저장소로 설정되어 있습니다.");
        return;
    }
    // objects 폴더 생성
    mkdir(".gitt/objects", 0755);
    // refs 폴더 생성
    mkdir(".gitt/refs", 0755);
    // refs 폴더안에 heads 폴더와 tags 폴더 생성
    mkdir(".gitt/refs/heads", 0755);
    mkdir(".gitt/refs/tags", 0755);
    // HEAD 파일 생성
    fp = fopen(".gitt/HEAD", "w");
    fprintf(fp, "refs/heads/master");
    fclose(fp);

    printf("빈 gitt 저장소로 초기화 하였습니다. (%s%s)\n", cwd, "/.gitt");
}
//./gitt status
void gitt_status(int argc, char *argv[])
{
    printf("gitt status\n");
}

void add_all_files_to_index(char *cur_folder, FILE *index)
{
    DIR *dir;
    struct dirent *file;
    struct stat file_stat;

    //현재 폴더를 open
    dir = opendir(cur_folder);
    //파일들 순회
    while ((file = readdir(dir)))
    {
        //해당 파일 or 폴더까지의 경로 생성
        char path[MAX_LINE];
        memset(path, '\0', MAX_LINE);
        
        strcpy(path, cur_folder);
        strcat(path, "/");
        strcat(path, file->d_name);

        stat(path, &file_stat);


        if (S_ISDIR(file_stat.st_mode)) // directory이고 숨김폴더가 아니라면
        {
            if (file->d_name[0] != '.')
            {
                //재귀 적으로 해당 폴더에 들어가서 실행되도록 함
                add_all_files_to_index(path, index);
            }
        }
        else // directory가 아니라면, blob파일을 생성하고, index에 저장
        {
            char hashed_str[MAX_LINE];
            memset(hashed_str, '\0', MAX_LINE);
            create_blob_file(hashed_str, path, file_stat.st_size);
            //+1을 해주는 이유는 cwd이후에 /가 하나 있기 때문
            fprintf(index, "%s %s\n", hashed_str, path + strlen(cwd) + 1);
        }
    }
    closedir(dir);
}

//./gitt add .
void gitt_add_dot()
{
    FILE *index;
    // index 파일을 만듦
    index = fopen(".gitt/index", "w");
    //현재 폴더안에 모든 파일 및 폴더를 받아서
    add_all_files_to_index(cwd, index);
    fclose(index);
}

void byte_to_hex(char *hex, unsigned char *byte)
{
    int i;
    char *ptr = hex;
    for (i = 0; i < 20; i++)
        sprintf(ptr + 2 * i, "%02x", byte[i]);
}

//file이 존재하면 1 반환, 존재하지 않으면 0 반환
int is_file_exist(char *file_path)
{
    //file이 존재하면
    if (access(file_path, F_OK)==1)
    {
        return 1;
    }
    return 0;
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
    strcpy(blob_path, ".gitt/objects/");
    strncat(blob_path, hashed_str, 2);
    mkdir(blob_path, 0755);
    strcat(blob_path, "/");
    strcat(blob_path, hashed_str + 2);

    //해당 blob file이 없으면 blob 파일 생성
    if (!is_file_exist(blob_path))
    {

        fp = fopen(blob_path, "w");
        fwrite(buffer, file_size, sizeof(unsigned char),fp);
        fclose(fp);
    }

    free(buffer);
}

void create_tree_file(char *hashed_str, char *folder_path)
{
}
//./gitt add [filename] .. or ./gitt add .
void gitt_add(int argc, char *argv[])
{
    if (!strcmp(argv[2], ".") && argc == 3) //./gitt add .인 경우
    {
        gitt_add_dot();
    }
}
//./gitt commit [commit message]
void gitt_commit(int argc, char *argv[])
{
    printf("gitt commit\n");
}
//./gitt branch [branch name]
void gitt_branch(int argc, char *argv[])
{
    printf("gitt branch\n");
}
//./gitt checkout [branch name] or [commit hash]
void gitt_checkout(int argc, char *argv[])
{
    printf("gitt checkout\n");
}

void print_error(char *msg)
{
    printf("ERROR: %s\n", msg);
}