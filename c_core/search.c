#include "search.h"
#include <string.h>
#include <ctype.h>

// 简单的分词函数
static char** tokenize_query(const char *query, int *token_count) {
    *token_count = 0;
    char **tokens = NULL;
    
    if (!query || strlen(query) == 0) {
        return NULL;
    }
    
    // 创建查询副本以便修改
    char *query_copy = (char*)malloc(strlen(query) + 1);
    strcpy(query_copy, query);
    
    // 转换为小写
    for (int i = 0; query_copy[i]; i++) {
        query_copy[i] = tolower(query_copy[i]);
    }
    
    // 分词（分隔符包含常见标点）
    char *token = strtok(query_copy, " \t\n\r.,;:!?()[]{}");
    while (token) {
        // 过滤空字符串
        if (strlen(token) > 0) {
            *token_count += 1;
            tokens = (char**)realloc(tokens, *token_count * sizeof(char*));
            tokens[*token_count - 1] = (char*)malloc(strlen(token) + 1);
            strcpy(tokens[*token_count - 1], token);
        }
        token = strtok(NULL, " \t\n\r.,;:!?()[]{}");
    }
    
    free(query_copy);
    return tokens;
}

// 辅助函数：检查词是否已在扩展列表中（优化效率）
static int is_term_in_list(char **list, int list_len, const char *term) {
    for (int i = 0; i < list_len; i++) {
        if (strcmp(list[i], term) == 0) {
            return 1;
        }
    }
    return 0;
}

SearchResult* perform_search(TrieNode *trie, InvertedIndex *index, const char *query, 
                            char **doc_paths, int num_docs, int *result_count) {
    *result_count = 0;
    if (!trie || !index || !query || !doc_paths || num_docs <= 0) {
        return NULL;
    }
    
    // 1. 分词
    int token_count;
    char **tokens = tokenize_query(query, &token_count);
    
    if (token_count == 0) {
        printf("查询词不能为空！\n");
        return NULL;
    }
    
    // 2. 处理前缀匹配（扩展查询词）
    char **expanded_terms = NULL;
    int expanded_count = 0;
    
    for (int i = 0; i < token_count; i++) {
        char **matches;
        int match_count;
        
        // 步骤1：添加完整词（若存在）
        if (trie_search(trie, tokens[i]) && !is_term_in_list(expanded_terms, expanded_count, tokens[i])) {
            expanded_count++;
            expanded_terms = (char**)realloc(expanded_terms, expanded_count * sizeof(char*));
            expanded_terms[expanded_count - 1] = (char*)malloc(strlen(tokens[i]) + 1);
            strcpy(expanded_terms[expanded_count - 1], tokens[i]);
        }
        
        // 步骤2：添加前缀匹配词（去重）
        trie_get_prefix_matches(trie, tokens[i], &matches, &match_count);
        for (int j = 0; j < match_count; j++) {
            if (!is_term_in_list(expanded_terms, expanded_count, matches[j])) {
                expanded_count++;
                expanded_terms = (char**)realloc(expanded_terms, expanded_count * sizeof(char*));
                expanded_terms[expanded_count - 1] = (char*)malloc(strlen(matches[j]) + 1);
                strcpy(expanded_terms[expanded_count - 1], matches[j]);
            }
        }
        
        // 释放前缀匹配的临时内存
        for (int j = 0; j < match_count; j++) {
            free(matches[j]);
        }
        free(matches);
    }
    
    // 3. 若无扩展词，使用原始词
    if (expanded_count == 0) {
        expanded_terms = tokens;
        expanded_count = token_count;
        tokens = NULL; // 避免双重释放
    }
    
    // 4. 计算文档分数
    DocScore *doc_scores = calculate_document_scores(index, expanded_terms, expanded_count, result_count);
    
    // 5. 排序（降序）
    if (*result_count > 0) {
        sort_doc_scores(doc_scores, *result_count);
    } else {
        printf("未找到与\"%s\"匹配的文档\n", query);
        // 清理内存
        for (int i = 0; i < expanded_count; i++) free(expanded_terms[i]);
        free(expanded_terms);
        for (int i = 0; i < token_count; i++) free(tokens[i]);
        free(tokens);
        return NULL;
    }
    
    // 6. 准备搜索结果（复制文档路径）
    SearchResult *results = (SearchResult*)malloc(*result_count * sizeof(SearchResult));
    for (int i = 0; i < *result_count; i++) {
        results[i].doc_id = doc_scores[i].doc_id;
        results[i].score = doc_scores[i].score;
        
        // 验证文档ID有效性（避免越界）
        if (doc_scores[i].doc_id >= 0 && doc_scores[i].doc_id < num_docs) {
            results[i].doc_path = (char*)malloc(strlen(doc_paths[doc_scores[i].doc_id]) + 1);
            strcpy(results[i].doc_path, doc_paths[doc_scores[i].doc_id]);
        } else {
            results[i].doc_path = (char*)malloc(sizeof("无效文档路径"));
            strcpy(results[i].doc_path, "无效文档路径");
        }
    }
    
    // 清理内存
    free(doc_scores);
    for (int i = 0; i < expanded_count; i++) free(expanded_terms[i]);
    free(expanded_terms);
    for (int i = 0; i < token_count; i++) free(tokens[i]);
    free(tokens);
    
    return results;
}

void free_search_results(SearchResult *results, int count) {
    if (!results) return;
    
    for (int i = 0; i < count; i++) {
        free(results[i].doc_path);
    }
    
    free(results);
}