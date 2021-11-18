#ifndef STRING_PARSER_H
#define STRING_PARSER_H
#define MAX_LENGTH 1024
#define MAX_ARGC 256
/*
copy str to temp_str
and change ' ' or '\t' to '\0' in temp_str
*/
void init_temp_str(char *str, char *temp_str);

/*
if string contains \' or \", and isn't matched,
then get next line and reinit str, temp_str, length to match it.
*/
void get_next_and_reinit(char *str, char *temp_str);

/*
return matched index of char c
if not exists return -1
*/
int get_matched_idx(int idx, char* str, char c);

/*
return argc of string
return argv of string

if string contains \', \", it should be matched.
so if it isn't matched, get another string to match it.
*/
int parse_argv(char argv[MAX_ARGC][MAX_LENGTH], char *str);

#endif