#include "utils.h"
#include <dirent.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>  // 新增：用于文件类型判断

char** load_stop_words(const char *filename, int *count) {
    *count = 0;
    char **stop_words = NULL;
    
    FILE *file = fopen(filename, "r");
    if (!file) return NULL;
    
    char buffer[100];
    while (fgets(buffer, sizeof(buffer), file)) {
        // 移除换行符
        buffer[strcspn(buffer, "\n")] = '\0';
        
        // 转换为小写
        for (int i = 0; buffer[i]; i++) {
            buffer[i] = tolower(buffer[i]);
        }
        
        *count += 1;
        stop_words = (char**)realloc(stop_words, *count * sizeof(char*));
        stop_words[*count - 1] = (char*)malloc(strlen(buffer) + 1);
        strcpy(stop_words[*count - 1], buffer);
    }
    
    fclose(file);
    return stop_words;
}

int is_stop_word(char **stop_words, int count, const char *word) {
    for (int i = 0; i < count; i++) {
        if (strcmp(stop_words[i], word) == 0) {
            return 1;
        }
    }
    return 0;
}

// 简单的分词函数
static char** tokenize_document(const char *content, int *token_count, char **stop_words, int stop_word_count) {
    *token_count = 0;
    char **tokens = NULL;
    
    if (!content || strlen(content) == 0) {
        return NULL;
    }
    
    // 创建内容副本以便修改
    char *content_copy = (char*)malloc(strlen(content) + 1);
    strcpy(content_copy, content);
    
    // 转换为小写并替换非字母字符为空格
    for (int i = 0; content_copy[i]; i++) {
        if (isalpha(content_copy[i])) {
            content_copy[i] = tolower(content_copy[i]);
        } else {
            content_copy[i] = ' ';
        }
    }
    
    // 分词
    char *token = strtok(content_copy, " ");
    while (token) {
        // 过滤短词和停用词
        if (strlen(token) > 1 && !is_stop_word(stop_words, stop_word_count, token)) {
            *token_count += 1;
            tokens = (char**)realloc(tokens, *token_count * sizeof(char*));
            tokens[*token_count - 1] = (char*)malloc(strlen(token) + 1);
            strcpy(tokens[*token_count - 1], token);
        }
        token = strtok(NULL, " ");
    }
    
    free(content_copy);
    return tokens;
}

// 读取文件内容
static char* read_file_content(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) return NULL;
    
    // 获取文件大小
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    // 分配内存并读取内容
    char *content = (char*)malloc(length + 1);
    if (content) {
        fread(content, 1, length, file);
        content[length] = '\0';
    }
    
    fclose(file);
    return content;
}

void build_index_from_docs(const char *doc_dir, TrieNode *trie, InvertedIndex *index, 
                          char ***doc_paths, int *num_docs) {
    *num_docs = 0;
    *doc_paths = NULL;
    
    DIR *dir = opendir(doc_dir);
    if (!dir) return;
    
    // 加载停用词
    int stop_word_count;
    char **stop_words = load_stop_words("stop_words.txt", &stop_word_count);
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // 构建完整路径（仅定义一次）
        char *full_path = (char*)malloc(strlen(doc_dir) + strlen(entry->d_name) + 2);
        strcpy(full_path, doc_dir);
        strcat(full_path, "/");
        strcat(full_path, entry->d_name);
        
        // 检查是否为文件（而非目录）和隐藏文件
        struct stat path_stat;
        stat(full_path, &path_stat);
        if (!S_ISREG(path_stat.st_mode) || entry->d_name[0] == '.') {
            free(full_path);  // 释放内存
            continue;
        }
        
        // 读取文件内容
        char *content = read_file_content(full_path);
        if (!content) {
            free(full_path);
            continue;
        }
        
        // 分词
        int token_count;
        char **tokens = tokenize_document(content, &token_count, stop_words, stop_word_count);
        
        // 添加到Trie树和倒排索引
        for (int i = 0; i < token_count; i++) {
            trie_insert(trie, tokens[i]);
            inverted_index_add_term(index, tokens[i], *num_docs);
            
            free(tokens[i]);
        }
        free(tokens);
        
        // 保存文档路径
        *num_docs += 1;
        *doc_paths = (char**)realloc(*doc_paths, *num_docs * sizeof(char*));
        (*doc_paths)[*num_docs - 1] = full_path;  // 使用之前定义的full_path
        
        free(content);
    }
    
    // 清理
    for (int i = 0; i < stop_word_count; i++) {
        free(stop_words[i]);
    }
    free(stop_words);
    
    closedir(dir);
}

void save_doc_paths(char **doc_paths, int num_docs, const char *filename) {
    if (!doc_paths || num_docs <= 0 || !filename) return;
    
    FILE *file = fopen(filename, "wb");
    if (!file) return;
    
    fwrite(&num_docs, sizeof(int), 1, file);
    
    for (int i = 0; i < num_docs; i++) {
        int len = strlen(doc_paths[i]);
        fwrite(&len, sizeof(int), 1, file);
        fwrite(doc_paths[i], sizeof(char), len, file);
    }
    
    fclose(file);
}

char** load_doc_paths(const char *filename, int *num_docs) {
    *num_docs = 0;
    if (!filename) return NULL;
    
    FILE *file = fopen(filename, "rb");
    if (!file) return NULL;
    
    fread(num_docs, sizeof(int), 1, file);
    if (*num_docs <= 0) {
        fclose(file);
        return NULL;
    }
    
    char **doc_paths = (char**)malloc(*num_docs * sizeof(char*));
    if (!doc_paths) {
        fclose(file);
        return NULL;
    }
    
    for (int i = 0; i < *num_docs; i++) {
        int len;
        fread(&len, sizeof(int), 1, file);
        
        doc_paths[i] = (char*)malloc(len + 1);
        fread(doc_paths[i], sizeof(char), len, file);
        doc_paths[i][len] = '\0';
    }
    
    fclose(file);
    return doc_paths;
}

// 递归保存Trie树
static void trie_save_recursive(TrieNode *node, FILE *file) {
    if (!node || !file) return;
    
    // 保存节点信息
    fwrite(&node->is_end_of_word, sizeof(bool), 1, file);
    
    // 保存子节点
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        int has_child = (node->children[i] != NULL) ? 1 : 0;
        fwrite(&has_child, sizeof(int), 1, file);
        
        if (has_child) {
            trie_save_recursive(node->children[i], file);
        }
    }
}

void trie_save(TrieNode *root, const char *filename) {
    if (!root || !filename) return;
    
    FILE *file = fopen(filename, "wb");
    if (!file) return;
    
    trie_save_recursive(root, file);
    fclose(file);
}

// 递归加载Trie树
static TrieNode* trie_load_recursive(FILE *file) {
    if (!file) return NULL;
    
    TrieNode *node = trie_create_node();
    if (!node) return NULL;
    
    // 加载节点信息
    fread(&node->is_end_of_word, sizeof(bool), 1, file);
    
    // 加载子节点
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        int has_child;
        fread(&has_child, sizeof(int), 1, file);
        
        if (has_child) {
            node->children[i] = trie_load_recursive(file);
        } else {
            node->children[i] = NULL;
        }
    }
    
    return node;
}

TrieNode* trie_load(const char *filename) {
    if (!filename) return NULL;
    
    FILE *file = fopen(filename, "rb");
    if (!file) return NULL;
    
    TrieNode *root = trie_load_recursive(file);
    fclose(file);
    
    return root;
}