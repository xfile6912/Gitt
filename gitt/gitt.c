
#include "sha1.h"
#include "gitt.h"

// file path가 hash의 키가 됨
unsigned index_item_hash_func(const struct hash_elem *e, void *aux)
{
    struct index_item *idx_item = hash_entry(e, struct index_item, elem);
    return hash_string(idx_item->file_path);
}
bool index_item_hash_less_func(const struct hash_elem *a, const struct hash_elem *b, void *aux)
{
    struct index_item *idx_item1 = hash_entry(a, struct index_item, elem);
    struct index_item *idx_item2 = hash_entry(b, struct index_item, elem);
    if (strcmp(idx_item1->file_path, idx_item2->file_path) < 0)
        return false;
    return true;
}
void index_item_hash_delete_func(struct hash_elem* e, void* aux)
{
    struct index_item* idx_item = hash_entry(e, struct index_item, elem);
	free(idx_item);
}
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
    char absolute_path[MAX_LINE]; //절대 주소 저장
    strcpy(absolute_path, cwd);
    strcat(absolute_path, "/");
    strcat(absolute_path, cur_folder);

    //현재 폴더를 open
    dir = opendir(absolute_path);
    //파일들 순회
    while ((file = readdir(dir)))
    {
        //해당 파일 or 폴더까지의 경로 생성
        char path[MAX_LINE];
        memset(path, '\0', MAX_LINE);
        strcat(path, file->d_name);

        stat(path, &file_stat);

        if (S_ISDIR(file_stat.st_mode)) // directory이고 숨김폴더가 아니라면
        {
            strcat(path, "/");
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
            fprintf(index, "%s %s\n", hashed_str, path);
        }
    }
    closedir(dir);
}

void byte_to_hex(char *hex, unsigned char *byte)
{
    int i;
    char *ptr = hex;
    for (i = 0; i < 20; i++)
        sprintf(ptr + 2 * i, "%02x", byte[i]);
}

// file이 존재하면 1 반환, 존재하지 않으면 0 반환
int is_file_exist(char *file_path)
{
    struct stat file_stat;
    stat(file_path, &file_stat);
    // file이 존재하고 directory가 아니면
    if (access(file_path, F_OK) == 0 && !S_ISDIR(file_stat.st_mode))
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
        fwrite(buffer, file_size, sizeof(unsigned char), fp);
        fclose(fp);
    }

    free(buffer);
}

void create_tree_file(char *hashed_str, char *folder_path)
{
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

//./gitt add [filename] .. or ./gitt add .
void gitt_add(int argc, char *argv[])
{
    if (!strcmp(argv[2], ".") && argc == 3) //./gitt add .인 경우
    {
        gitt_add_dot();
    }
    else
    {
        int i;
        //인자 중에 실제로 존재하지 않는 파일이 있다면 오류 메시지 출력하고 종료
        for (int i = 2; i < argc; i++)
        {
            if (!is_file_exist(argv[i]))
            {
                print_error("존재하지 않는 파일이 있습니다.");
                return;
            }
        }

        FILE *index = fopen(".gitt/index", "r");
        // index파일의 정보를 저장할 hash
        struct hash idx_hash;
        hash_init(&idx_hash, index_item_hash_func, index_item_hash_less_func, NULL);
        if (index != NULL)//INDEX파일이 존재한다면
        {
            int res;

            while (1) // index 파일을 한 줄씩 읽어 idx_hash에 저장
            {
                struct index_item *idx_item=(struct index_item*)malloc(sizeof(struct index_item));
                res = fscanf(index, "%s %s", idx_item->hashed_str, idx_item->file_path);
                if (res == EOF)
                    break;

                hash_insert(&idx_hash, &(idx_item->elem));
            }
            fclose(index);
        }
        // argument로 받은 파일을 이용하여 hash 자료구조에 추가하거나, 수정
        for (int i = 2; i < argc; i++)
        {

            struct stat file_stat;
            struct index_item *idx_item=(struct index_item*)malloc(sizeof(struct index_item));
            //파일 path 저장
            strcpy(idx_item->file_path, argv[i]);
            //파일의 size 정보를 알기 위해 file_stat에 파일의 정보 저장
            stat(idx_item->file_path, &file_stat);
            // blob파일을 만들고 blob파일에 대한 hashed_str을 저장
            create_blob_file(idx_item->hashed_str, idx_item->file_path, file_stat.st_size);

            struct hash_elem *e = hash_find(&idx_hash, &idx_item->elem);
            if (e) //해당 파일이 이미 idx_hash에 있다면
            {
                //기존의 index_item의 hashed_str을 새로 생성한 blob_file의 hashed_str로 바꾸어 줌
                struct index_item *existed_item = hash_entry(e, struct index_item, elem);
                strcpy(existed_item->hashed_str, idx_item->hashed_str);
                free(idx_item);
            }
            else //없으면 추가해줌
            {
                hash_insert(&idx_hash, &idx_item->elem);
            }
        }

        // hash의 내용을 새로 index파일에 써줌
        index = fopen(".gitt/index", "w");
        struct hash_iterator it;

        hash_first(&it, &idx_hash);
        while (hash_next(&it))
        {
            struct index_item *idx_item = hash_entry(hash_cur(&it), struct index_item, elem);
            fprintf(index, "%s %s\n", idx_item->hashed_str, idx_item->file_path);
        }

        hash_destroy(&idx_hash, index_item_hash_delete_func);
        fclose(index);
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
