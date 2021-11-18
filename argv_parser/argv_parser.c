#include <stdio.h>
#include <string.h>
#include "argv_parser.h"

void init_temp_str(char *str, char *temp_str)
{
    strcpy(temp_str, str); //str을 temp_str에 복사

    int i;
    for (i = 0; str[i]!='\0'; i++)
    {
        //빈칸을 모두 \0로 바꾸어줌
        if (temp_str[i] == ' ' || temp_str[i] == '\t')
            temp_str[i] = '\0';
    }
}

void get_next_and_reinit(char *str, char *temp_str)
{
    char new_str[MAX_LENGTH];
    printf("quote> ");
    scanf(" %[^\n]", new_str);
    strcat(str, "\n");//enter를 입력해주고
    strcat(str, new_str);//new_string을 입력해줌

    init_temp_str(str, temp_str);//str이 더 길어졌으므로 temp_str도 그에 맞게 재초기화 해줌
}

int get_matched_idx(int idx, char *str, char c)
{
    int i;
    int matched_idx = -1; //matched인 경우 matched한 char의 idx 반환

    for (i = idx + 1; str[i] != '\0'; i++)
    {
        if (str[i] == c)
        {
            matched_idx = i;
            break;
        }
    }
    return matched_idx;
}

int parse_argv(char argv[MAX_ARGC][MAX_LENGTH], char *str)
{
    int i;
    char temp_str[1024];      //str을 parsing할 때 임시로 쓰이는 공간
    int argc = 0;
    init_temp_str(str, temp_str);

    int idx = 0;
    while (idx < strlen(str))
    {
        if (temp_str[idx] == '\0')
        {
            idx++;
            continue;
        }

        if (temp_str[idx] == '\"' || temp_str[idx] == '\'')
        {
            
            while (1)
            {
                int matched_idx = get_matched_idx(idx, str, temp_str[idx]);
                if (matched_idx != -1) //matched 한 경우
                {
                    //따옴표는 빼주어야 하기 때문에 str+idx+1부터 시작
                    strncpy(argv[argc++], str + idx + 1, matched_idx - idx - 1);
                    idx=matched_idx+1;
                    break;
                }
                else//matched하지 않은 경우 matched 할때 까지 new_line을 받아야함
                {
                    get_next_and_reinit(str, temp_str);
                }
            }
        }
        else
        {
            //idx에 해당하는 문자가 \0가 아니라면 argv에 넣어줌
            strcpy(argv[argc++], temp_str + idx);
            idx += strlen(temp_str + idx);
        }
    }

    return argc;
}