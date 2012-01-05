#include "LuaSL_LSL_tree.h"
#include <stdio.h>


int main(void)
{
    const char test[] = " 4 + 2*10 + 3*( 5 + 1 )";
    LSL_Expression *exp;

    if ((exp = newTree(test)))
    {
	int result = evaluateExpression(exp, 0);

	printf("Result of '%s' is %d\n", test, result);
	outputExpression(exp);
	printf("\n");
	burnLSLExpression(exp);
    }

    return 0;
}

