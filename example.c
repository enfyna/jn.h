#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define JN_IMPLEMENTATION
#include "jn.h"

int main(int argc, char* argv[])
{
    (void)argv;

    char str_obj[] = "{\"id\":1,\"companyId\":1,\"name\":\"Warehouse1\",\"location\":{\"x\":6,\"y\":7},\"monthlyExpense\":120}";
    char str_arr[] = "[{\"id\":1,\"companyId\":1,\"name\":\"Warehouse1\",\"location\":{\"x\":6,\"y\":7},\"monthlyExpense\":120}]";

    JN j = { 0 };

    if (argc == 2)
        j = jn_alloc(str_obj, strlen(str_obj));
    else
        j = jn_alloc(str_arr, strlen(str_arr));

    printf("---------------------------\n");
    jn_print_full(&j);
    printf("---------------------------\n");

    if (j.type == JN_VT_ARRAY) {
        JN temp = j;
        j = jn_arr_get(&j, 0);
        jn_free(&temp);
    }

    char* keys[] = {
        NULL,
        "non_existing_key",
        "id",
        "companyId",
        "name",
        "monthlyExpense",
        "location",
    };
    size_t key_count = sizeof(keys) / sizeof(keys[0]);

    for (size_t i = 0; i < key_count; i++) {
        JN value = jn_obj_get(&j, keys[i]);
        if (value.type == JN_VT_INVALID) {
            printf("%s -> Invalid arguments!\n", keys[i]);
        } else if (value.type == JN_VT_NONE) {
            printf("%s -> Not found!\n", keys[i]);
        } else {
            printf("%s -> ", keys[i]);
            jn_print_full(&value);
        }
        jn_free(&value);
    }

    printf("---------------------------\n");

    JN location = jn_obj_get(&j, "location");
    for (size_t i = 0; i < location.kvs.count; i++) {
        JN_KeyValuePair kv = location.kvs.items[i];
        printf("\t%.*s :: %.*s (%s)\n",
            jn_data_str(location, kv.key),
            jn_data_str(location, kv.value.data),
            JN_VT_String[kv.value.type]);
    }

    printf("---------------------------\n");

    jn_free(&location);
    jn_free(&j);

    printf("\n\n*** *** *** *** *** *** ***\n\n");

    char json_arr[] = "[{\"nums\":[1,8,12],\"companyName\":\"Neovim\",\"auth\":\"cXdlOjEyMw==\"},"
                      "{\"nums\":[1,4],\"company\":\"GNU\",\"authkey\":\"38912*==\"},"
                      "[{\"nums\":[1,2,3],\"company\":\"Linux\",\"authkey\":\"asdjlkasdjlk=\"},{\"nums\":4,\"company\":\"Emacs\",\"authkey\":\"98ad0*sdsaj\"}],"
                      "{\"nums\":[[1],2,[3],4,5],\"comp\":\"XFCE\",\"token\":\"asjdkl==\"}]";

    JN ja = jn_alloc(json_arr, strlen(json_arr));

    for (size_t i = 0; i < ja.list.count; i++) {
        JN jo = jn_arr_get(&ja, i);
        if (jo.type == JN_VT_OBJECT)
            jn_print_full(&jo);
        else if (jo.type == JN_VT_ARRAY) {
            for (size_t k = 0; k < jo.list.count; k++) {
                JN je = jn_arr_get(&jo, k);
                jn_print_full(&je);
                jn_free(&je);
            }
        }
        printf("---------------------------\n");
        jn_free(&jo);
    }

    jn_free(&ja);

    return 0;
}
