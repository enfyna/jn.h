#include <stdbool.h>

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

#ifndef MAX_ARR_SIZE
#define MAX_ARR_SIZE 24
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

typedef struct {
    struct JN_Arr** arrays;
    int arr_count;
    int arr_capacity;
    JN_Obj** items;
    int item_count;
    int item_capacity;
} JN_Arr;

JN_Arr* jn_arr(char* json_data);
JN_Obj* jn_obj(char* json_data);
char* jo_value(JN_Obj* o, const char* key);
void ja_free(JN_Arr* j);

#endif // JN_H_

#ifdef JN_IMPLEMENTATION
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

JN_Obj* jn_obj(char* json_data) {
    JN_Obj* jo = calloc(1, sizeof(JN_Obj));

    strsep(&json_data, "{");
    json_data = strsep(&json_data, "}");

    char* sep;
    while ((sep = strsep(&json_data, ",")) != 0) {
        char* key = strsep(&sep, ":");
        char* val = sep;
        if (key[0] == '"') {
            strsep(&key, "\"");
            key = strsep(&key, "\"");
        }
        if (val[0] == '\"') {
            strsep(&val, "\"");
            val = strsep(&val, "\"");
        }
        memcpy(jo->items[jo->count].key, key, strlen(key));
        mempcpy(jo->items[jo->count].value, val, strlen(val));
        jo->count++;
    }
    return jo;
}

void ja_add_obj(JN_Arr* ja, char* json_data) {
    JN_Obj* jo = jn_obj(json_data);
    if (ja != NULL) {
        while (ja->item_count >= ja->item_capacity) {
            ja->item_capacity *= 2;
            ja->items = realloc(ja->items, sizeof(JN_Obj*) * ja->item_capacity);
        }
        ja->items[ja->item_count++] = jo;
    }
}

JN_Arr* jn_arr(char* json_data) {
    JN_Arr* ja = calloc(1, sizeof(JN_Arr));

    bool is_array = false;

    switch (json_data[0]) {
    case '{': {
        ja->item_capacity = 1;
        ja->items = malloc(sizeof(JN_Obj*) * ja->item_capacity);
        ja->arrays = NULL;
        break;
    }
    case '[': {
        is_array = true;
        ja->item_capacity = 8;
        ja->items = malloc(sizeof(JN_Obj*) * ja->item_capacity);
        ja->arr_capacity = 12;
        ja->arrays = malloc(sizeof(JN_Arr*) * ja->item_capacity);
        break;
    }
    default:
        return NULL;
    }

    if (!is_array) {
        ja_add_obj(ja, json_data);
    } else {
        strsep(&json_data, "[");
        json_data = strsep(&json_data, "]");

        char* sep;
        while ((sep = strsep(&json_data, "}")) != 0) {
            ja_add_obj(ja, sep);
        }
    }
    return ja;
}

char* jo_value(JN_Obj* jo, const char* key) {
    for (int i = 0; i < jo->count; i++) {
        if (strcmp(jo->items[i].key, key) == 0) {
            return jo->items[i].value;
        }
    }
    return NULL;
}

void ja_free(JN_Arr* ja) {
    for (int i = 0; i < ja->arr_count; i++) {
        free(ja->arrays[i]);
    }
    for (int i = 0; i < ja->item_count; i++) {
        free(ja->items[i]);
    }
    free(ja->arrays);
    free(ja->items);
    free(ja);
}

#endif // JN_IMPLEMENTATION
