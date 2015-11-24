#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

struct Attack {
    int coordx;
    int coordy;
    int radius;
    int power;
};

struct Target {
    int coordx;
    int coordy;
    int strength;
    int oldStrength;
};

struct Resultado {
    int strength;
    int oldStrength;
};


/*
    Para correr MPI: $ mpirun -np 4 parallel-battle-field test
    Para compilar con MPI : $ mpicc battle-field.c -o parallel-battle-field -std=c99 -g
*/

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

    // inicializamos MPI y le pasamos los argumentos del programa
    MPI_Comm_size(MPI_COMM_WORLD, &numeroProcesos);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    fscanf(file, "%i", &size);      //Lee el tamano del campo.
    fscanf(file, "%i", &targets);   //Lee la cantidad de blancos.

    //int inicio = id*(targets / numeroProcesos);
    //int fin = inicio + (targets / numeroProcesos) - 1;

    // Lee todas las lineas relevantes a los blancos, guarda el blanco
    // que le pertenece al proceso.

    blancos = (struct Target *) malloc(sizeof(struct Target));
    if (blancos == NULL)
    {
        fprintf(stderr, "No hay memoria\n");
        exit(1);
    }

    for (i = 0; i < targets; i++) {
        if (i == id) {
            fscanf(file, "%i %i %i", &(blancos->coordx), &(blancos->coordy), &(blancos->strength));
            blancos->oldStrength = blancos->strength;
        }
        else
            fscanf(file, "%*i %*i %*i");
    }

    fscanf(file, "%i", &numAttacks);    //Lee el numero de ataques.

    // Arreglo con los ataques.
    attacks = malloc(numAttacks * sizeof(struct Attack));
    if (attacks == NULL)
    {
        fprintf(stderr, "No hay memoria\n");
        free(blancos);
        exit(1);
    }

    // Almacena los valores de los ataques en el arreglo de ataques.
    for (i = 0; i < numAttacks; i++) {
        struct Attack singleAttack;
        fscanf(file, "%i %i %i %i", &singleAttack.coordx, &singleAttack.coordy,
                                    &singleAttack.radius, &singleAttack.power);
        attacks[i] = singleAttack;
    }

    // Imprime la informacion.
    /*printf("Size: %i, Targets: %i \n", size, targets);
    printf("Target coordinates: (%i,%i) Strength: %i\n", coordx, coordy, strength);
    printf("Attacks: %i\n", numAttacks);

    for (i = 0; i < numAttacks; i++) {
        struct Attack singleAttack = attacks[i];
        printf("%i %i %i %i\n", singleAttack.coordx, singleAttack.coordy,
                                singleAttack.radius, singleAttack.power);
    }*/


    // Simular el ataque para un proceso (id 0)
    printf("Target coordinates: (%i,%i) Strength: %i\n",
            blancos->coordx, blancos->coordy, blancos->strength);
    int radioX_Sup = 0, radioY_Sup = 0;
    int radioX_Inf = 0, radioY_Inf = 0;
    int destruidos_CT = 0, parcial_destruidos_CT = 0, intactos_CT = 0;
    int destruidos_MT = 0, parcial_destruidos_MT = 0, intactos_MT = 0;

    for (i = 0; i < numAttacks; i++) {
        struct Attack singleAttack = attacks[i];
        radioX_Sup = singleAttack.coordx + singleAttack.radius;
        radioY_Sup = singleAttack.coordy + singleAttack.radius;
        radioX_Inf = singleAttack.coordx - singleAttack.radius;
        radioY_Inf = singleAttack.coordy - singleAttack.radius;

        if ((radioX_Inf <= blancos->coordx && blancos->coordx <= radioX_Sup)
            && (radioY_Inf <= blancos->coordy && blancos->coordy <= radioY_Sup)) {

            //printf("B%d -> Power: %d\n", i, singleAttack.power);
            // Si el blanco es militar
            if (blancos->oldStrength < 0)
            {
                blancos->strength += singleAttack.power;
                // Si el blanco es destruido asigna 0
                if (blancos->strength >= 0)
                    blancos->strength = 0;
            }
            else
            {
                blancos->strength -= singleAttack.power;
                // Si el blanco es destruido asigna 0
                if (blancos->strength <= 0)
                    blancos->strength = 0;
            }
        }

    }

    resultado.oldStrength = blancos->oldStrength;
    resultado.strength = blancos->strength;

    if (id == 0)
    {
        struct Resultado respuesta[targets];
        respuesta[0] = resultado;
        for (i = 1; i < numeroProcesos; ++i)
        {
            // Recibo la respuesta
            MPI_Recv(&respuesta[i], 1,
                            MPI_2INT,
                                MPI_ANY_SOURCE,
                                    MPI_ANY_TAG,
                                        MPI_COMM_WORLD,
                                            MPI_STATUS_IGNORE);
        }

        for (i = 0; i < targets; ++i)
        {
            // Si el blanco es CT
            if (respuesta[i].oldStrength > 0)
            {
                if (respuesta[i].oldStrength > respuesta[i].strength &&
                    respuesta[i].strength > 0)
                    parcial_destruidos_CT += 1;
                else
                {
                    if (respuesta[i].strength == 0)
                        destruidos_CT += 1;
                    else
                    intactos_CT += 1;
                }

            }
            else
            {
                if (respuesta[i].oldStrength < respuesta[i].strength &&
                    respuesta[i].strength < 0)
                    parcial_destruidos_MT += 1;
                else
                {
                    if (respuesta[i].strength == 0)
                        destruidos_MT += 1;
                    else
                        intactos_MT += 1;
                }
            }
        }

        // Imprimir resultados
        printf("Military Targets totally destroyed: %d\n", destruidos_MT);
        printf("Military Targets partially destroyed: %d\n", parcial_destruidos_MT);
        printf("Military Targets not affected: %d\n", intactos_MT);
        printf("Civilian Targets totally destroyed: %d\n", destruidos_CT);
        printf("Civilian Targets partially destroyed: %d\n", parcial_destruidos_CT);
        printf("Civilian Targets not affected: %d\n", intactos_CT);
    } else
        MPI_Send(&resultado,1,MPI_2INT,0,0,MPI_COMM_WORLD);

    free(blancos);
    free(attacks);
    MPI_Finalize();
    //printf("Resultado:\n");
    //printf("T%d -> Strength: %i\n", blancos->id, blancos->strength);
}
