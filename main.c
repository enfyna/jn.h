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
    JN_Obj* o = &j->items[0];

    // Method 2 - If you know if it is a object get it directly
    // JN_Obj o = jn_obj(warehouse);

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
    printf("%s -> %s\n", "non_existing_key", jo_value(o, "non_existing_key"));
    printf("---------------------------\n");

    JN_Obj loc = jn_obj(jo_value(o, "location"));
    for (int i = 0; i < loc.count; i++) {
        printf("\"%s\": %s\n", loc.items[i].key, loc.items[i].value);
    }
    printf("---------------------------\n");

    ja_free(j);

    printf("\n\n*** *** *** *** *** *** ***\n\n");

    char json_arr[] = "[{\"nums\":[1,8,12],\"companyName\":\"Neovim\",\"auth\":\"cXdlOjEyMw==\"},"
                      "{\"nums\":[1,4],\"company\":\"GNU\",\"authkey\":\"38912*==\"},"
                      "[{\"nums\":[1,2,3],\"company\":\"Linux\",\"authkey\":\"asdjlkasdjlk=\"},{\"nums\":4,\"company\":\"Emacs\",\"authkey\":\"98ad0*sdsaj\"}],"
                      "{\"nums\":[[1],2,[3],4,5],\"comp\":\"XFCE\",\"token\":\"asjdkl==\"}]";

    JN_Arr* ja = jn_arr(json_arr);

    for (int i = 0; i < ja->count; i++) {
        JN_Obj* jo = &ja->items[i];
        for (int k = 0; k < jo->count; k++) {
            printf("\"%s\": %s\n", jo->items[k].key, jo->items[k].value);
        }
        printf("---------------------------\n");
    }

    for (int i = 0; i < ja->arr_count; i++) {
        JN_Arr* ji = ja->arrays[i];
        for (int k = 0; k < ji->count; k++) {
            JN_Obj* jo = &ji->items[k];
            for (int j = 0; j < jo->count; j++) {
                printf("\"%s\": %s\n", jo->items[j].key, jo->items[j].value);
            }
            printf("---------------------------\n");
        }
    }

    ja_free(ja);

