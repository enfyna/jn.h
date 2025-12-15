#ifndef JN_H_
#define JN_H_
#include <stdbool.h>
#include <sys/types.h>

#define jn_data_str(j, jn_data) jn_data.length, jn_data.start + j.data

#define FOREACH_JN_VALUE_TYPE(Function)                    \
    Function(JN_VT_NONE)                                   \
        Function(JN_VT_INVALID)                            \
            Function(JN_VT_STRING)                         \
                Function(JN_VT_NUMBER)                     \
                    Function(JN_VT_ARRAY)                  \
                        Function(JN_VT_OBJECT)             \
                            Function(JN_VT_KEYWORD)        \
                                Function(JN_VT_WHITESPACE) \
                                    Function(JN_VT_VALUE)

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

enum JN_ValueType {
    FOREACH_JN_VALUE_TYPE(GENERATE_ENUM)
};

static const char* JN_VT_String[] = {
    FOREACH_JN_VALUE_TYPE(GENERATE_STRING)
};

typedef struct {
    uint start;
    uint length;
    bool isValid;
} JN_Data;

typedef struct {
    JN_Data data;
    enum JN_ValueType type;
} JN_Value;

typedef struct {
    JN_Data key;
    JN_Value value;
} JN_KeyValuePair;

typedef struct {
    JN_Value* items;
    uint count;
    uint capacity;
} JN_ValueList;

typedef struct {
    JN_KeyValuePair* items;
    uint count;
    uint capacity;
} JN_KeyValuePairList;

typedef struct {
    const char* data;
    uint data_length;

    enum JN_ValueType type;
    union {
        JN_Data literal;
        JN_ValueList list;
        JN_KeyValuePairList kvs;
    };

    bool isValid;
} JN;

JN jn_alloc(char* json_data, uint length);

JN jn_obj_get(JN* j, char* key);
JN_Value jn_obj_get_value(JN* j, char* key);
bool jn_is_key(JN* j, JN_KeyValuePair* kv, char* key);

JN jn_arr_get(JN* j, size_t idx);

void jn_print_full(JN* j);
void jn_print(JN* j);
void jn_free(JN* j);


#ifdef JN_IMPLEMENTATION


#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define STRSTR(S) #S
#define STR(S) STRSTR(S)

JN_Data skip_next_array(JN* j, size_t* pos);
JN_Data skip_next_string(JN* j, size_t* pos);
JN_Data skip_next_number(JN* j, size_t* pos);
JN_Data skip_next_object(JN* j, size_t* pos);
JN_Data skip_next_keyword(JN* j, size_t* pos);
JN_KeyValuePair skip_next_kv(JN* j, size_t* pos, bool* is_last);

void skip_whitespace(JN* j, size_t* pos)
{
    const char whitespaceChars[] = { ' ', '\n', '\r', '\t' };
    for (; *pos < j->data_length; ++*pos) {
        char c = j->data[*pos];
        if (memchr(whitespaceChars, c, 4) == NULL)
            return;
    }
}

JN_Value skip_next_value(JN* j, size_t* pos)
{
    JN_Value val = { 0 };

    if (*pos >= j->data_length)
        return (JN_Value) { .type = JN_VT_NONE };

    char c = j->data[*pos];
    if (c == '"') {
        val.type = JN_VT_STRING;
        val.data = skip_next_string(j, pos);
    } else if (c == '{') {
        val.type = JN_VT_OBJECT;
        val.data = skip_next_object(j, pos);
    } else if (isdigit(c) || c == '-') {
        val.type = JN_VT_NUMBER;
        val.data = skip_next_number(j, pos);
    } else if (c == '[') {
        val.type = JN_VT_ARRAY;
        val.data = skip_next_array(j, pos);
    } else if (c == 'n' || c == 'f' || c == 't') {
        val.type = JN_VT_KEYWORD;
        val.data = skip_next_keyword(j, pos);
    } else {
        val.type = JN_VT_INVALID;
        val.data.isValid = false;
        fprintf(stderr, "Found unexpected value start %c @ index: %zu!\n", c, *pos);
    }

    if (val.data.start < 0)
        val.data.isValid = false;

    if (val.data.isValid) {
        // printf("Found value: %.*s\n", val.data.length, j->data + val.data.start);
    } else
        fprintf(stderr, "Error parsing value: %.*s @ index: %zu\n", val.data.length, j->data + val.data.start, *pos);

    return val;
}

