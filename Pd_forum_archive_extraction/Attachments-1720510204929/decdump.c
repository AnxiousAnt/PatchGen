#include <stdio.h>

int main(void) {
    int c;
    while (EOF != (c = getchar())) {
        printf("%d\n", c);
    }
    return (0);
}
