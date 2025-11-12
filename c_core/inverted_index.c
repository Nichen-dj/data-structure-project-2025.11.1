#include "inverted_index.h"

unsigned int hash_function(const char *term, int num_buckets) {
    unsigned int hash = 0;
    while (*term) {
        hash = (hash << 5) + *term++;
    }
    return hash % num_buckets;
}

InvertedIndex* inverted_index_create(int num_buckets, int num_docs) {
    InvertedIndex *index = (InvertedIndex*)malloc(sizeof(InvertedIndex));
    if (index) {
        index->num_buckets = num_buckets;
        index->num_docs = num_docs;
        index->buckets = (IndexNode**)calloc(num_buckets, sizeof(IndexNode*));
    }
    return index;
}

void inverted_index_add_term(InvertedIndex *index, const char *term, int doc_id) {
    if (!index || !term) return;
    
    unsigned int bucket = hash_function(term, index->num_buckets);
    IndexNode *current = index->buckets[bucket];
    
    // 检查词是否已存在
    while (current) {
        if (strcmp(current->term, term) == 0) {
            // 检查该文档是否已在 postings 中
            Posting *post = current->postings;
            while (post) {
                if (post->doc_id == doc_id) {
                    post->term_frequency++;
                    return;
                }
                if (!post->next) break;
                post = post->next;
            }
            
            // 添加新的 posting
            Posting *new_post = (Posting*)malloc(sizeof(Posting));
            new_post->doc_id = doc_id;
            new_post->term_frequency = 1;
            new_post->next = current->postings;
            current->postings = new_post;
            current->doc_count++;
            return;
        }
        if (!current->next) break;
        current = current->next;
    }
    
    // 词不存在，创建新的索引节点
    IndexNode *new_node = (IndexNode*)malloc(sizeof(IndexNode));
    new_node->term = (char*)malloc(strlen(term) + 1);
    strcpy(new_node->term, term);
    new_node->doc_count = 1;
    
    // 创建 posting
    new_node->postings = (Posting*)malloc(sizeof(Posting));
    new_node->postings->doc_id = doc_id;
    new_node->postings->term_frequency = 1;
    new_node->postings->next = NULL;
    
    // 添加到链表
    new_node->next = index->buckets[bucket];
    index->buckets[bucket] = new_node;
}

Posting* inverted_index_get_postings(InvertedIndex *index, const char *term) {
    if (!index || !term) return NULL;
    
    unsigned int bucket = hash_function(term, index->num_buckets);
    IndexNode *current = index->buckets[bucket];
    
    while (current) {
        if (strcmp(current->term, term) == 0) {
            return current->postings;
        }
        current = current->next;
    }
    
    return NULL;
}

void inverted_index_free(InvertedIndex *index) {
    if (!index) return;
    
    for (int i = 0; i < index->num_buckets; i++) {
        IndexNode *current = index->buckets[i];
        while (current) {
            IndexNode *temp = current;
            current = current->next;
            
            // 释放 postings
            Posting *post = temp->postings;
            while (post) {
                Posting *post_temp = post;
                post = post->next;
                free(post_temp);
            }
            
            free(temp->term);
            free(temp);
        }
    }
    
    free(index->buckets);
    free(index);
}

void inverted_index_save(InvertedIndex *index, const char *filename) {
    if (!index || !filename) return;
    
    FILE *file = fopen(filename, "wb");
    if (!file) return;
    
    // 保存基本信息
    fwrite(&index->num_buckets, sizeof(int), 1, file);
    fwrite(&index->num_docs, sizeof(int), 1, file);
    
    // 保存每个桶的内容
    for (int i = 0; i < index->num_buckets; i++) {
        IndexNode *current = index->buckets[i];
        
        // 保存该桶中节点数量
        int node_count = 0;
        IndexNode *temp = current;
        while (temp) {
            node_count++;
            temp = temp->next;
        }
        fwrite(&node_count, sizeof(int), 1, file);
        
        // 保存每个节点
        while (current) {
            // 保存词
            int term_len = strlen(current->term);
            fwrite(&term_len, sizeof(int), 1, file);
            fwrite(current->term, sizeof(char), term_len, file);
            
            // 保存文档计数
            fwrite(&current->doc_count, sizeof(int), 1, file);
            
            // 保存posting数量
            int post_count = 0;
            Posting *post = current->postings;
            while (post) {
                post_count++;
                post = post->next;
            }
            fwrite(&post_count, sizeof(int), 1, file);
            
            // 保存每个posting
            post = current->postings;
            while (post) {
                fwrite(&post->doc_id, sizeof(int), 1, file);
                fwrite(&post->term_frequency, sizeof(int), 1, file);
                post = post->next;
            }
            
            current = current->next;
        }
    }
    
    fclose(file);
}

InvertedIndex* inverted_index_load(const char *filename) {
    if (!filename) return NULL;
    
    FILE *file = fopen(filename, "rb");
    if (!file) return NULL;
    
    InvertedIndex *index = (InvertedIndex*)malloc(sizeof(InvertedIndex));
    if (!index) {
        fclose(file);
        return NULL;
    }
    
    // 加载基本信息
    fread(&index->num_buckets, sizeof(int), 1, file);
    fread(&index->num_docs, sizeof(int), 1, file);
    
    index->buckets = (IndexNode**)calloc(index->num_buckets, sizeof(IndexNode*));
    if (!index->buckets) {
        free(index);
        fclose(file);
        return NULL;
    }
    
    // 加载每个桶的内容
    for (int i = 0; i < index->num_buckets; i++) {
        int node_count;
        fread(&node_count, sizeof(int), 1, file);
        
        IndexNode *head = NULL;
        IndexNode **current_ptr = &head;
        
        for (int j = 0; j < node_count; j++) {
            // 加载词
            int term_len;
            fread(&term_len, sizeof(int), 1, file);
            
            char *term = (char*)malloc(term_len + 1);
            fread(term, sizeof(char), term_len, file);
            term[term_len] = '\0';
            
            // 加载文档计数
            int doc_count;
            fread(&doc_count, sizeof(int), 1, file);
            
            // 加载posting数量
            int post_count;
            fread(&post_count, sizeof(int), 1, file);
            
            // 创建索引节点
            *current_ptr = (IndexNode*)malloc(sizeof(IndexNode));
            (*current_ptr)->term = term;
            (*current_ptr)->doc_count = doc_count;
            (*current_ptr)->next = NULL;
            
            // 加载postings
            Posting *post_head = NULL;
            Posting **post_ptr = &post_head;
            
            for (int k = 0; k < post_count; k++) {
                int doc_id, term_freq;
                fread(&doc_id, sizeof(int), 1, file);
                fread(&term_freq, sizeof(int), 1, file);
                
                *post_ptr = (Posting*)malloc(sizeof(Posting));
                (*post_ptr)->doc_id = doc_id;
                (*post_ptr)->term_frequency = term_freq;
                (*post_ptr)->next = NULL;
                
                post_ptr = &(*post_ptr)->next;
            }
            
            (*current_ptr)->postings = post_head;
            current_ptr = &(*current_ptr)->next;
        }
        
        index->buckets[i] = head;
    }
    
    fclose(file);
    return index;
}