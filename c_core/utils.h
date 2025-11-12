#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include "trie.h"
#include "inverted_index.h"

// 从文件加载停用词
char** load_stop_words(const char *filename, int *count);

// 检查是否是停用词
int is_stop_word(char **stop_words, int count, const char *word);

// 从文档目录构建Trie树和倒排索引
void build_index_from_docs(const char *doc_dir, TrieNode *trie, InvertedIndex *index, 
                          char ***doc_paths, int *num_docs);

// 保存文档路径
void save_doc_paths(char **doc_paths, int num_docs, const char *filename);

// 加载文档路径
char** load_doc_paths(const char *filename, int *num_docs);

// 保存Trie树
void trie_save(TrieNode *root, const char *filename);

// 加载Trie树
TrieNode* trie_load(const char *filename);

#endif