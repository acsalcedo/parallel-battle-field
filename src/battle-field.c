#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

//Estructura de un ataque.
struct Attack {
    int coordx;
    int coordy;
    int radius;
    int power;
};

//Estructura de un blanco.
struct Target {
    int coordx;
    int coordy;
    int strength;
    int oldStrength;
};

//Estructura del resultado de un blanco.
struct Resultado {
    int strength;
    int oldStrength;
};

int main (int argc, char **argv) {

    MPI_Init(&argc,&argv);
    FILE *file;
    file = fopen(argv[1],"r");

    if (file == NULL) {
        fprintf(stderr, "No se puede abrir el archivo.\n");
        exit(1);
    }

    int size = 0, targets = 0, numAttacks = 0, numeroProcesos = 0;
    int i = 0, id = 0;

    struct Attack *attacks;
    struct Target *blancos;
    struct Resultado resultado;

    //Se inicializa MPI y se le pasa los argumentos del programa
    MPI_Comm_size(MPI_COMM_WORLD, &numeroProcesos);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    fscanf(file, "%i", &size);      //Lee el tamano del campo.
    fscanf(file, "%i", &targets);   //Lee la cantidad de blancos.

    blancos = (struct Target *) malloc(sizeof(struct Target));

    if (blancos == NULL) {
        fprintf(stderr, "No hay memoria\n");
        exit(1);
    }

    /* Lee todas las lineas relevantes a los blancos, guarda el blanco
     * que le pertenece al proceso. */
    for (i = 0; i < targets; i++) {
        if (i == id) {
            fscanf(file, "%i %i %i", &(blancos->coordx), &(blancos->coordy), &(blancos->strength));
            blancos->oldStrength = blancos->strength;
        } else
            fscanf(file, "%*i %*i %*i");
    }

    fscanf(file, "%i", &numAttacks);    //Lee el numero de ataques.

    //Arreglo con los ataques.
    attacks = malloc(numAttacks * sizeof(struct Attack));
    if (attacks == NULL) {
        fprintf(stderr, "No hay memoria\n");
        free(blancos);
        exit(1);
    }

    //Almacena los valores de los ataques en el arreglo de ataques.
    for (i = 0; i < numAttacks; i++) {
        struct Attack singleAttack;
        fscanf(file, "%i %i %i %i", &singleAttack.coordx, &singleAttack.coordy,
                                    &singleAttack.radius, &singleAttack.power);
        attacks[i] = singleAttack;
    }

    int radioX_Sup = 0, radioY_Sup = 0;
    int radioX_Inf = 0, radioY_Inf = 0;
    int destruidos_CT = 0, parcial_destruidos_CT = 0, intactos_CT = 0;
    int destruidos_MT = 0, parcial_destruidos_MT = 0, intactos_MT = 0;

    /* Hace los calculos de los ataques en los blancos. Cada proceso calcula
     * la fuerza del blanco despues del ataque. */
    for (i = 0; i < numAttacks; i++) {
        struct Attack singleAttack = attacks[i];
        radioX_Sup = singleAttack.coordx + singleAttack.radius;
        radioY_Sup = singleAttack.coordy + singleAttack.radius;
        radioX_Inf = singleAttack.coordx - singleAttack.radius;
        radioY_Inf = singleAttack.coordy - singleAttack.radius;

        if ((radioX_Inf <= blancos->coordx && blancos->coordx <= radioX_Sup)
            && (radioY_Inf <= blancos->coordy && blancos->coordy <= radioY_Sup)) {

            // Si el blanco es militar
            if (blancos->oldStrength < 0) {
                blancos->strength += singleAttack.power;

                // Si el blanco es destruido asigna 0
                if (blancos->strength >= 0)
                    blancos->strength = 0;
            } else {
                blancos->strength -= singleAttack.power;

                // Si el blanco es destruido asigna 0
                if (blancos->strength <= 0)
                    blancos->strength = 0;
            }
        }
    }
    resultado.oldStrength = blancos->oldStrength;
    resultado.strength = blancos->strength;

    //El proceso 0 recibe los resultados de los otros procesos y los imprime.
    if (id == 0) {
        struct Resultado respuesta[targets];
        respuesta[0] = resultado;

        for (i = 1; i < numeroProcesos; ++i) {
            //Recibe la respuesta
            MPI_Recv(&respuesta[i],2,MPI_INT,MPI_ANY_SOURCE,MPI_ANY_TAG,
                     MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        }
        for (i = 0; i < targets; ++i) {
            // Si el blanco es CT
            if (respuesta[i].oldStrength > 0) {

                if (respuesta[i].oldStrength > respuesta[i].strength &&
                    respuesta[i].strength > 0)
                    parcial_destruidos_CT += 1;
                else {
                    if (respuesta[i].strength == 0)
                        destruidos_CT += 1;
                    else
                    intactos_CT += 1;
                }
            } else {

                if (respuesta[i].oldStrength < respuesta[i].strength &&
                    respuesta[i].strength < 0)
                    parcial_destruidos_MT += 1;

                else {
                    if (respuesta[i].strength == 0)
                        destruidos_MT += 1;
                    else
                        intactos_MT += 1;
                }
            }
        }
        //Imprime los resultados
        printf("Military Targets totally destroyed: %d\n", destruidos_MT);
        printf("Military Targets partially destroyed: %d\n", parcial_destruidos_MT);
        printf("Military Targets not affected: %d\n", intactos_MT);
        printf("Civilian Targets totally destroyed: %d\n", destruidos_CT);
        printf("Civilian Targets partially destroyed: %d\n", parcial_destruidos_CT);
        printf("Civilian Targets not affected: %d\n", intactos_CT);

    //Manda el resultado al proceso 0.
    } else
        MPI_Send(&resultado,2,MPI_INT,0,0,MPI_COMM_WORLD);

    free(blancos);
    free(attacks);
    MPI_Finalize();
    return 0;
}
