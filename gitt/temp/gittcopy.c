#include "gitt.h"

int main(int argc, char* argv[])
{
    //현재 디렉토리를 cwd에 저장
    getcwd(cwd, sizeof(cwd));
    //명령어 실행
    execute_command(argc, argv);
}