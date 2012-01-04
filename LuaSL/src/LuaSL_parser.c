#include "LuaSL_LSL_tree.h"
#include <stdio.h>


int main(void)
{
    char test[]=" 4 + 2*10 + 3*( 5 + 1 )";
    SExpression *e = NULL;
    int result = 0;

    e = getAST(test);
    result = evaluate(e);
    printf("Result of '%s' is %d\n", test, result);
    deleteExpression(e);

    return 0;
}

