#include <stdio.h>
#include <stdlib.h>

int** open_map(char *filename, int size)
{
    FILE* f = fopen(filename, "r");

    int** map = (int**) malloc(size * sizeof(int*));

    for (int y=0; y < size; ++y ) {
        map[y] = (int*)malloc(size * sizeof(int));
        for (int x=0; x < size; ++x ) {
            int value = 0;
            fscanf(f, "%d", &value);
            map[y][x] = value;
        }
    }
    fclose( f );
	return (map);
}
