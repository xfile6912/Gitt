#include <stdio.h>


//구현할 기능
//gitt init
//gitt status
//gitt add
//gitt commit 
//gitt branch
//gitt checkout

//명령을 분류하여 실행
void execute_command(int argc, char *argv[]);
//gitt 저장소로 초기화
void gitt_init();
//gitt의 상태를 출력
void gitt_status();
//파일을 index 영역으로 올림
void gitt_add();
//index 영역을 버전으로서 등록
void gitt_commit();
//git의 branch
void gitt_branch();
//git의 checkout
void gitt_checkout();