#ifndef INVERTED_INDEX_H
#define INVERTED_INDEX_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Posting {
    int doc_id;
    int term_frequency;
    struct Posting *next;
} Posting;

typedef struct IndexNode {
    char *term;
    Posting *postings;
    int doc_count; // 包含该词的文档数
    struct IndexNode *next;
} IndexNode;

typedef struct InvertedIndex {
    IndexNode **buckets;
    int num_buckets;
    int num_docs; // 总文档数
} InvertedIndex;

// 哈希函数
unsigned int hash_function(const char *term, int num_buckets);

// 倒排索引操作
InvertedIndex* inverted_index_create(int num_buckets, int num_docs);
void inverted_index_add_term(InvertedIndex *index, const char *term, int doc_id);
Posting* inverted_index_get_postings(InvertedIndex *index, const char *term);
void inverted_index_free(InvertedIndex *index);
void inverted_index_save(InvertedIndex *index, const char *filename);
InvertedIndex* inverted_index_load(const char *filename);

#endif