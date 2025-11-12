#ifndef SEARCH_H
#define SEARCH_H

#include "trie.h"
#include "inverted_index.h"
#include "tfidf.h"

// 搜索结果结构
typedef struct SearchResult {
    int doc_id;
    double score;
    char *doc_path; // 文档路径
} SearchResult;

// 执行搜索
SearchResult* perform_search(TrieNode *trie, InvertedIndex *index, const char *query, 
                            char **doc_paths, int num_docs, int *result_count);

// 释放搜索结果
void free_search_results(SearchResult *results, int count);

#endif