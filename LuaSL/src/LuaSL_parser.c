#include "LuaSL_LSL_tree.h"
#include <stdio.h>


int main(void)
{
    const char test[] = " 4 + 2*10 + 3*( 5 + 1 )";
    SExpression *e = NULL;

    if ((e = getAST(test)))
    {
	int result = evaluate(e);

	printf("Result of '%s' is %d\n", test, result);
	deleteExpression(e);
    }

    return 0;
}

