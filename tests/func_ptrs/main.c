#include <stdio.h>

int add(int a, int b) { return a + b; }
int subtract(int a, int b) { return a - b; }
int multiply(int a, int b) { return a * b; }
int divide(int a, int b) { return b != 0 ? a / b : 0; }

int (*operations[4])(int, int) = { add, subtract, multiply, divide };

int main() {
    int x = 10, y = 5;

    printf("Add: %d\n", operations[0](x, y));       // add
    printf("Subtract: %d\n", operations[1](x, y));  // subtract
    printf("Multiply: %d\n", operations[2](x, y));  // multiply
    printf("Divide: %d\n", operations[3](x, y));    // divide

    return 0;
}
