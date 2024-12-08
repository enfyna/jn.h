#ifndef JN_H_
#define JN_H_

#ifndef MAX_KEY_LEN
#define MAX_KEY_LEN 16
#endif

#ifndef MAX_VALUE_LEN
#define MAX_VALUE_LEN 64
#endif

#ifndef MAX_JN_ITEMS
#define MAX_JN_ITEMS 32
#endif

typedef struct {
    char key[MAX_KEY_LEN];
    char value[MAX_VALUE_LEN];
    int type;
} JN_Key_Value_Pair;

typedef struct {
    JN_Key_Value_Pair items[MAX_JN_ITEMS];
    int count;
} JN_Obj;

struct JN_Arr {
    struct JN_Arr** arrays;
    int arr_count;
    int arr_capacity;
    JN_Obj** items;
    int item_count;
    int item_capacity;
};

typedef struct JN_Arr JN_Arr;

JN_Arr* jn_arr(char* json_data);
JN_Obj* jn_obj(char* json_data);
char* jo_value(JN_Obj* o, const char* key);
void ja_free(JN_Arr* j);

#endif // JN_H_

#ifdef JN_IMPLEMENTATION
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

JN_Obj* jn_obj(char* json_data)
{
    JN_Obj* jo = calloc(1, sizeof(JN_Obj));

    int start_idx = 0;
    int end_idx = 0;
    int stack = 0;
    for (size_t i = 0; i < strlen(json_data); i++) {
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
            val_start = i + 1;
            val_end = i + 1;
            if (cur == ':') {
                part++;
            }
        } else if (part == 2) {
            if (val_type != 0) {
                if (val_type == 3 && (cur == ',' || cur == '}')) {
                    val_end = i;
                    // printf("%.*s\n",
                    //     val_end - val_start, json_data + val_start);
                    part = 0;
                    val_type = 0;
                    stack = 0;
                    memcpy(jo->items[jo->count].key, json_data + key_start, key_end - key_start);
                    mempcpy(jo->items[jo->count].value, json_data + val_start, val_end - val_start);
                    jo->count++;
                } else if (val_type == 2 && cur == '"') {
                    val_end = i;
                    // printf("%.*s\n",
                    //     val_end - val_start, json_data + val_start);
                    part = 0;
                    val_type = 0;
                    stack = 0;
                    memcpy(jo->items[jo->count].key, json_data + key_start, key_end - key_start);
                    mempcpy(jo->items[jo->count].value, json_data + val_start, val_end - val_start);
                    jo->count++;
                } else if (val_type == 1 && (cur == '}' || cur == ']')) {
                    val_end = i + 1;
                    // printf("%.*s\n",
                    //     val_end - val_start, json_data + val_start);
                    part++;
                    val_type = 0;
                    stack = 0;
                    memcpy(jo->items[jo->count].key, json_data + key_start, key_end - key_start);
                    mempcpy(jo->items[jo->count].value, json_data + val_start, val_end - val_start);
                    jo->count++;
                }
            } else if (cur == '{' || cur == '[') {
                val_type = 1;
                val_start = i;
                stack = 1;
            } else if (cur == '"') {
                val_type = 2;
                val_start = i + 1;
                stack = 1;
            } else if (cur - '0' < 9) {
                val_type = 3;
                val_start = i;
                stack = 0;
            }
        } else if (part == 3) {
            if (cur == ',') {
                part = 0;
            }
        }
    }

    return jo;
}

void ja_add_obj(JN_Arr* ja, char* json_data)
{
    JN_Obj* o = jn_obj(json_data);
    if (ja != NULL) {
        while (ja->item_count >= ja->item_capacity) {
            ja->item_capacity *= 2;
            ja->items = realloc(ja->items, sizeof(JN_Obj*) * ja->item_capacity);
        }
        ja->items[ja->item_count++] = o;
    }
}

JN_Arr* jn_arr(char* json_data)
{
    JN_Arr* ja = calloc(1, sizeof(JN_Arr));

    if (json_data[0] == '{') {
        ja->item_capacity = 1;
        ja->items = malloc(sizeof(JN_Obj*) * ja->item_capacity);
        ja->arrays = NULL;
        ja_add_obj(ja, json_data);
        return ja;
    }

    ja->item_capacity = 8;
    ja->items = malloc(sizeof(JN_Obj*) * ja->item_capacity);
    ja->arr_capacity = 12;
    ja->arrays = malloc(sizeof(JN_Arr*) * ja->item_capacity);

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
                    ja_add_obj(ja, json_data + item_start_idx);
                } else if (type == 2 && cur == ']') {
                    // printf("\ntype2: %.*s\n",
                    //     item_end_idx - item_start_idx, json_data + item_start_idx);
                    json_data[item_end_idx] = '\0';
                    JN_Arr* ji = jn_arr(json_data + item_start_idx);
                    ja->arrays[ja->arr_count++] = ji;
                } else {
                    // printf("\nassert: %.*s\n",
                    //     item_end_idx - item_start_idx, json_data + item_start_idx);
                    assert(false && "Invalid JSON format.");
                }
                type = 0;
            }
        }
    }
    printf("\n");

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
    for (int i = 0; i < ja->item_count; i++) {
        free(ja->items[i]);
    }
    free(ja->arrays);
    free(ja->items);
    free(ja);
}

#endif // JN_IMPLEMENTATION
