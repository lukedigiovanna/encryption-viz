#include <stdio.h>
#include <stdlib.h>

// Set's the bit in the given character array by a given bit offset
// the val must be either 0 or 1.
static void set_bit(char* arr, int bit_num, char val) {
    int ind = bit_num / 8;
    int off = bit_num % 8;
    char mask = ~(1 << off);
    arr[ind] = (arr[ind] & mask) ^ (val << off);
}

static char get_bit(char* arr, int bit_num) {
    int ind = bit_num / 8;
    int off = bit_num % 8;
    return (arr[ind] >> bit_num) & 1;
}

/**
 * Performs a single round of the substitution-permutation network cipher.
 * 
 * @param data a byte array of the data to encrypt ("w^{r-1}")
 * @param num_bytes the number of bytes in the data pointer
 * @param substitution a static table to use when performing substitution. substitution is performed in 8-bit blocks
 * @param permutation the permutation table to use; must match the size of the data input
 * @param key the key to use when xoring
*/
void spn_round(char* data, int num_bytes, char substitution[256], int* permutation, char* key) {
    char* new = malloc(num_bytes);

    // Step 1: Sum data with the key
    for (int i = 0; i < num_bytes; i++) {
        new[i] = data[i] ^ key[i];
    }

    // Step 2: Perform substitution on each character.
    for (int i = 0; i < num_bytes; i++) {
        new[i] = substitution[new[i]];
    }

    // Step 3: Perform permutation on the bits (this step writes to the inputted data)
    for (int i = 0; i < num_bytes * 8; i++) {
        set_bit(data, i, get_bit(new, i));
    }

    free(new);
}

// dynamically allocates an array of integers indicating what index permutes to the given index.
// randomly generates the permutation
int* generate_permutation(int num_elements) {
    int n = num_elements * 8;

    int* perm = malloc(n * sizeof(int));

    int i;

    for (i = 0; i < n; i++) {
        perm[i] = i;
    }

    // perform n swaps.
    for (i = 0; i < n; i++) {
        int i1 = rand() % n, i2 = rand() % n;
        int t1 = perm[i1];
        perm[i1] = perm[i2];
        perm[i2] = t1;
    }

    return perm;
}

// generates a 256 char array that maps a char value to its substitution
char* generate_substitution() {
    char* sub = malloc(256);
    for (int i = 0; i < 256; i++) {
        sub[i] = i;
    }
    for (int i = 0; i < 256; i++) {
        int i1 = rand() % 256, i2 = rand() % 256;
        int t1 = sub[i1];
        sub[i1] = sub[i2];
        sub[i2] = t1;
    }
    return sub;
}

char* generate_key(int num_elements) {
    char* key = malloc(num_elements);
    for (int i = 0; i < num_elements; i++) {
        key[i] = rand() % 256;
    }
    return key;
}

int main(int argc, char* argv[]) {
    srand(4);

    if (argc != 2) {
        printf("Usage:\t./spn <file>");
        return -1;
    }

    char* filename = argv[1];
    FILE* file = fopen(filename, "rb");

    fseek(file, 0, SEEK_END);
    int num_elements = ftell(file);
    rewind(file);

    char* data = malloc(num_elements);

    fread(data, 1, num_elements, file);

    char* substitution = generate_substitution();

    int* permutation = generate_permutation(num_elements);
    char* key = generate_key(num_elements);

    spn_round(data, num_elements, substitution, permutation, key);

    for (int i = 0; i < num_elements; i++) {
        printf("%c", data[i]);
    }

    free(data);
    free(permutation);
    free(substitution);
    free(key);

    fclose(file);

    return 0;
}