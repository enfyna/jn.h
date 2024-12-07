#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define JN_IMPLEMENTATION
#include "jn.h"

int main(void) {
    char json[] = "{\"password\":9876543210123,\"companyName\":\"Company\",\"auth\":\"cXdlOjEyMw==\"}";

    // JN_Arr* j = jn_arr(json);
    // JN_Obj* o = j->items[0];
    JN_Obj* o = jn_obj(json);

    printf("---------------------------\n");
    for (int i = 0; i < o->count; i++) {
        printf("\"%s\": %s\n", o->items[i].key, o->items[i].value);
    }
    printf("---------------------------\n");
    printf("%s -> %s\n", "auth", jo_value(o, "auth"));
    printf("%s -> %s\n", "pass", jo_value(o, "password"));
    printf("%s -> %s\n", "comp", jo_value(o, "comp"));
    printf("---------------------------\n");

    char json_arr[] = 
        "[{\"password\":1,\"companyName\":\"Neovim\",\"auth\":\"cXdlOjEyMw==\"},"
        "{\"pass\":2,\"company\":\"Linux\",\"authkey\":\"38912*==\"},"
        "{\"passkey\":3,\"comp\":\"XFCE\",\"token\":\"asjdkl==\"}]"
    ;

    JN_Arr* ja = jn_arr(json_arr);

    for (int i = 0; i < ja->item_count - 1; i++) {
        JN_Obj* jo = ja->items[i];
        for (int k = 0; k < jo->count; k++) {
            printf("\"%s\": %s\n", jo->items[k].key, jo->items[k].value);
        }
        printf("---------------------------\n");
    }
    // ja_free(j);
    ja_free(ja);
    free(o);
    return 0;
}
