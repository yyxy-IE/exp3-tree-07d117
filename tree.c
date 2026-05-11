#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

typedef struct Node {
    char name[256];
    int is_dir;
    struct Node* children[100];
    int child_count;
} Node;

Node* create_node(const char* name, int is_dir) {
    Node* node = (Node*)malloc(sizeof(Node));
    strncpy(node->name, name, sizeof(node->name)-1);
    node->name[sizeof(node->name)-1] = '\0';
    node->is_dir = is_dir;
    node->child_count = 0;
    return node;
}

void build_tree(const char* path, Node* parent) {
    DIR* dir = opendir(path);
    if (!dir) return;
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
        char full_path[512];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        struct stat st;
        stat(full_path, &st);
        int is_dir = S_ISDIR(st.st_mode);
        Node* child = create_node(entry->d_name, is_dir);
        parent->children[parent->child_count++] = child;
        if (is_dir) build_tree(full_path, child);
    }
    closedir(dir);
}

void print_tree(Node* node, const char* prefix, int is_last) {
    printf("%s%s%s%s\n", prefix, is_last ? "`-- " : "|-- ", node->name, node->is_dir ? "/" : "");
    char new_prefix[256];
    snprintf(new_prefix, sizeof(new_prefix), "%s%s", prefix, is_last ? "    " : "|   ");
    for (int i = 0; i < node->child_count; i++) {
        print_tree(node->children[i], new_prefix, i == node->child_count - 1);
    }
}

void count_stats(Node* node, int depth, int* dir_cnt, int* file_cnt, int* total, int* leaf, int* height) {
    (*total)++;
    if (node->is_dir) {
        (*dir_cnt)++;
    } else {
        (*file_cnt)++;
    }
    if (node->child_count == 0) {
        (*leaf)++;
    }
    if (depth > *height) {
        *height = depth;
    }
    for (int i = 0; i < node->child_count; i++) {
        count_stats(node->children[i], depth+1, dir_cnt, file_cnt, total, leaf, height);
    }
}

// 补上free_tree函数，修复编译错误
void free_tree(Node* node) {
    if (node == NULL) return;
    for (int i = 0; i < node->child_count; i++) {
        free_tree(node->children[i]);
    }
    free(node);
}

int compare_nodes(const void* a, const void* b) {
    Node* na = *(Node**)a;
    Node* nb = *(Node**)b;
    if (na->is_dir != nb->is_dir) {
        return nb->is_dir - na->is_dir;
    }
    return strcmp(na->name, nb->name);
}

int main() {
    const char* root_path = "C:\\Users\\Lenovo\\Desktop\\test";
    DIR* dir = opendir(root_path);
    if (!dir) { 
        printf("错误：路径不存在！\n"); 
        return 1; 
    }
    closedir(dir);

    Node* root = create_node("test", 1);
    build_tree(root_path, root);

    qsort(root->children, root->child_count, sizeof(Node*), compare_nodes);

    printf("%s/\n", root->name);
    for (int i = 0; i < root->child_count; i++) {
        print_tree(root->children[i], "", i == root->child_count - 1);
    }

    int dir_cnt=0, file_cnt=0, total=0, leaf=0, height=0;
    count_stats(root, 0, &dir_cnt, &file_cnt, &total, &leaf, &height);

    printf("\n%d 个目录，%d 个文件\n", dir_cnt-1, file_cnt);
    printf("二叉树结点总数：%d\n", total);
    printf("叶子结点数：%d\n", leaf);
    printf("树的高度：%d\n", height);

    free_tree(root);
    return 0;
}