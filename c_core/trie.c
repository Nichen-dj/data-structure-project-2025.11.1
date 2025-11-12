#include "trie.h"

TrieNode* trie_create_node() {
    TrieNode *node = (TrieNode*)malloc(sizeof(TrieNode));
    if (node) {
        node->is_end_of_word = false;
        for (int i = 0; i < ALPHABET_SIZE; i++) {
            node->children[i] = NULL;
        }
    }
    return node;
}

void trie_insert(TrieNode *root, const char *word) {
    TrieNode *current = root;
    for (int i = 0; word[i] != '\0'; i++) {
        int index = word[i] - 'a';
        if (index < 0 || index >= ALPHABET_SIZE) continue;
        
        if (!current->children[index]) {
            current->children[index] = trie_create_node();
        }
        current = current->children[index];
    }
    current->is_end_of_word = true;
}

bool trie_search(TrieNode *root, const char *word) {
    TrieNode *current = root;
    for (int i = 0; word[i] != '\0'; i++) {
        int index = word[i] - 'a';
        if (index < 0 || index >= ALPHABET_SIZE || !current->children[index]) {
            return false;
        }
        current = current->children[index];
    }
    return current->is_end_of_word;
}

static void collect_words(TrieNode *node, char *current_word, int depth, char ***matches, int *count) {
    if (node->is_end_of_word) {
        current_word[depth] = '\0';
        *matches = (char**)realloc(*matches, (*count + 1) * sizeof(char*));
        (*matches)[*count] = (char*)malloc((depth + 1) * sizeof(char));
        strcpy((*matches)[*count], current_word);
        (*count)++;
    }
    
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (node->children[i]) {
            current_word[depth] = 'a' + i;
            collect_words(node->children[i], current_word, depth + 1, matches, count);
        }
    }
}

void trie_get_prefix_matches(TrieNode *root, const char *prefix, char ***matches, int *count) {
    *matches = NULL;
    *count = 0;
    
    TrieNode *current = root;
    int prefix_len = strlen(prefix);
    
    // 移动到前缀的最后一个节点
    for (int i = 0; i < prefix_len; i++) {
        int index = prefix[i] - 'a';
        if (index < 0 || index >= ALPHABET_SIZE || !current->children[index]) {
            return; // 前缀不存在
        }
        current = current->children[index];
    }
    
    // 收集所有匹配的单词
    char *current_word = (char*)malloc((prefix_len + 100) * sizeof(char)); // 假设单词长度不超过100
    strcpy(current_word, prefix);
    collect_words(current, current_word, prefix_len, matches, count);
    
    free(current_word);
}

void trie_free(TrieNode *root) {
    if (!root) return;
    
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        trie_free(root->children[i]);
    }
    
    free(root);
}