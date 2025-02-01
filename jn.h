#ifndef JN_H_
#define JN_H_

#ifndef MAX_KEY_LEN
#define MAX_KEY_LEN 16
#endif

#ifndef MAX_VALUE_LEN
#define MAX_VALUE_LEN 64
#endif

#ifndef MAX_JN_ITEMS
#define MAX_JN_ITEMS 8
#endif

#ifndef JA_ITEM_CAP_INIT 
#define JA_ITEM_CAP_INIT 8
#endif

#ifndef JA_ARRAY_CAP_INIT 
#define JA_ARRAY_CAP_INIT 2
#endif

#ifndef JN_ASSERT
#define JN_ASSERT assert
#endif

#ifndef JN_MALLOC
#define JN_MALLOC malloc
#endif

#ifndef JN_REALLOC
#define JN_REALLOC realloc
#endif

#ifndef JN_CALLOC
#define JN_CALLOC calloc
#endif

typedef struct {
    char key[MAX_KEY_LEN];
    char value[MAX_VALUE_LEN];
} JN_KV;

typedef struct {
    int count;
    JN_KV items[MAX_JN_ITEMS];
} JN_Obj;

struct JN_Arr {
    struct JN_Arr** arrays;
    int arr_count;
    int arr_capacity;

    int count;
    int capacity;
    JN_Obj items[];
};

typedef struct JN_Arr JN_Arr;

JN_Arr* jn_arr(char* json_data);
JN_Obj jn_obj(char* json_data);
char* jo_value(JN_Obj* o, const char* key);
void ja_free(JN_Arr* j);

#endif // JN_H_

#ifdef JN_IMPLEMENTATION
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

JN_Obj jn_obj(char* json_data)
{
    JN_Obj jo = { 0 };

    size_t len = strlen(json_data);
    int start_idx = -1;
    int end_idx = -1;
    int stack = 0;
    for (size_t i = 0; i < len; i++) {
        char cur = json_data[i];
        if (cur == '{') {
            if (stack == 0) {
                start_idx = i;
            }
            stack++;
        }
        if (cur == '}') {
            stack--;
            if (stack == 0) {
                end_idx = i;
            }
        }
    }
    JN_ASSERT(end_idx > 0 && "Invalid JSON format.");
    JN_ASSERT(end_idx > start_idx && "Invalid JSON format.");

    json_data[end_idx + 1] = '\0';
    json_data += start_idx + 1;

    int key_start = 0, key_end = 0;
    int val_start = 0, val_end = 0;
    int val_type = 0;

    stack = 0;
    int part = 0;
    bool escaped = false;
    for (size_t i = 0; i < strlen(json_data); i++) {
        if (escaped) {
            continue;
        }
        char cur = json_data[i];
        if (cur == '\\') {
            escaped = true;
            continue;
        }
        if (cur == ' ') {
            continue;
        }
        if (part == 0) {
            if (cur == '"') {
                if (stack == 0) {
                    key_start = i + 1;
                    stack = 1;
                } else if (stack == 1) {
                    stack = 0;
                    part++;
                    key_end = i;
                    // printf("%.*s: ",
                    //     key_end - key_start, json_data + key_start);
                }
            }
        } else if (part == 1) {
            val_type = 0;
            stack = 0;
            if (cur == ':') {
                part++;
            }
        } else if (part == 2) {
            if (val_type == 0) {
                val_start = i;
                if (cur == '{' || cur == '[') {
                    val_type = 1;
                    stack = 1;
                } else if (cur == '"') {
                    val_type = 2;
                    val_start++;
                    stack = 1;
                } else if (cur - '0' < 10) {
                    val_type = 3;
                    stack = 0;
                }
                continue;
            } else if (cur == '{' || cur == '[') {
                stack++;
            } else if ((val_type == 3 && (cur == ',' || cur == '}'))
                || (val_type == 2 && cur == '"')
                || (val_type == 1 && (cur == '}' || cur == ']'))) {
                if (val_type == 1) {
                    if (stack > 1) {
                        stack--;
                        continue;
                    }
                    val_end = i + 1;
                    part++;
                } else {
                    val_end = i;
                    part = 0;
                }
                // printf("%.*s\n",
                //     val_end - val_start, json_data + val_start);
                val_type = 0;
                stack = 0;
                JN_ASSERT(key_end - key_start + 1 <= MAX_KEY_LEN);
                memcpy(jo.items[jo.count].key, json_data + key_start, key_end - key_start);
                JN_ASSERT(val_end - val_start + 1 <= MAX_VALUE_LEN);
                memcpy(jo.items[jo.count].value, json_data + val_start, val_end - val_start);
                jo.count++;
            }
        } else if (part == 3) {
            if (cur == ',') {
                part = 0;
            }
        }
    }

    return jo;
}