JN_Data skip_next_chars(JN* j, size_t* pos, char* cs)
{
    assert(cs != NULL);
    uint char_count = strlen(cs);
    int found = -1;
    assert(char_count > 0);

    JN_Data data = { .isValid = true };
    if (*pos >= j->data_length) {
        fprintf(stderr, "Expected '%s' got EOF @ index: %zu\n", cs, *pos);
        data.isValid = false;
    }
    char c = j->data[*pos];
    for (size_t i = 0; i < char_count; i++) {
        if (cs[i] == c) {
            found = i;
            break;
        }
    }
    if (found == -1) {
        fprintf(stderr, "Expected '%s' got '%c' @ index: %zu\n", cs, j->data[*pos], *pos);
        data.isValid = false;
    }
    if (found >= 0) {
        data.start = (*pos)++;
        data.length = 1;
        // printf("Found \'%c\': %.*s\n", cs[found], 1, j->data + data.start);
    }
    return data;
}

JN_Data skip_next_char(JN* j, size_t* pos, char c)
{
    JN_Data data = { .isValid = true };
    if (c != j->data[*pos]) {
        fprintf(stderr, "Expected '%c' got '%c' @ index: %zu\n", c, j->data[*pos], *pos);
        data.isValid = false;
    }
    data.start = (*pos)++;
    data.length = 1;
    // printf("Found \'%c\': %.*s\n", c, 1, j->data + data.start);
    return data;
}

JN_Data skip_next_keyword(JN* j, size_t* pos)
{
    skip_whitespace(j, pos);

    JN_Data data = { .isValid = true };

    const char f[] = "false";
    const char t[] = "true";
    const char n[] = "null";
    const char* ks[] = { f, t, n };
    size_t ks_count = sizeof(ks) / sizeof(ks[0]);

    const char* found = NULL;

    for (size_t i = 0; i < ks_count; i++) {
        char c = j->data[*pos];
        if (c == ks[i][0]) {
            found = ks[i];
        }
    }
    if (found == NULL) {
        fprintf(stderr, "Expected 'f', 't' or 'n' got '%c' @ index: %zu\n", j->data[*pos], *pos);
        data.isValid = false;
        return data;
    }

    size_t found_length = strlen(found);
    data.start = *pos;

    for (size_t i = 0; *pos < j->data_length && i < found_length; (*pos)++, i++) {
        char c = j->data[*pos];
        if (c != found[i]) {
            fprintf(stderr, "Expected '%c' got '%c' @ index: %zu\n", found[i], j->data[*pos], *pos);
            data.isValid = false;
            return data;
        }
    }

    data.length = *pos - data.start;
    char next = j->data[*pos];
    if (isalpha(next)) {
        fprintf(stderr, "Keyword doesnt end \'%c\' @ index: %zu\n", next, *pos);
        data.isValid = false;
    } else if (found_length != data.length) {
        fprintf(stderr, "Corrupt keyword %.*s @ index: %zu\n", data.length, j->data + data.start, *pos);
        data.isValid = false;
    } else {
        // printf("Found \'%s\': %.*s\n", found, data.length, j->data + data.start);
    }

    return data;
}