#define LIMITS
#ifdef LIMITS
    char body[] = "\
[\
[{\"id\":1,\"companyId\":1,\"name\":\"Depo1\",\"location\":{\"x\":0,\"y\":0},\"monthlyExpense\":101}],\
[{\"id\":2,\"companyId\":2,\"name\":\"Depo2\",\"location\":{\"x\":1,\"y\":1},\"monthlyExpense\":102}],\
[{\"id\":3,\"companyId\":3,\"name\":\"Depo3\",\"location\":{\"x\":2,\"y\":2},\"monthlyExpense\":103}],\
[{\"id\":4,\"companyId\":4,\"name\":\"Depo4\",\"location\":{\"x\":3,\"y\":3},\"monthlyExpense\":104}],\
[{\"id\":5,\"companyId\":5,\"name\":\"Depo5\",\"location\":{\"x\":4,\"y\":4},\"monthlyExpense\":105}],\
[{\"id\":6,\"companyId\":6,\"name\":\"Depo6\",\"location\":{\"x\":5,\"y\":5},\"monthlyExpense\":106}],\
[{\"id\":7,\"companyId\":7,\"name\":\"Depo7\",\"location\":{\"x\":6,\"y\":6},\"monthlyExpense\":107}],\
[{\"id\":8,\"companyId\":8,\"name\":\"Depo8\",\"location\":{\"x\":7,\"y\":7},\"monthlyExpense\":108}],\
[{\"id\":9,\"companyId\":9,\"name\":\"Depo9\",\"location\":{\"x\":8,\"y\":8},\"monthlyExpense\":109}],\
[{\"id\":10,\"companyId\":10,\"name\":\"Depo10\",\"location\":{\"x\":6,\"y\":8},\"monthlyExpense\":143}],\
[{\"id\":11,\"companyId\":11,\"name\":\"Depo11\",\"location\":{\"x\":30,\"y\":70},\"monthlyExpense\":185}],\
\
[{\"id\":12,\"companyId\":12,\"name\":\"Depo12\",\"location\":{\"x\":72,\"y\":402},\"monthlyExpense\":1127},\
{\"id\":13,\"companyId\":13,\"name\":\"Depo13\",\"location\":{\"x\":82,\"y\":412},\"monthlyExpense\":1237},\
{\"id\":14,\"companyId\":14,\"name\":\"Depo14\",\"location\":{\"x\":92,\"y\":422},\"monthlyExpense\":1347},\
{\"id\":15,\"companyId\":15,\"name\":\"Depo15\",\"location\":{\"x\":102,\"y\":702},\"monthlyExpense\":4427},\
{\"id\":16,\"companyId\":16,\"name\":\"Depo16\",\"location\":{\"x\":112,\"y\":802},\"monthlyExpense\":5527},\
{\"id\":13,\"companyId\":13,\"name\":\"Depo13\",\"location\":{\"x\":82,\"y\":412},\"monthlyExpense\":1237},\
{\"id\":14,\"companyId\":14,\"name\":\"Depo14\",\"location\":{\"x\":92,\"y\":422},\"monthlyExpense\":1347},\
{\"id\":15,\"companyId\":15,\"name\":\"Depo15\",\"location\":{\"x\":102,\"y\":702},\"monthlyExpense\":4427},\
{\"id\":16,\"companyId\":16,\"name\":\"Depo16\",\"location\":{\"x\":112,\"y\":802},\"monthlyExpense\":5527},\
{\"id\":13,\"companyId\":13,\"name\":\"Depo13\",\"location\":{\"x\":82,\"y\":412},\"monthlyExpense\":1237},\
{\"id\":14,\"companyId\":14,\"name\":\"Depo14\",\"location\":{\"x\":92,\"y\":422},\"monthlyExpense\":1347},\
{\"id\":15,\"companyId\":15,\"name\":\"Depo15\",\"location\":{\"x\":102,\"y\":702},\"monthlyExpense\":4427},\
{\"id\":16,\"companyId\":16,\"name\":\"Depo16\",\"location\":{\"x\":112,\"y\":802},\"monthlyExpense\":5527},\
{\"id\":17,\"companyId\":17,\"name\":\"Depo17\",\"location\":{\"x\":122,\"y\":902},\"monthlyExpense\":6627},\
{\"id\":17,\"companyId\":17,\"name\":\"Depo17\",\"location\":{\"x\":122,\"y\":902},\"monthlyExpense\":6627},\
{\"id\":17,\"companyId\":17,\"name\":\"Depo17\",\"location\":{\"x\":122,\"y\":902},\"monthlyExpense\":6627},\
{\"id\":17,\"companyId\":17,\"name\":\"Depo17\",\"location\":{\"x\":122,\"y\":902},\"monthlyExpense\":6627},\
{\"id\":17,\"companyId\":17,\"name\":\"Depo17\",\"location\":{\"x\":122,\"y\":902},\"monthlyExpense\":6627},\
{\"id\":17,\"companyId\":17,\"name\":\"Depo17\",\"location\":{\"x\":122,\"y\":902},\"monthlyExpense\":6627},\
{\"id\":18,\"companyId\":18,\"name\":\"Depo18\",\"location\":{\"x\":132,\"y\":1002},\"monthlyExpense\":7127}],\
]"
    "";

    JN_Arr* h = jn_arr(body);

    for (int i = 0; i < h->count; i++) {
        JN_Obj* jo = &h->items[i];
        for (int k = 0; k < jo->count; k++) {
            printf("\"%s\": %s\n", jo->items[k].key, jo->items[k].value);
        }
        printf("---------------------------\n");
    }

    for (int i = 0; i < h->arr_count; i++) {
        JN_Arr* ji = h->arrays[i];
        printf("===========================\n");
        for (int k = 0; k < ji->count; k++) {
            JN_Obj* jo = &ji->items[k];
            for (int j = 0; j < jo->count; j++) {
                printf("\"%s\": %s\n", jo->items[j].key, jo->items[j].value);
            }
            printf("---------------------------\n");
        }
    }
    ja_free(h);
#endif

    return 0;
}
