#ifndef TFIDF_H
#define TFIDF_H

#include <stdio.h>
#include <stdlib.h>
#include "inverted_index.h"

typedef struct DocScore {
    int doc_id;
    double score;
} DocScore;

// 计算TF-IDF分数
double calculate_tfidf(int term_freq, int doc_count, int total_docs);

// 为一组词计算文档分数
DocScore* calculate_document_scores(InvertedIndex *index, char **terms, int num_terms, int *result_count);

// 对文档分数进行排序
void sort_doc_scores(DocScore *scores, int count);

#endif