JN_Data skip_next_number(JN* j, size_t* pos)
{
    JN_Data data = { .isValid = true };
    data.start = *pos;

    enum JN_NUMBER_SECTION {
        START,
        MINUS,
        LEADING_ZERO,
        DIGITS,
        FRACTION_DOT,
        FRACTION_DIGITS,
        EXPONENT_E,
        EXPONENT_SIGN,
        EXPONENT_DIGITS,
    } current_section
        = START;

    for (size_t i = 0; *pos < j->data_length; ++*pos, i++) {
        char c = j->data[*pos];
        if (isspace(c) || c == ',' || c == ']' || c == '}')
            break;
        switch (current_section) {
        case START: {
            if (c == '-') {
                current_section = MINUS;
            } else if (c == '0') {
                current_section = LEADING_ZERO;
            } else if (isdigit(c)) {
                current_section = DIGITS;
            } else {
                fprintf(stderr, "Line[" STR(__LINE__) "]: Error parsing number. Didnt expect '%c' @ index: %zu\n", c, *pos);
                data.isValid = false;
                return data;
            }
            break;
        }
        case MINUS: {
            if (c == '0') {
                current_section = LEADING_ZERO;
            } else if (isdigit(c)) {
                current_section = DIGITS;
            } else {
                fprintf(stderr, "Line[" STR(__LINE__) "]: Error parsing number. Didnt expect '%c' @ index: %zu\n", c, *pos);
                data.isValid = false;
                return data;
            }
            break;
        }
        case LEADING_ZERO: {
            if (c == '.') {
                current_section = FRACTION_DOT;
            } else if (c == 'e' || c == 'E') {
                current_section = EXPONENT_E;
            } else {
                fprintf(stderr, "Line[" STR(__LINE__) "]: Error parsing number. Didnt expect '%c' @ index: %zu\n", c, *pos);
                data.isValid = false;
                return data;
            }
            break;
        }
        case DIGITS: {
            if (isdigit(c)) {
                continue;
            } else if (c == '.') {
                current_section = FRACTION_DOT;
            } else if (c == 'e' || c == 'E') {
                current_section = EXPONENT_E;
            } else {
                fprintf(stderr, "Line[" STR(__LINE__) "]: Error parsing number. Didnt expect '%c' @ index: %zu\n", c, *pos);
                data.isValid = false;
                return data;
            }
            break;
        }
        case FRACTION_DOT: {
            if (isdigit(c)) {
                current_section = FRACTION_DIGITS;
            } else {
                fprintf(stderr, "Line[" STR(__LINE__) "]: Error parsing number. Didnt expect '%c' @ index: %zu\n", c, *pos);
                data.isValid = false;
                return data;
            }
            break;
        }
        case FRACTION_DIGITS: {
            if (isdigit(c)) {
                continue;
            } else if (c == 'e' || c == 'E') {
                current_section = EXPONENT_E;
            } else {
                fprintf(stderr, "Line[" STR(__LINE__) "]: Error parsing number. Didnt expect '%c' @ index: %zu\n", c, *pos);
                data.isValid = false;
                return data;
            }
            break;
        }
        case EXPONENT_E: {
            if (isdigit(c)) {
                current_section = EXPONENT_DIGITS;
            } else if (c == '-' || c == '+') {
                current_section = EXPONENT_SIGN;
            } else {
                fprintf(stderr, "Line[" STR(__LINE__) "]: Error parsing number. Didnt expect '%c' @ index: %zu\n", c, *pos);
                data.isValid = false;
                return data;
            }
            break;
        }
        case EXPONENT_SIGN: {
            if (isdigit(c)) {
                current_section = EXPONENT_DIGITS;
            } else {
                fprintf(stderr, "Line[" STR(__LINE__) "]: Error parsing number. Didnt expect '%c' @ index: %zu\n", c, *pos);
                data.isValid = false;
                return data;
            }
            break;
        }
        case EXPONENT_DIGITS: {
            if (isdigit(c)) {
                continue;
            } else {
                fprintf(stderr, "Line[" STR(__LINE__) "]: Error parsing number. Didnt expect '%c' @ index: %zu\n", c, *pos);
                data.isValid = false;
                return data;
            }
            break;
        }
        }
    }

    data.length = *pos - data.start;

    if (current_section == MINUS
        || current_section == FRACTION_DOT
        || current_section == EXPONENT_E
        || current_section == EXPONENT_SIGN) {
        data.isValid = false;
        fprintf(stderr, "Line[" STR(__LINE__) "]: Error parsing number. Too short @ index: %zu\n", *pos);
    } else {
        // printf("Found number[%d,%d]: %.*s\n", data.start, data.length, data.length, j->data + data.start);
    }

    return data;
}

