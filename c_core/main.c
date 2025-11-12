#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "trie.h"
#include "inverted_index.h"
#include "search.h"
#include "utils.h"
#ifdef _WIN32
#include <windows.h>
#endif


#define INDEX_DIR "../python_preprocess/index_data"
#define BUFFER_SIZE 1024

// 创建索引目录（简单兼容Windows）
void create_index_dir() {
    #ifdef _WIN32
        system("mkdir index_data 2>NUL");
    #else
        system("mkdir -p index_data");
    #endif
}

// 构建索引（使用相对路径）
void build_index(const char *doc_dir) {
    printf("正在构建索引...\n");
    
    // 创建索引目录
    create_index_dir();
    
    // 初始化数据结构
    TrieNode *trie = trie_create_node();
    const int NUM_BUCKETS = 10007;
    int num_docs = 0;
    char **doc_paths = NULL;
    InvertedIndex *index = inverted_index_create(NUM_BUCKETS, num_docs);
    
    // 从文档目录构建索引
    build_index_from_docs(doc_dir, trie, index, &doc_paths, &num_docs);
    index->num_docs = num_docs;
    
    // 保存索引到index_data目录（相对路径）
    trie_save(trie, INDEX_DIR "/trie.dat");
    inverted_index_save(index, INDEX_DIR "/inverted_index.dat");
    save_doc_paths(doc_paths, num_docs, INDEX_DIR "/doc_paths.dat");
    
    // 释放内存
    trie_free(trie);
    inverted_index_free(index);
    for (int i = 0; i < num_docs; i++) free(doc_paths[i]);
    free(doc_paths);
    
    printf("索引构建完成，共处理 %d 个文档\n", num_docs);
}

// 加载索引（使用相对路径）
void load_index(TrieNode **trie, InvertedIndex **index, char ***doc_paths, int *num_docs) {
    printf("正在加载索引...\n");
    
    // 从index_data目录加载索引（相对路径）
    *trie = trie_load(INDEX_DIR "/trie.dat");
    *index = inverted_index_load(INDEX_DIR "/inverted_index.dat");
    *doc_paths = load_doc_paths(INDEX_DIR "/doc_paths.dat", num_docs);
    
    // 检查是否加载成功
    if (!*trie || !*index || !*doc_paths || *num_docs <= 0) {
        printf("索引加载失败！请先构建索引。\n");
        printf("请确保index_data目录下有以下文件：\n");
        printf("- trie.dat\n- inverted_index.dat\n- doc_paths.dat\n");
        exit(1);
    }
    
    printf("索引加载完成，共 %d 个文档\n", *num_docs);
}

// 交互式搜索功能
void interactive_search(TrieNode *trie, InvertedIndex *index, char **doc_paths, int num_docs) {
    char query[BUFFER_SIZE];
    printf("\n进入搜索模式，输入查询词（输入q退出）：\n");
    
    while (1) {
        printf("搜索: ");
        if (fgets(query, BUFFER_SIZE, stdin) == NULL) break;
        
        // 移除换行符
        query[strcspn(query, "\n")] = '\0';
        
        // 退出条件
        if (strcmp(query, "q") == 0 || strcmp(query, "Q") == 0) break;
        
        // 执行搜索并显示结果
        int result_count;
        SearchResult *results = perform_search(trie, index, query, doc_paths, num_docs, &result_count);
        
        printf("\n找到 %d 个结果：\n", result_count);
        for (int i = 0; i < result_count; i++) {
            // 修正：输出格式标准化（便于Python解析，去除多余空格）
            printf("%d. 文档: %s (分数: %.4f)\n", 
                   i + 1, results[i].doc_path, results[i].score);
        }
        
        free_search_results(results, result_count);
    }
}

int main(int argc, char *argv[]) {
    #ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8); // 确保中文输出正常（若有）
    #endif

    // 检查参数：支持3种模式（构建索引、交互搜索、命令行搜索）
    if (argc == 2) {
        // 模式1：构建索引（参数为文档目录）
        if (strcmp(argv[1], "search") != 0) {
            build_index(argv[1]);
        } 
        // 模式2：交互搜索（参数为"search"）
        else {
            TrieNode *trie = NULL;
            InvertedIndex *index = NULL;
            char **doc_paths = NULL;
            int num_docs = 0;
            
            load_index(&trie, &index, &doc_paths, &num_docs);
            interactive_search(trie, index, doc_paths, num_docs);
            
            // 释放资源
            trie_free(trie);
            inverted_index_free(index);
            for (int i = 0; i < num_docs; i++) free(doc_paths[i]);
            free(doc_paths);
        }
    }
    // 模式3：命令行搜索（参数为"search" + 查询词，供Python调用）
    else if (argc == 3 && strcmp(argv[1], "search") == 0) {
        TrieNode *trie = NULL;
        InvertedIndex *index = NULL;
        char **doc_paths = NULL;
        int num_docs = 0;
        const char *query = argv[2];
        
        load_index(&trie, &index, &doc_paths, &num_docs);
        
        // 执行搜索并按标准化格式输出
        int result_count;
        SearchResult *results = perform_search(trie, index, query, doc_paths, num_docs, &result_count);
        printf("找到 %d 个结果：\n", result_count);
        for (int i = 0; i < result_count; i++) {
            printf("%d. 文档: %s (分数: %.4f)\n", 
                   i + 1, results[i].doc_path, results[i].score);
        }
        
        // 释放资源
        free_search_results(results, result_count);
        trie_free(trie);
        inverted_index_free(index);
        for (int i = 0; i < num_docs; i++) free(doc_paths[i]);
        free(doc_paths);
    }
    else {
        printf("用法：\n");
        printf("  构建索引：%s <文档目录路径>\n", argv[0]);
        printf("  交互搜索：%s search\n", argv[0]);
        printf("  命令行搜索：%s search <查询词>\n", argv[0]);
        return 1;
    }
    
    return 0;
}