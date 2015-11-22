#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

struct Attack {
    int coordx;
    int coordy;
    int radius;
    int power;
};

int main (int argc, char **argv) {

    FILE *file;
    file = fopen(argv[1],"r");

    if (file == NULL) {
        fprintf(stderr, "No se puede abrir el archivo.\n");
        exit(1);
    }
    int size = 0, targets = 0, numAttacks = 0;
    int coordx = 0, coordy = 0, strength = 0;
    struct Attack *attacks;

    int id = 2; //TODO: Cambiar al ID del proceso.
    int i;

    fscanf(file, "%i", &size);      //Lee el tamano del campo.
    fscanf(file, "%i", &targets);   //Lee la cantidad de blancos.

    // Lee todas las lineas relevantes a los blancos, guarda el blanco
    // que le pertenece al proceso.
    for (i = 0; i < targets; i++) {
        if (i == id)
            fscanf(file, "%i %i %i", &coordx, &coordy, &strength);
        else
            fscanf(file, "%*i %*i %*i");
    }

    fscanf(file, "%i", &numAttacks);    //Lee el numero de ataques.

    // Arreglo con los ataques.
    attacks = malloc(numAttacks * sizeof(struct Attack));

    // Almacena los valores de los ataques en el arreglo de ataques.
    for (i = 0; i < numAttacks; i++) {
        struct Attack singleAttack;
        fscanf(file, "%i %i %i %i", &singleAttack.coordx, &singleAttack.coordy,
                                    &singleAttack.radius, &singleAttack.power);
        attacks[i] = singleAttack;
    }

    // Imprime la informacion.
    printf("Size: %i, Targets: %i \n", size, targets);
    printf("Target coordinates: (%i,%i) Strength: %i\n", coordx, coordy, strength);
    printf("Attacks: %i\n", numAttacks);

    for (i = 0; i < numAttacks; i++) {
        struct Attack singleAttack = attacks[i] ;
        printf("%i %i %i %i\n", singleAttack.coordx, singleAttack.coordy,
                                singleAttack.radius, singleAttack.power);
    }
}