JN_Data skip_next_string(JN* j, size_t* pos)
{
    const char specialChars[] = " !@#$%^&*()_+-=[]{}|;:',.<>/?";
    const char escapeChars[] = "bfnrt\\/\"";

    JN_Data data = { .isValid = true };
    data.start = *pos;

    enum JN_STRING_SECTION {
        START,
        INSIDE,
        ESCAPE,
        ESCAPE_U_1,
        ESCAPE_U_2,
        ESCAPE_U_3,
        ESCAPE_U_4,
        END,
    } current_section
        = START;

    for (size_t i = 0; *pos < j->data_length; ++*pos, i++) {
        if (current_section == END)
            break;
        char c = j->data[*pos];
        switch (current_section) {
        case START: {
            if (c == '"') {
                current_section = INSIDE;
            } else {
                fprintf(stderr, "Line[" STR(__LINE__) "]: Error parsing string. Didnt expect '%c' @ index: %zu\n", c, *pos);
                data.isValid = false;
                return data;
            }
            break;
        }
        case END: {
            break;
        }
        case INSIDE: {
            if (c == '"') {
                current_section = END;
            } else if (iscntrl(c)) {
                fprintf(stderr, "Line[" STR(__LINE__) "]: Error parsing string. Control characters not allowed: '%c' @ index: %zu\n", c, *pos);
                data.isValid = false;
                return data;
            } else if (c == '\\') {
                current_section = ESCAPE;
            } else if (isalnum(c) || memchr(specialChars, c, 29) != NULL) {
                continue;
            } else {
                fprintf(stderr, "Line[" STR(__LINE__) "]: Error parsing string. Didnt expect '%c' @ index: %zu\n", c, *pos);
                data.isValid = false;
                return data;
            }
            break;
        }
        case ESCAPE: {
            if (memchr(escapeChars, c, 8) != NULL) {
                current_section = INSIDE;
            } else if (c == 'u') {
                current_section = ESCAPE_U_1;
            } else {
                fprintf(stderr, "Line[" STR(__LINE__) "]: Error parsing string. Invalid escape character '%c' @ index: %zu\n", c, *pos);
                data.isValid = false;
                return data;
            }
            break;
        }
        case ESCAPE_U_1: {
            if (isxdigit(c)) {
                current_section = ESCAPE_U_2;
            } else {
                fprintf(stderr, "Line[" STR(__LINE__) "]: Error parsing string. Invalid hex digit '%c' @ index: %zu\n", c, *pos);
                data.isValid = false;
                return data;
            }
            break;
        }
        case ESCAPE_U_2: {
            if (isxdigit(c)) {
                current_section = ESCAPE_U_3;
            } else {
                fprintf(stderr, "Line[" STR(__LINE__) "]: Error parsing string. Invalid hex digit '%c' @ index: %zu\n", c, *pos);
                data.isValid = false;
                return data;
            }
            break;
        }
        case ESCAPE_U_3: {
            if (isxdigit(c)) {
                current_section = ESCAPE_U_4;
            } else {
                fprintf(stderr, "Line[" STR(__LINE__) "]: Error parsing string. Invalid hex digit '%c' @ index: %zu\n", c, *pos);
                data.isValid = false;
                return data;
            }
            break;
        }
        case ESCAPE_U_4: {
            if (isxdigit(c)) {
                current_section = INSIDE;
            } else {
                fprintf(stderr, "Line[" STR(__LINE__) "]: Error parsing string. Invalid hex digit '%c' @ index: %zu\n", c, *pos);
                data.isValid = false;
                return data;
            }
            break;
        }
        }
    }

    data.length = *pos - data.start;

    if (current_section != END) {
        data.isValid = false;
        fprintf(stderr, "Line[" STR(__LINE__) "]: Error parsing string. Didnt reach end @ index: %zu\n", *pos);
    } else {
        // printf("Found string[%d,%d]: %.*s\n", data.start, data.length, data.length, j->data + data.start);
    }

    return data;
}

