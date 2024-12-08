#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#define JN_IMPLEMENTATION
#include "jn.h"

/*
 *  {}
 *  [{}]
 *  [{},{}]
 *  [[],{}]
 *  [[{}],{}]
 *  [[{},{}],{},{}]
 *  [[{},{}],[{},{}],{},{}]
 */

int main(void)
{
    char warehouse[] = "{\"id\":1,\"companyId\":1,\"name\":\"Warehouse1\",\"location\":{\"x\":0,\"y\":0},\"monthlyExpense\":120}";
    (void)warehouse;
    char warehouse_arr[] = "[{\"id\":1,\"companyId\":1,\"name\":\"Warehouse1\",\"location\":{\"x\":0,\"y\":0},\"monthlyExpense\":120}]";
    (void)warehouse_arr;

    // Method 1 - If you dont know if it is a array or object
    JN_Arr* j = jn_arr(warehouse);
    JN_Obj* o = j->items[0];
    
    // Method 2 - If you know if it is a object get it directly
    // JN_Obj* o = jn_obj(warehouse);

    printf("---------------------------\n");
    for (int i = 0; i < o->count; i++) {
        printf("\"%s\": %s\n", o->items[i].key, o->items[i].value);
    }
    printf("---------------------------\n");
    printf("%s -> %s\n", "id", jo_value(o, "id"));
    printf("%s -> %s\n", "companyId", jo_value(o, "companyId"));
    printf("%s -> %s\n", "name", jo_value(o, "name"));
    printf("%s -> %s\n", "monthlyExpense", jo_value(o, "monthlyExpense"));
    printf("%s -> %s\n", "location", jo_value(o, "location"));
    printf("%s -> %s\n", "random", jo_value(o, "random"));
    printf("---------------------------\n");

    JN_Obj* loc = jn_obj(jo_value(o, "location"));
    for (int i = 0; i < loc->count; i++) {
        printf("\"%s\": %s\n", loc->items[i].key, loc->items[i].value);
    }
    printf("---------------------------\n");

    free(loc);
    ja_free(j);

    printf("\n\n*** *** *** *** *** *** ***\n\n");

    char json_arr[] = "[{\"password\":1,\"companyName\":\"Neovim\",\"auth\":\"cXdlOjEyMw==\"},"
                      "{\"pass\":2,\"company\":\"GNU\",\"authkey\":\"38912*==\"},"
                      "[{\"pass\":3,\"company\":\"Linux\",\"authkey\":\"asdjlkasdjlk=\"},{\"pass\":4,\"company\":\"Linux\",\"authkey\":\"98ad0*sdsaj\"}],"
                      "{\"passkey\":[1,2,3,4,5],\"comp\":\"XFCE\",\"token\":\"asjdkl==\"}]";

    JN_Arr* ja = jn_arr(json_arr);

    for (int i = 0; i < ja->item_count; i++) {
        JN_Obj* jo = ja->items[i];
        for (int k = 0; k < jo->count; k++) {
            printf("\"%s\": %s\n", jo->items[k].key, jo->items[k].value);
        }
        printf("---------------------------\n");
    }

    for (int i = 0; i < ja->arr_count; i++) {
        JN_Arr* ji = ja->arrays[i];
        for (int k = 0; k < ji->item_count; k++) {
            JN_Obj* jo = ji->items[k];
            for (int j = 0; j < jo->count; j++) {
                printf("\"%s\": %s\n", jo->items[j].key, jo->items[j].value);
            }
            printf("---------------------------\n");
        }
    }

    ja_free(ja);
    return 0;
}
