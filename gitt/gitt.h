#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

#define MAX_LINE 2048

char cwd[MAX_LINE];


//명령을 분류하여 실행
void execute_command(int argc, char *argv[]);
//./gitt init
void gitt_init(int argc, char *argv[]);
//./gitt status
void gitt_status(int argc, char *argv[]);
//./gitt add [filename] .. or ./gitt add .
void gitt_add(int argc, char *argv[]);
//./gitt commit [commit message]
void gitt_commit(int argc, char *argv[]);
//./gitt branch [branch name]
void gitt_branch(int argc, char *argv[]);
//./gitt checkout [branch name] or [commit hash]
void gitt_checkout(int argc, char *argv[]);

void print_error(char *msg);

void execute_command(int argc, char *argv[])
{
    if(!strcmp(argv[1], "init"))
        gitt_init(argc, argv);

    else if(!strcmp(argv[1], "status"))
        gitt_status(argc, argv);

    else if(!strcmp(argv[1], "add"))
        gitt_add(argc, argv);

    else if(!strcmp(argv[1], "commit"))
        gitt_commit(argc, argv);

    else if(!strcmp(argv[1], "branch"))
        gitt_branch(argc, argv);

    else if(!strcmp(argv[1], "checkout"))
        gitt_checkout(argc, argv);
    else
        printf("가용가능 명령어가 아닙니다.");
}

//./gitt init
void gitt_init(int argc, char *argv[])
{
    int result;
    FILE *fp;

    //.gitt폴더를 생성
    result = mkdir(".gitt", 0755);
    if(result && errno==EEXIST)
    {
        print_error("이미 git 저장소로 설정되어 있습니다.");
        return;
    }
    //objects 폴더 생성
    mkdir(".gitt/objects", 0755);
    //refs 폴더 생성
    mkdir(".gitt/refs", 0755);
    //refs 폴더안에 heads 폴더와 tags 폴더 생성
    mkdir(".gitt/refs/heads", 0755);
    mkdir(".gitt/refs/tags", 0755);
    //HEAD 파일 생성
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
//./gitt add [filename] .. or ./gitt add .
void gitt_add(int argc, char *argv[])
{
    printf("gitt add\n");
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