JN_Data skip_next_array(JN* j, size_t* pos)
{
    JN_Data data = { .isValid = true };

    char c = j->data[*pos];
    data.start = *pos;
    if (c != '[') {
        fprintf(stderr, "Error parsing array @ index: %zu\n", *pos);
        data.isValid = false;
        return data;
    }

    ++*pos;
    skip_whitespace(j, pos);
    if (j->data[*pos] != ']')
        while (*pos <= j->data_length) {
            skip_whitespace(j, pos);
            JN_Value val = skip_next_value(j, pos);
            if (val.data.isValid == false) {
                data.isValid = false;
                break;
            }
            skip_whitespace(j, pos);
            JN_Data delim = skip_next_chars(j, pos, ",]");
            if (delim.isValid == false) {
                data.isValid = false;
                break;
            }
            if (j->data[(*pos) - 1] == ']')
                break;
        }
    else
        ++*pos;

    if (data.isValid == false) {
        fprintf(stderr, "Error parsing array @ index: %zu\n", *pos);
    } else {
        data.length = (*pos) - data.start;
        // printf("Found array: %.*s\n", data.length, j->data + data.start);
    }

    return data;
}

JN_Data skip_next_object(JN* j, size_t* pos)
{
    JN_Data data = { .isValid = true };
    data.start = *pos;

    JN_KeyValuePair kv = { 0 };
    bool is_last = false;

    ++*pos;
    skip_whitespace(j, pos);

    if (j->data[*pos] == '}') {
        ++*pos;
        skip_whitespace(j, pos);
    } else {
        while ((kv = skip_next_kv(j, pos, &is_last)).value.type != JN_VT_NONE) {
            if (kv.value.type == JN_VT_INVALID) {
                fprintf(stderr, "Error: key/value pair is invalid!\n");
                j->isValid = false;
                break;
            }
            if (is_last)
                break;
        }
    }

    data.length = *pos - data.start;
    // printf("Found object: %.*s\n", length, j->data + start);
    return data;
}

JN_KeyValuePair skip_next_kv(JN* j, size_t* pos, bool* is_last)
{
    skip_whitespace(j, pos);
    JN_Data key = skip_next_string(j, pos);
    // printf("key: start: %d, length: %d\n", key.start, key.length);
    if (key.isValid == false)
        return (JN_KeyValuePair) { .value.type = JN_VT_INVALID };

    skip_whitespace(j, pos);
    JN_Data col = skip_next_char(j, pos, ':');
    // printf("col: start: %d, length: %d\n", col.start, col.length);
    if (col.isValid == false)
        return (JN_KeyValuePair) { .value.type = JN_VT_INVALID };

    skip_whitespace(j, pos);
    JN_Value val = skip_next_value(j, pos);
    // printf("val: start: %d, length: %d\n", val.data.start, val.data.length);
    if (val.data.isValid == false)
        return (JN_KeyValuePair) { .value.type = JN_VT_INVALID };

    skip_whitespace(j, pos);
    JN_Data com = skip_next_chars(j, pos, ",}");
    skip_whitespace(j, pos);
    // printf("com: start: %d, length: %d\n", com.start, com.length);
    if (com.isValid == false)
        return (JN_KeyValuePair) { .value.type = JN_VT_INVALID };

    if (j->data[com.start] == '}')
        *is_last = true;

    // printf("\nafter: %s\n", j->data + *pos);
    return (JN_KeyValuePair) {
        .key = key,
        .value = val
    };
}

