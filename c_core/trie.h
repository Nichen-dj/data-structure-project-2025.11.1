#ifndef TRIE_H
#define TRIE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define ALPHABET_SIZE 26

typedef struct TrieNode {
    struct TrieNode *children[ALPHABET_SIZE];
    bool is_end_of_word;
} TrieNode;

TrieNode* trie_create_node();
void trie_insert(TrieNode *root, const char *word);
bool trie_search(TrieNode *root, const char *word);
void trie_get_prefix_matches(TrieNode *root, const char *prefix, char ***matches, int *count);
void trie_free(TrieNode *root);

#endif