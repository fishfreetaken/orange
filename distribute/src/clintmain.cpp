
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
struct abc{
    int b;
    float c;
    size_t f;
    uint16_t a;
    int m;
    char ss;
} ab;


class fff{
    int b;
    float c;
    size_t f;
    uint16_t a;
    int m;
    char ss;
};

int main()
{
    fff at;
    printf("%d\n",sizeof(at));
    printf("%d\n",sizeof(ab));
    printf("%d\n",sizeof(size_t));
    return 0;
}