void jn_get_info(JN* j, uint length)
{
    if (j->data == NULL) {
        j->isValid = false;
        return;
    }

    size_t pos = 0;

    skip_whitespace(j, &pos);

    if (pos < 0) {
        fprintf(stderr, "First position is corrupt!\n");
        j->isValid = false;
        return;
    }

    if (j->data[pos] != '[' && j->data[pos] != '{') {
        skip_whitespace(j, &pos);
        // fprintf(stderr, "Parsing non object/array: %s\n", j->data + pos);
        JN_Value value = skip_next_value(j, &pos);
        skip_whitespace(j, &pos);
        j->type = value.type;
        j->literal = value.data;
        j->isValid = value.data.isValid;
        if (j->isValid && pos != length) {
            fprintf(stderr, "Corrupt length! pos:%zu != length:%d\n", pos, length);
            j->isValid = false;
        }
        return;
    }

    if (j->data[pos] == '[') {
        // fprintf(stderr, "Parsing array: %s\n", j->data + pos);

        j->type = JN_VT_ARRAY;
        j->isValid = true;
        j->list.count = 0;
        j->list.capacity = 16;
        j->list.items = malloc(sizeof(JN_ValueList) * j->list.capacity);

        JN_Value vl = { 0 };
        bool found_array_end = false;

        pos++;
        skip_whitespace(j, &pos);

        if (j->data[pos] == ']') {
            ++pos;
            skip_whitespace(j, &pos);
        } else {
            while ((vl = skip_next_value(j, &pos)).type != JN_VT_NONE) {
                if (j->list.count >= j->list.capacity) {
                    j->list.capacity = j->list.count * 2;
                    j->list.items = realloc(j->list.items, sizeof(JN_ValueList) * j->list.capacity);
                }

                if (vl.type == JN_VT_INVALID || vl.data.isValid == false) {
                    fprintf(stderr, "Error: Invalid value found!\n");
                    j->isValid = false;
                    break;
                }

                if (vl.type != JN_VT_WHITESPACE)
                    j->list.items[j->list.count++] = vl;

                skip_whitespace(j, &pos);
                JN_Data com = skip_next_chars(j, &pos, ",]");
                if (com.isValid == false)
                    break;
                skip_whitespace(j, &pos);
                if (j->data[com.start] == ']') {
                    found_array_end = true;
                    break;
                }
            }
            if (found_array_end == false)
                j->isValid = false;
        }
    } else if (j->data[pos] == '{') {
        // fprintf(stderr, "Parsing object: %s\n", j->data + pos);

        j->type = JN_VT_OBJECT;
        j->isValid = true;
        j->kvs.count = 0;
        j->kvs.capacity = 16;
        j->kvs.items = malloc(sizeof(JN_KeyValuePair) * j->kvs.capacity);

        JN_KeyValuePair kv = { 0 };
        bool is_last = false;

        pos++;
        skip_whitespace(j, &pos);

        if (j->data[pos] == '}') {
            ++pos;
            skip_whitespace(j, &pos);
        } else {
            while ((kv = skip_next_kv(j, &pos, &is_last)).value.type != JN_VT_NONE) {
                if (j->kvs.count >= j->kvs.capacity) {
                    j->kvs.capacity = j->kvs.count * 2;
                    j->kvs.items = realloc(j->kvs.items, sizeof(JN_KeyValuePair) * j->kvs.capacity);
                }

                if (kv.value.type == JN_VT_INVALID) {
                    fprintf(stderr, "Error: Invalid key/value pair found!\n");
                    j->isValid = false;
                    return;
                }

                if (kv.value.type != JN_VT_WHITESPACE)
                    j->kvs.items[j->kvs.count++] = kv;

                if (is_last)
                    break;
            }
        }
    }

    if (j->isValid && pos == length) {
        // printf("isValid: %zu == %u (%d)\n", pos, length, j->isValid);
    } else
        j->isValid = false;

    return;
}

JN jn_alloc(char* json_data, uint length)
{
    JN j = { 0 };
    j.data = json_data;
    j.data_length = length;

    j.isValid = false;
    if (length > 0 && j.data != NULL)
        jn_get_info(&j, length);
    return j;
}

void jn_free(JN* jn)
{
    if (jn->type == JN_VT_OBJECT)
        free(jn->kvs.items);
    else if (jn->type == JN_VT_ARRAY)
        free(jn->list.items);
}

bool jn_is_key(JN* j, JN_KeyValuePair* kv, char* key)
{
    if (key == NULL || kv == NULL || j == NULL)
        return false;

    size_t key_len = strlen(key);
    if (key_len == 0)
        return false;

    JN_Data k = kv->key;
    if (k.isValid == false || k.length == 0)
        return false;

    if (key_len != k.length - 2)
        return false;

    for (size_t i = 0; i < key_len; i++) {
        if (j->data[kv->key.start + 1 + i] != key[i]) {
            return false;
        }
    }

    return true;
}

