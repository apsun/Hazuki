#include <stdio.h>
#include <stdlib.h>

extern void test_utils(void);
extern void test_map(void);
extern void test_vector(void);

int
main()
{
    test_vector();
    test_map();
    test_utils();
    printf("All tests passed!\n");
    getchar();
    return 0;
}
