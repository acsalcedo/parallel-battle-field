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

    //Se inicializa MPI y se le pasa los argumentos del programa
    MPI_Comm_size(MPI_COMM_WORLD, &numeroProcesos);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    fscanf(file, "%i", &size);      //Lee el tamano del campo.
    fscanf(file, "%i", &targets);   //Lee la cantidad de blancos.

    int sizeTargets = targets / numeroProcesos;
    int remain = targets % numeroProcesos;

    if (id < remain)
        sizeTargets++;

    blancos = (struct Target *) malloc(sizeTargets * sizeof(struct Target));

    if (blancos == NULL) {
        fprintf(stderr, "No hay memoria\n");
        exit(1);
    }

    /* Lee todas las lineas relevantes a los blancos, guarda el blanco
     * que le pertenece al proceso. */
    int j = 0;
    for (i = 0; i < targets; i++) {
        if (i % numeroProcesos == id) {
            fscanf(file, "%i %i %i", &(blancos[j].coordx),
                                        &(blancos[j].coordy),
                                            &(blancos[j].strength));
            blancos[j].oldStrength = blancos[j].strength;
            j++;
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
    struct Resultado resultado[sizeTargets];

    for (j = 0; j < sizeTargets; ++j) {
        for (i = 0; i < numAttacks; i++) {
            struct Attack singleAttack = attacks[i];
            radioX_Sup = singleAttack.coordx + singleAttack.radius;
            radioY_Sup = singleAttack.coordy + singleAttack.radius;
            radioX_Inf = singleAttack.coordx - singleAttack.radius;
            radioY_Inf = singleAttack.coordy - singleAttack.radius;

            if ((radioX_Inf <= blancos[j].coordx && blancos[j].coordx <= radioX_Sup)
                && (radioY_Inf <= blancos[j].coordy && blancos[j].coordy <= radioY_Sup)) {

                // Si el blanco es militar
                if (blancos[j].oldStrength < 0) {
                    blancos[j].strength += singleAttack.power;

                    // Si el blanco es destruido asigna 0
                    if (blancos[j].strength >= 0)
                        blancos[j].strength = 0;

                // Si el blanco es civil
                } else {
                    blancos[j].strength -= singleAttack.power;

                    // Si el blanco es destruido asigna 0
                    if (blancos[j].strength <= 0)
                        blancos[j].strength = 0;
                }
            }
        }
        resultado[j].oldStrength = blancos[j].oldStrength;
        resultado[j].strength = blancos[j].strength;
    }

    //El proceso 0 recibe los resultados de los otros procesos y los imprime.
    if (id == 0) {
        struct Resultado respuesta[targets];

        for (j = 0; j < sizeTargets; ++j) {
            respuesta[j] = resultado[j];
        }

        //Recibe la respuestas
        for (i = j; i < targets; ++i) {
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
            // Si el blanco es MT
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
        printf("Military Targets totally destroyed: %3d\n", destruidos_MT);
        printf("Military Targets partially destroyed: %d\n", parcial_destruidos_MT);
        printf("Military Targets not affected: %8d\n", intactos_MT);
        printf("Civilian Targets totally destroyed: %3d\n", destruidos_CT);
        printf("Civilian Targets partially destroyed: %d\n", parcial_destruidos_CT);
        printf("Civilian Targets not affected: %8d\n", intactos_CT);
    } else {
        //Manda los resultados al proceso 0.
        for (i = 0; i < sizeTargets; ++i) {
            MPI_Send(&resultado[i],2,MPI_INT,0,0,MPI_COMM_WORLD);
        }
    }
    free(blancos);
    free(attacks);
    MPI_Finalize();
    return 0;
}