JN_Arr* ja_add_obj(JN_Arr* ja, char* json_data)
{
    JN_ASSERT(ja != NULL);

    if (ja->count >= ja->capacity) {
        while (ja->count >= ja->capacity) {
            ja->capacity *= 2;
        }
        ja = JN_REALLOC(ja, sizeof(JN_Arr) + sizeof(JN_Obj) * ja->capacity);
    }

    JN_Obj o = jn_obj(json_data);
    ja->items[ja->count++] = o;
    return ja;
}

JN_Arr* jn_arr(char* json_data)
{
    JN_Arr* ja;

    if (json_data[0] == '{') {
        ja = JN_CALLOC(1, sizeof(JN_Arr) + sizeof(JN_Obj) * 1);
        ja->capacity = 1;
        ja->arrays = NULL;
        ja_add_obj(ja, json_data);
        return ja;
    }

    ja = JN_CALLOC(1, sizeof(JN_Arr) + sizeof(JN_Obj) * JA_ITEM_CAP_INIT);
    ja->capacity = JA_ITEM_CAP_INIT;
    ja->arr_capacity = JA_ARRAY_CAP_INIT;
    ja->arrays = JN_MALLOC(sizeof(JN_Arr*) * ja->arr_capacity);

    bool escaped = false;
    int stack = 0;
    int type = 0;
    int item_start_idx = 0;
    int item_end_idx = 0;
    size_t length = strlen(json_data);
    for (size_t i = 1; i < length; i++) {
        if (escaped) {
            continue;
        }
        char cur = json_data[i];
        if (cur == '\\') {
            escaped = true;
            continue;
        }
        if (cur == ' ') {
            continue;
        }
        if (cur == '{' || cur == '[') {
            if (type == 0) {
                if (cur == '{') {
                    type = 1;
                } else {
                    type = 2;
                }
                item_start_idx = i;
            }
            stack++;
        } else if (cur == ']' || cur == '}') {
            stack--;
            if (stack == 0) {
                item_end_idx = i + 1;
                if (type == 1 && cur == '}') {
                    // printf("\ntype1: %.*s\n",
                    //     item_end_idx - item_start_idx, json_data + item_start_idx);
                    json_data[item_end_idx] = '\0';
                    ja = ja_add_obj(ja, json_data + item_start_idx);
                } else if (type == 2 && cur == ']') {
                    // printf("\ntype2: %.*s\n",
                    //     item_end_idx - item_start_idx, json_data + item_start_idx);
                    json_data[item_end_idx] = '\0';

                    if (ja->arr_count >= ja->arr_capacity) {
                        while (ja->arr_count >= ja->arr_capacity) {
                            ja->arr_capacity *= 2;
                        }
                        ja->arrays = JN_REALLOC(ja->arrays, sizeof(JN_Arr*) * ja->arr_capacity);
                    }

                    JN_Arr* ji = jn_arr(json_data + item_start_idx);
                    ja->arrays[ja->arr_count++] = ji;
                } else {
                    // printf("\nassert: %.*s\n",
                    //     item_end_idx - item_start_idx, json_data + item_start_idx);
                    JN_ASSERT(false && "Invalid JSON format.");
                }
                type = 0;
            }
        }
    }
    return ja;
}

char* jo_value(JN_Obj* jo, const char* key)
{
    for (int i = 0; i < jo->count; i++) {
        if (strcmp(jo->items[i].key, key) == 0) {
            return jo->items[i].value;
        }
    }
    return NULL;
}

void ja_free(JN_Arr* ja)
{
    for (int i = 0; i < ja->arr_count; i++) {
        ja_free(ja->arrays[i]);
    }
    free(ja->arrays);
    free(ja);
}

#endif // JN_IMPLEMENTATION
