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

void gitt_status(int argc, char *argv[])
{
}

void wd_recursive_list(char *cur_folder, struct list *wd_list)
{
    DIR *dir;
    struct dirent *file;
    struct stat file_stat;
    char cur_folder_absolute[MAX_LINE]; // cur folder의 절대 주소 저장

    // curfolder를 통해 curfolder까지의 절대경로를 알아냄
    strcpy(cur_folder_absolute, cwd);
    strcat(cur_folder_absolute, "/");
    strcat(cur_folder_absolute, cur_folder);

    //절대 경로를 통해 cur folder를 open
    dir = opendir(cur_folder_absolute);

    // cur folder 내의 파일들 및 폴더들 순회
    while ((file = readdir(dir)))
    {

        // cur folder에 속한 파일들 및 폴더들 까지의 상대 경로를 저장
        char file_relative_path[MAX_LINE];
        memset(file_relative_path, '\0', MAX_LINE);
        strcpy(file_relative_path, cur_folder);
        strcat(file_relative_path, file->d_name);

        //순회한 파일의 정보를 file_stat에 저장
        stat(file_relative_path, &file_stat);
        if (S_ISDIR(file_stat.st_mode)) // directory이고 숨김폴더가 아니라면
        {
            if (file->d_name[0] != '.')
            {
                //폴더 경로이므로 뒤에 / 붙여줌
                strcat(file_relative_path, "/");
                //폴더 안에서 재귀 적으로 실행되도록 함
                wd_recursive_list(file_relative_path, wd_list);
            }
        }
        else // directory가 아니라면, 파일의 hashed_str을 생성하고, list에 저장
        {
            char hashed_str[MAX_LINE];
            memset(hashed_str, '\0', MAX_LINE);
            // hashed_str 생성
            get_file_hash(hashed_str, file_relative_path);

            // item 생성하여 wd_list에 저장
            struct blob_item *blb_item = (struct blob_item *)malloc(sizeof(struct blob_item));
            strcpy(blb_item->hashed_str, hashed_str);
            strcpy(blb_item->file_name, file_relative_path);
            list_insert_ordered(wd_list, &(blb_item->elem), blob_item_less_func, NULL);
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

int read_index_file_to_list(struct list *idx_list)
{
    FILE *index = fopen(".gitt/index", "r");
    if (index) // INDEX파일이 존재한다면
    {
        int res;

        while (1) // index 파일을 한 줄씩 읽어 idx_list에 저장
        {
            struct blob_item *blb_item = (struct blob_item *)malloc(sizeof(struct blob_item));
            res = fscanf(index, "%s %s", blb_item->hashed_str, blb_item->file_name);
            if (res == EOF)
                break;

            list_insert_ordered(idx_list, &(blb_item->elem), blob_item_less_func, NULL);
        }
        fclose(index);
        return 1;
    }
    return 0;
}

void write_list_to_index_file(struct list *li)
{
    // list의 내용을 새로 index파일에 써주 면서 동적 할당 해제
    FILE *index = fopen(".gitt/index", "w");
    struct list_elem *e;
    for (e = list_begin(li); e != list_end(li);)
    {
        struct blob_item *blb_item = list_entry(e, struct blob_item, elem);
        fprintf(index, "%s %s\n", blb_item->hashed_str, blb_item->file_name);

        e = list_remove(e);
        free(blb_item);
    }

    fclose(index);
}

//./gitt add [filename] .. or ./gitt add .
void gitt_add(int argc, char *argv[])
{
    if (argc == 3 && !strcmp(argv[2], ".")) //./gitt add .인 경우
    {
        // index 파일을 만듦
        FILE *index = fopen(".gitt/index", "w");

        // 현재 woriking_directory의 정보를 저장할 list 초기화
        struct list wd_list;
        list_init(&wd_list);

        //현재 working directory를 wd_list로 정보를 읽어옴
        wd_recursive_list("", &wd_list);

        //wd_list의 정보를 index파일에 써줌
        write_list_to_index_file(&wd_list);

        fclose(index);
    }
    else if (argc >= 3)
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

        // index파일의 정보를 저장할 list
        struct list idx_list;
        list_init(&idx_list);
        // index파일로부터 idx_list로 정보를 읽어옴
        read_index_file_to_list(&idx_list);

        // argument로 받은 파일을 list 자료구조에 추가하거나, 수정
        for (int i = 2; i < argc; i++)
        {

            struct stat file_stat;
            struct blob_item *blb_item = (struct blob_item *)malloc(sizeof(struct blob_item));
            //파일 path 저장
            strcpy(blb_item->file_name, argv[i]);
            //파일의 size 정보를 알기 위해 file_stat에 파일의 정보 저장
            stat(blb_item->file_name, &file_stat);
            // blob파일을 만들고 blob파일에 대한 hashed_str을 저장
            create_blob_file(blb_item->hashed_str, blb_item->file_name);

            struct list_elem *e = list_find(&idx_list, &blb_item->elem, blob_item_less_func);
            if (e) //해당 파일이 이미 idx_hash에 있다면
            {
                //기존의 blob_item의 hashed_str을 새로 생성한 blob_file의 hashed_str로 바꾸어 줌
                struct blob_item *existed_item = list_entry(e, struct blob_item, elem);
                strcpy(existed_item->hashed_str, blb_item->hashed_str);
                free(blb_item);
            }
            else //없으면 추가해줌
            {
                list_insert_ordered(&idx_list, &blb_item->elem, blob_item_less_func, NULL);
            }
        }

        write_list_to_index_file(&idx_list);
    }
    else
        print_error("파일 이름이 필요합니다.");
}

//./gitt commit [commit message]
void gitt_commit(int argc, char *argv[])
{
    FILE *head;
    char head_path[MAX_LINE];
    // HEAD 파일로부터 현재 브랜치 얻어옴
    head = fopen(".gitt/HEAD", "r");
    fscanf(head, "%s", head_path);
    fclose(head);
    if (argc <= 2)
    {
        print_error("commit message가 필요합니다.");
        return;
    }

    char commit_msg[MAX_LINE];
    int i;
    // commit message 생성
    memset(commit_msg, '\0', MAX_LINE);
    for (i = 2; i < argc; i++)
    {
        strcat(commit_msg, argv[i]);
        if (i != argc - 1)
            strcat(commit_msg, " ");
    }

    struct tree_item *t = (struct tree_item *)malloc(sizeof(struct tree_item));
    // index파일 읽어 tree 자료구조 생성
    if (!read_index_file_to_tree(t))
    {
        print_error("staged area를 나타내는 index파일이 존재하지 않습니다. gitt add가 필요합니다.");
        free(t);
        return;
    }

    //현재 head가 가리키고 있는 것이 refs가 아닌 커밋 hash인 경우
    if (strncmp(head_path, ".gitt/refs/heads", 16))
    {
        //기존 head가 가리키고 있는 commit의 hash를 parent로 읽어옴
        char parent[MAX_LINE];
        strcpy(parent, head_path);

        // parent를 이용해 head가 가리키고 있는 파일 경로를 얻고
        char commit_path[MAX_LINE];
        make_object_path(commit_path, parent);

        // parent commit의 tree 정보를 가져옴
        FILE *commit_file = fopen(commit_path, "r");
        if (!commit_file)
        {
            print_error("commit file이 존재하지 않습니다.");
            free_all_sub_trees_and_blobs(t);
            free(t);
            return;
        }
        char temp[MAX_LINE];            //앞의 "tree" 저장
        char tree_hashed_str[MAX_LINE]; // commit이 가리키는 tree의 hashed_str저장
        fscanf(commit_file, "%s %s", temp, tree_hashed_str);
        fclose(commit_file);

        //현재 index를  통해 만든 tree와 기존의 commit이 나타내는 tree가 달라 commit이 가능한 경우
        if (strcmp(t->hashed_str, tree_hashed_str))
        {
            //현재 head를 새로 갱신
            FILE *head = fopen(".gitt/HEAD", "w");
            // tree 자료구조 기반으로 새로운  commit 파일 생성
            char hashed_str[MAX_LINE];
            create_commit_file(hashed_str, t, commit_msg, parent);
            //현재 head가 가리키는 파일에 commit에 대한 hashed string 기록
            fprintf(head, "%s", hashed_str);
            fclose(head);
        }
        else
        {
            print_error("commit을 위한 변동사항이 없습니다.");
            free_all_sub_trees_and_blobs(t);
            free(t);
            return;
        }
    }
    //현재 head가 refs(branch)를 가리키지만 .gitt/refs/heads에 없는 경우(첫 커밋인 경우), .gitt/refs/heads에 해당 refs(branch) 파일 생성
    else if (!is_file_exist(head_path))
    {
        //현재 branch에 해당하는 파일을 만들어줌
        FILE *now_branch = fopen(head_path, "w");
        // tree자료구조 기반으로 commit 파일 생성
        char hashed_str[MAX_LINE];
        create_commit_file(hashed_str, t, commit_msg, NULL);
        //현재 branch 파일에 commit에 대한 hashed string 기록
        fprintf(now_branch, "%s", hashed_str);
        fclose(now_branch);
    }
    //있는 경우
    else
    {
        //현재 head가 가리키는 파일
        FILE *now_branch = fopen(head_path, "r");

        //기존 head가 가리키고 있는 commit을 읽어옴
        char parent[MAX_LINE];
        fscanf(now_branch, "%s", parent);
        fclose(now_branch);

        // parent를 이용해 head가 가리키고 있는 파일 경로를 얻고
        char commit_path[MAX_LINE];
        make_object_path(commit_path, parent);

        // parent commit의 tree 정보를 가져옴
        FILE *commit_file = fopen(commit_path, "r");
        if (!commit_file)
        {
            print_error("commit file이 존재하지 않습니다.");
            free_all_sub_trees_and_blobs(t);
            free(t);
            return;
        }
        char temp[MAX_LINE];            //앞의 "tree" 저장
        char tree_hashed_str[MAX_LINE]; // commit이 가리키는 tree의 hashed_str저장
        fscanf(commit_file, "%s %s", temp, tree_hashed_str);
        fclose(commit_file);

        //현재 index를  통해 만든 tree와 기존의 commit이 나타내는 tree가 달라 commit이 가능한 경우
        if (strcmp(t->hashed_str, tree_hashed_str))
        {
            //현재 head가 가리키는 파일을 새로 갱신
            FILE *now_branch = fopen(head_path, "w");
            // tree 자료구조 기반으로 새로운  commit 파일 생성
            char hashed_str[MAX_LINE];
            create_commit_file(hashed_str, t, commit_msg, parent);
            //현재 head가 가리키는 파일에 commit에 대한 hashed string 기록
            fprintf(now_branch, "%s", hashed_str);
            fclose(now_branch);
        }
        else
        {
            print_error("commit을 위한 변동사항이 없습니다.");
            free_all_sub_trees_and_blobs(t);
            free(t);
            return;
        }
    }
    free_all_sub_trees_and_blobs(t);
    free(t);
}
//./gitt branch [branch name]
void gitt_branch(int argc, char *argv[])
{
    FILE *head;
    char head_path[MAX_LINE];
    // HEAD 파일로부터 현재 브랜치 얻어옴
    head = fopen(".gitt/HEAD", "r");
    fscanf(head, "%s", head_path);
    fclose(head);

    //./gitt branch만 들어온 경우 현재 branch 출력
    if (argc == 2)
    {
        // head가 branch를 가리키고 있는 경우
        if (!strncmp(head_path, ".gitt/refs/heads", 16))
        {
            char now_branch_name[MAX_LINE];
            strcpy(now_branch_name, head_path + 17);
            printf("current branch: %s\n", now_branch_name);
        }
        else // head가 커밋 해시를 가리키고 있는 경우
        {
            printf("current head: %s [head가 branch를 가리키고 있지 않음]\n", head_path);
        }
        return;
    }

    if (argc != 3)
    {
        print_error("올바른 branch name이 필요합니다.");
        return;
    }

    // branch 이름과 저장할 path를 생성
    char branch_name[MAX_LINE];
    char branch_path[MAX_LINE];
    strcpy(branch_name, argv[2]);
    strcpy(branch_path, ".gitt/refs/heads/");
    strcat(branch_path, branch_name);

    //해당 branch가 이미 존재하는 브랜치라면
    if (is_file_exist(branch_path))
    {
        print_error("이미 존재하는 branch입니다.");
        return;
    }

    //현재 head가 가리키고 있는 것이 refs가 아닌 커밋 hash인 경우
    if (strncmp(head_path, ".gitt/refs/heads", 16))
    {
        //새로운 브랜치 파일을 생성하고 해당 파일에 커밋 hash내용을 적어줌
        FILE *branch = fopen(branch_path, "w");
        fprintf(branch, "%s", head_path);
        fclose(branch);
    }
    //현재 head가 refs(branch)를 가리키지만 .gitt/refs/heads에 파일로써 존재하지 않는 경우 오류 출력
    else if (!is_file_exist(head_path))
    {
        print_error("새로운 브랜치를 생성할 수 없습니다, 기존 branch의 파일이 존재하지 않습니다.");
    }
    //있는 경우
    else
    {
        //현재 head가 가리키는 파일
        FILE *now_branch = fopen(head_path, "r");

        //기존 branch가 가리키고 있는 commit hash를 읽어옴
        char commit_hash[MAX_LINE];
        fscanf(now_branch, "%s", commit_hash);
        fclose(now_branch);

        //새로운 브랜치 파일을 생성하고 해당 파일에 커밋 hash내용을 적어줌
        FILE *new_branch = fopen(branch_path, "w");
        fprintf(new_branch, "%s", commit_hash);
        fclose(new_branch);
    }
}

void delete_all_files_in_folder(char *cur_folder)
{
    DIR *dir;
    struct dirent *file;
    struct stat file_stat;
    char cur_folder_absolute[MAX_LINE]; //절대 주소 저장

    // cur folder를 통해 cur folder까지의 절대경로를 알아냄
    strcpy(cur_folder_absolute, cwd);
    strcat(cur_folder_absolute, "/");
    strcat(cur_folder_absolute, cur_folder);

    //절대 경로를 통해 cur folder를 open
    dir = opendir(cur_folder_absolute);

    // cur folder 내의 파일들 및 폴더들 순회
    while ((file = readdir(dir)))
    {

        // cur folder에 속한 파일들 및 폴더들 까지의 상대 경로를 저장
        char file_relative_path[MAX_LINE];
        memset(file_relative_path, '\0', MAX_LINE);
        strcpy(file_relative_path, cur_folder);
        strcat(file_relative_path, file->d_name);

        //순회한 파일의 정보를 file_stat에 저장
        stat(file_relative_path, &file_stat);
        if (S_ISDIR(file_stat.st_mode)) // directory이고 숨김폴더가 아니라면
        {
            if (file->d_name[0] != '.')
            {
                //폴더 경로이므로 뒤에 / 붙여줌
                strcat(file_relative_path, "/");
                //폴더 안에서 재귀적으로 파일들을 삭제하도록 함
                delete_all_files_in_folder(file_relative_path);
                //폴더 안의 파일들을 다 삭제한 후에 폴더 삭제
                rmdir(file_relative_path);
            }
        }
        else //파일이라면 해당 파일 삭제
        {
            remove(file_relative_path);
        }
    }
    closedir(dir);
}

void create_files_using_tree_hash(char *tree_hash, char *relative_path)
{

    char class[10], hash[MAX_LINE], file_name[MAX_LINE];

    // tree hash를 이용하여 tree 파일에 대한 path 생성
    char tree_path[MAX_LINE];
    make_object_path(tree_path, tree_hash);

    // tree파일에 속한 파일들의 내용을 읽어옴
    FILE *tree_file = fopen(tree_path, "r");
    while (1)
    {
        //한 줄씩 읽으면서,tree파일에 속한 파일의 종류, 해시, 파일 이름을 저장
        int res = fscanf(tree_file, "%s %s %s", class, hash, file_name);
        if (res == EOF) //파일의 끝
            break;

        if (!strcmp(class, "tree")) //속한 파일이 tree(폴더)인 경우
        {
            //속한 폴더에 대한 상대경로 생성
            char folder_relative_path[MAX_LINE];
            strcpy(folder_relative_path, relative_path);
            strcat(folder_relative_path, file_name);
            strcat(folder_relative_path, "/");

            //상대경로를 이용하여 속한 폴더에 대한 실제 폴더 생성
            mkdir(folder_relative_path, 0755);

            //속한 폴더에 대한 tree_hash와 path를 통해 파일들을 재귀적으로 생성
            create_files_using_tree_hash(hash, folder_relative_path);
        }
        if (!strcmp(class, "blob")) //속한 파일이 blob(파일)인 경우
        {
            //속한 파일에 대한 상대경로 생성
            char file_relative_path[MAX_LINE];
            strcpy(file_relative_path, relative_path);
            strcat(file_relative_path, file_name);

            // hash를 이용해 blob파일에 대한 path 생성
            char blob_path[MAX_LINE];
            make_object_path(blob_path, hash);

            // blob path를 통해 얻은 파일을, 속한파일에 대한 상대경로로 카피
            file_copy(blob_path, file_relative_path);
        }
    }
    fclose(tree_file);
}

void file_copy(char *from_path, char *to_path)
{
    FILE *from = fopen(from_path, "r");
    FILE *to = fopen(to_path, "w");
    char c;
    while (1)
    {
        c = fgetc(from);
        if (feof(from))
            break;
        fputc(c, to);
    }

    fclose(from);
    fclose(to);

    //파일 권한 복사
    struct stat file_stat;
    stat(from_path, &file_stat);
    chmod(to_path, file_stat.st_mode);
}

void update_files(char *commit_hash)
{
    char commit_path[MAX_LINE];
    char temp[MAX_LINE];
    char tree_hash[MAX_LINE];

    //숨김폴더를 제외한 모든 폴더랑 파일을 삭제
    delete_all_files_in_folder("");

    // commit hash를 이용해서 commit_path를 생성
    make_object_path(commit_path, commit_hash);

    // commit_path를 이용해 해당 커밋의 tree를 읽어옴
    FILE *commit_file = fopen(commit_path, "r");
    // temp에는 tree가 저장되고, tree_hash에 commit의 tree에 대한 hash값이 저장됨
    fscanf(commit_file, "%s %s", temp, tree_hash);
    fclose(commit_file);

    // commit의 tree hash를 이용해 파일들을 재귀적으로 탐색하며 생성
    create_files_using_tree_hash(tree_hash, "");
}

//./gitt checkout [branch name] or [commit hash]
void gitt_checkout(int argc, char *argv[])
{

    if (argc != 3)
    {
        print_error("올바른 branch name이나, commit hash가 필요합니다.");
        return;
    }

    //가장 먼저 branch라고 가정하여, branch path를 생성
    char branch_name[MAX_LINE];
    char branch_path[MAX_LINE];
    strcpy(branch_name, argv[2]);
    strcpy(branch_path, ".gitt/refs/heads/");
    strcat(branch_path, branch_name);

    //해당 branch가 존재하면
    if (is_file_exist(branch_path))
    {
        //해당 branch로 checkout해줌
        FILE *head = fopen(".gitt/HEAD", "w");
        fprintf(head, "%s", branch_path);
        fclose(head);

        //해당 branch의 commit_hash 읽어옴
        char commit_hash[MAX_LINE];
        FILE *branch = fopen(branch_path, "r");
        fscanf(branch, "%s", commit_hash);
        fclose(branch);

        //해당 branch의 commit_hash에 해당하는 내용으로 working directory를 바꾸어줌
        update_files(commit_hash);

        printf("checkout 완료: %s\n", branch_name);
        return;
    }

    //그 다음으로는 commit hash라고 가정하고, commit hash를 통해 commit_file의 path를 생성
    char commit_hash[MAX_LINE];
    char commit_path[MAX_LINE];
    strcpy(commit_hash, argv[2]);
    make_object_path(commit_path, commit_hash);

    // commit file에 대한 path에 파일이 존재하면
    if (is_file_exist(commit_path))
    {
        //해당 commit hash로 checkout해줌
        FILE *head = fopen(".gitt/HEAD", "w");
        fprintf(head, "%s", commit_hash);
        fclose(head);

        //해당 commit_hash에 해당하는 내용으로 working directory를 바꾸어줌
        update_files(commit_hash);

        printf("checkout 완료: %s\n", commit_hash);
        return;
    }

    print_error("존재하지 않는 branch이거나 commit hash입니다.");
}

void print_error(char *msg)
{
    printf("ERROR: %s\n", msg);
}
