## 따옴표에 대한 처리를 수행한 Argument Parser

### 함수
```
/* copy str to temp_str and change ' ' or '\t' to '\0' in temp_str */
void init_temp_str(char *str, char *temp_str);

/* if string contains \' or \", and isn't matched, then get next line and reinit str, temp_str to match it. */
void get_next_and_reinit(char *str, char *temp_str);

/* return matched index of char c. if not exists return -1 */
int get_matched_idx(int idx, char* str, char c);

/*return argc of string ,return argv of string*/
int parse_argv(char argv[MAX_ARGC][MAX_LENGTH], char *str);
```
### 결과<br>
<img src="https://user-images.githubusercontent.com/57051773/142829505-5e835fa6-9817-4c93-abf9-8770315021ba.png"  width="500"/>

