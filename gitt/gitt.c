#include "gitt.h"

int is_inited()
{
    char path[MAX_LINE];
    strcpy(path, cwd);
    strcat(path, "/.gitt");

    return access(path, F_OK);
}

void execute_command(int argc, char *argv[])
{
    if (argc == 1 || !strcmp(argv[1], "help"))
    {
        gitt_help();
    }
    else if (!strcmp(argv[1], "init"))
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

void gitt_help()
{
    printf("./gitt help\n");
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
    fprintf(fp, ".gitt/refs/heads/master");
    fclose(fp);

    printf("빈 gitt 저장소로 초기화 하였습니다. (%s%s)\n", cwd, "/.gitt");
}
//./gitt status
void gitt_status(int argc, char *argv[])
{
    printf("gitt status\n");
}

void gitt_add_dot(char *cur_folder, FILE *index)
{
    DIR *dir;
    struct dirent *file;
    struct stat file_stat;
    char cur_folder_absolute[MAX_LINE]; //절대 주소 저장

    // curfolder를 통해 curfolder까지의 절대경로를 알아냄
    strcpy(cur_folder_absolute, cwd);
    strcat(cur_folder_absolute, "/");
    strcat(cur_folder_absolute, cur_folder);

    //절대 경로를 통해 cur folder를 open
    dir = opendir(cur_folder_absolute);

    // cur folder 내의 파일들 및 폴더들 순회
    while ((file = readdir(dir)))
    {

        // cur folder 내의 파일들 및 폴더 까지의 상대 경로를 저장
        char file_relative_path[MAX_LINE];
        memset(file_relative_path, '\0', MAX_LINE);

        // cur folder(상대경로)까지의 경로를 path에 저장
        strcpy(file_relative_path, cur_folder);
        if (strcmp(file_relative_path, "")) //만약 cur_folder까지의 상대경로가 "", 즉 .gitt이 관리하는 가장 상위 폴더인 경우이면, "/"를 넣어줄 필요가 없음
        {
            strcat(file_relative_path, "/");
        }
        strcat(file_relative_path, file->d_name);

        stat(file_relative_path, &file_stat);
        if (S_ISDIR(file_stat.st_mode)) // directory이고 숨김폴더가 아니라면
        {
            if (file->d_name[0] != '.')
            {
                //폴더 안에서 재귀 적으로 실행되도록 함
                gitt_add_dot(file_relative_path, index);
            }
        }
        else // directory가 아니라면, blob파일을 생성하고, index에 저장
        {
            char hashed_str[MAX_LINE];
            memset(hashed_str, '\0', MAX_LINE);
            create_blob_file(hashed_str, file_relative_path, file_stat.st_size);
            //+1을 해주는 이유는 cwd이후에 /가 하나 있기 때문
            fprintf(index, "%s %s\n", hashed_str, file_relative_path);
        }
    }
    closedir(dir);
}

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

int read_index_file_to_hash(struct hash *idx_hash)
{
    FILE *index = fopen(".gitt/index", "r");
    if (index) // INDEX파일이 존재한다면
    {
        int res;

        while (1) // index 파일을 한 줄씩 읽어 idx_hash에 저장
        {
            struct blob_item *blb_item = (struct blob_item *)malloc(sizeof(struct blob_item));
            res = fscanf(index, "%s %s", blb_item->hashed_str, blb_item->file_name);
            if (res == EOF)
                break;

            hash_insert(idx_hash, &(blb_item->elem));
        }
        fclose(index);
        return 1;
    }
    return 0;
}

//./gitt add [filename] .. or ./gitt add .
void gitt_add(int argc, char *argv[])
{
    if (argc == 3 && !strcmp(argv[2], ".")) //./gitt add .인 경우
    {
        FILE *index;
        // index 파일을 만듦
        index = fopen(".gitt/index", "w");
        //현재 폴더안에 모든 파일 및 폴더를 index에 추가
        gitt_add_dot("", index);
        fclose(index);
    }
    else if(argc>=3)
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

        // index파일의 정보를 저장할 hash
        struct hash idx_hash;
        hash_init(&idx_hash, blob_item_hash_func, blob_item_hash_less_func, NULL);
        //index파일로부터 idx_hash로 정보를 읽어옴
        read_index_file_to_hash(&idx_hash);

        // argument로 받은 파일을 hash 자료구조에 추가하거나, 수정
        for (int i = 2; i < argc; i++)
        {

            struct stat file_stat;
            struct blob_item *blb_item = (struct blob_item *)malloc(sizeof(struct blob_item));
            //파일 path 저장
            strcpy(blb_item->file_name, argv[i]);
            //파일의 size 정보를 알기 위해 file_stat에 파일의 정보 저장
            stat(blb_item->file_name, &file_stat);
            // blob파일을 만들고 blob파일에 대한 hashed_str을 저장
            create_blob_file(blb_item->hashed_str, blb_item->file_name, file_stat.st_size);

            struct hash_elem *e = hash_find(&idx_hash, &blb_item->elem);
            if (e) //해당 파일이 이미 idx_hash에 있다면
            {
                //기존의 blob_item의 hashed_str을 새로 생성한 blob_file의 hashed_str로 바꾸어 줌
                struct blob_item *existed_item = hash_entry(e, struct blob_item, elem);
                strcpy(existed_item->hashed_str, blb_item->hashed_str);
                free(blb_item);
            }
            else //없으면 추가해줌
            {
                hash_insert(&idx_hash, &blb_item->elem);
            }
        }

        // hash의 내용을 새로 index파일에 써줌
        FILE *index = fopen(".gitt/index", "w");
        struct hash_iterator it;

        hash_first(&it, &idx_hash);
        while (hash_next(&it))
        {
            struct blob_item *blb_item = hash_entry(hash_cur(&it), struct blob_item, elem);
            fprintf(index, "%s %s\n", blb_item->hashed_str, blb_item->file_name);
        }

        hash_destroy(&idx_hash, blob_item_hash_delete_func);
        fclose(index);
    }
    else
        print_error("파일 이름이 필요합니다.");
}

//./gitt commit [commit message]
void gitt_commit(int argc, char *argv[])
{
    FILE *head;
    char head_path[MAX_LINE];
    //HEAD 파일로부터 현재 브랜치 얻어옴
    head=fopen(".gitt/HEAD", "r");
    fscanf(head, "%s", head_path);

    struct tree_item *t = (struct tree_item *) malloc (sizeof(struct tree_item));
    // index파일 읽어 tree 자료구조 생성
    if(!read_index_file_to_tree(t))
    {
        print_error("staged area를 나타내는 index파일이 존재하지 않습니다. gitt add가 필요합니다.");
        return;
    }

    //현재 head가 가리키고 있는 것이 refs가 아닌 커밋 hash인 경우
    if(strncmp(head_path, ".gitt/refs/heads", 16))
    {
        //기존 head가 가리키고 있는 commit의 tree file과 비교
    }
    //현재 head가 refs를 가리키지만 .gitt/refs/heads에 없는 경우 .gitt/refs/heads에 파일 생성
    else if(!is_file_exist(head_path))
    {
        //tree자료구조 기반으로 commit 메시지 생성
    }
    //있는 경우
    else
    {
        //기존 head가 가리키고있는 commit의 tree file과 비교
    }
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