JN_Value jn_obj_get_value(JN* j, char* key)
{
    JN_Value value = (JN_Value) { .type = JN_VT_INVALID };
    if (j == NULL || j->type != JN_VT_OBJECT || key == NULL)
        return value;

    for (size_t i = 0; i < j->kvs.count; i++) {
        if (jn_is_key(j, &j->kvs.items[i], key)) {
            return j->kvs.items[i].value;
        };
    }

    value.type = JN_VT_NONE;
    return value;
}

JN jn_obj_get(JN* j, char* key)
{
    JN new = (JN) { .type = JN_VT_INVALID };
    if (j == NULL || j->type != JN_VT_OBJECT || key == NULL)
        return new;

    for (size_t i = 0; i < j->kvs.count; i++) {
        if (jn_is_key(j, &j->kvs.items[i], key)) {
            JN_Value value = j->kvs.items[i].value;
            new = jn_alloc((char*)j->data + value.data.start, value.data.length);
            return new;
        };
    }

    new.type = JN_VT_NONE;
    return new;
}

JN jn_arr_get(JN* j, size_t idx)
{
    JN new = (JN) { .type = JN_VT_INVALID };
    if (j == NULL || j->type != JN_VT_ARRAY || idx >= j->list.count)
        return new;

    JN_Value value = j->list.items[idx];
    new = jn_alloc((char*)j->data + value.data.start, value.data.length);
    return new;
}

void jn_print_full(JN* j)
{
    if (j->isValid) {
        if (j->type == JN_VT_OBJECT) {
            printf("{\n");
            for (size_t i = 0; i < j->kvs.count; i++) {
                printf("\t%.*s: ",
                    j->kvs.items[i].key.length, j->data + j->kvs.items[i].key.start);

                if (j->kvs.items[i].value.data.length < 50)
                    printf("%.*s",
                        j->kvs.items[i].value.data.length, j->data + j->kvs.items[i].value.data.start);
                else
                    printf("%s", JN_VT_String[j->kvs.items[i].value.type]);

                (i < j->kvs.count - 1) ? printf(",\n") : printf("\n");
            }
            printf("}\n");
        } else if (j->type == JN_VT_ARRAY) {
            printf("[\n");
            for (size_t i = 0; i < j->list.count; i++) {
                printf("\t%.*s",
                    j->list.items[i].data.length, j->data + j->list.items[i].data.start);

                printf((i < j->list.count - 1) ? (",\n") : ("\n"));
            }
            printf("]\n");
        } else if (j->type != JN_VT_INVALID) {
            printf("%.*s\n", j->literal.length, j->data + j->literal.start);
        } else {
            fprintf(stderr, "Invalid data\n");
        }
    }
}

void jn_print(JN* j)
{
    if (j->isValid) {
        if (j->type == JN_VT_OBJECT) {
            printf("{\n");
            for (size_t i = 0; i < j->kvs.count; i++) {
                printf("\t%.*s :: %s",
                    j->kvs.items[i].key.length, j->data + j->kvs.items[i].key.start,
                    JN_VT_String[j->kvs.items[i].value.type]);

                (i < j->kvs.count - 1) ? printf(",\n") : printf("\n");
            }
            printf("}\n");
        } else if (j->type == JN_VT_ARRAY) {
            printf("[\n");
            for (size_t i = 0; i < j->list.count; i++) {
                printf("\t%s",
                    JN_VT_String[j->list.items[i].type]);

                printf((i < j->list.count - 1) ? (",\n") : ("\n"));
            }
            printf("]\n");
        } else if (j->type != JN_VT_INVALID) {
            printf("%s\n", JN_VT_String[j->type]);
        } else {
            fprintf(stderr, "Invalid data!\n");
        }
    }
}


#endif // JN_IMPLEMENTATION
#endif // JN_H_
