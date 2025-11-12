#include "tfidf.h"
#include <math.h>

double calculate_tfidf(int term_freq, int doc_count, int total_docs) {
    if (doc_count == 0) return 0.0;
    
    // TF: 词频，使用对数归一化
    double tf = log10(1 + term_freq);
    
    // IDF: 逆文档频率
    double idf = log10((double)total_docs / doc_count);
    
    return tf * idf;
}

DocScore* calculate_document_scores(InvertedIndex *index, char **terms, int num_terms, int *result_count) {
    if (!index || !terms || num_terms <= 0) {
        *result_count = 0;
        return NULL;
    }
    
    // 使用哈希表存储文档分数
    DocScore *scores = NULL;
    *result_count = 0;
    
    for (int i = 0; i < num_terms; i++) {
        const char *term = terms[i];
        Posting *postings = inverted_index_get_postings(index, term);
        
        if (!postings) continue;
        
        // 查找该词的文档计数
        unsigned int bucket = hash_function(term, index->num_buckets);
        IndexNode *node = index->buckets[bucket];
        while (node && strcmp(node->term, term) != 0) {
            node = node->next;
        }
        
        if (!node) continue;
        int doc_count = node->doc_count;
        
        // 计算每个文档的TF-IDF并累加
        Posting *post = postings;
        while (post) {
            double tfidf = calculate_tfidf(post->term_frequency, doc_count, index->num_docs);
            
            // 检查文档是否已在分数列表中
            int found = 0;
            for (int j = 0; j < *result_count; j++) {
                if (scores[j].doc_id == post->doc_id) {
                    scores[j].score += tfidf;
                    found = 1;
                    break;
                }
            }
            
            // 如果不在列表中，添加新条目
            if (!found) {
                *result_count += 1;
                scores = (DocScore*)realloc(scores, *result_count * sizeof(DocScore));
                scores[*result_count - 1].doc_id = post->doc_id;
                scores[*result_count - 1].score = tfidf;
            }
            
            post = post->next;
        }
    }
    
    return scores;
}

// 快速排序比较函数
static int compare_scores(const void *a, const void *b) {
    DocScore *score_a = (DocScore*)a;
    DocScore *score_b = (DocScore*)b;
    
    // 降序排列
    if (score_a->score < score_b->score) return 1;
    if (score_a->score > score_b->score) return -1;
    return 0;
}

void sort_doc_scores(DocScore *scores, int count) {
    qsort(scores, count, sizeof(DocScore), compare_scores);
}