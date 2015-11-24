#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

struct Attack {
    int coordx;
    int coordy;
    int radius;
    int power;
};

enum tipoBlanco { CT , MT};

struct Target {
    int id;
    int coordx;
    int coordy;
    int strength;
    enum tipoBlanco tipo;
};



int main (int argc, char **argv) {

    //MPI_Init(&argc,&argv);
    FILE *file;
    file = fopen(argv[1],"r");

    if (file == NULL) {
        fprintf(stderr, "No se puede abrir el archivo.\n");
        exit(1);
    }

    int size = 0, targets = 0, numAttacks = 0;
    int id = 2, numeroProcesos = 0; //TODO: Cambiar al ID del proceso.
    int i = 0;
    // Blanco para el proceso 0
    //int coordx = 0, coordy = 0, strength = 0;
    struct Attack *attacks;
    struct Target *blancos;
    MPI_Status Stat;

    // inicializamos MPI y le pasamos los argumentos del programa
    //MPI_Comm_size(MPI_COMM_WORLD, &numeroProcesos);
    //MPI_Comm_rank(MPI_COMM_WORLD, &Id);

    fscanf(file, "%i", &size);      //Lee el tamano del campo.
    fscanf(file, "%i", &targets);   //Lee la cantidad de blancos.

    //int inicio = Id*(targets / numeroProcesos);
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
            blancos->id = id;
            if (blancos->strength > 0) {
                blancos->tipo = CT;
            }
            else {
                blancos->tipo = MT;
            }
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


    // Simular el ataque para un proceso (ID 0)
    printf("Target coordinates: (%i,%i) Strength: %i\n",
            blancos->coordx, blancos->coordy, blancos->strength);
    int radioX_Sup = 0, radioY_Sup = 0;
    int radioX_Inf = 0, radioY_Inf = 0;

    for (i = 0; i < numAttacks; i++) {
        struct Attack singleAttack = attacks[i];
        radioX_Sup = singleAttack.coordx + singleAttack.radius;
        radioY_Sup = singleAttack.coordy + singleAttack.radius;
        radioX_Inf = singleAttack.coordx - singleAttack.radius;
        radioY_Inf = singleAttack.coordy - singleAttack.radius;

        if ((radioX_Inf <= blancos->coordx && blancos->coordx <= radioX_Sup)
            && (radioY_Inf <= blancos->coordy && blancos->coordy <= radioY_Sup)) {

            printf("B%d -> Power: %d\n", i, singleAttack.power);
            if (blancos->tipo == MT) {
                blancos->strength += singleAttack.power;
                // Si el blanco es destruido asigna 0
                if (blancos->strength >= 0)
                    blancos->strength = 0;
            }
            else {
                blancos->strength -= singleAttack.power;
                // Si el blanco es destruido asigna 0
                if (blancos->strength <= 0)
                    blancos->strength = 0;
            }
        }

    }

/*    if ( id == 0) {
        int respuesta[targets];
        respuesta[0]= strength;
        for (i = 1; i < numeroProcesos; ++i)
        {
            // Hago una prueba para ver quien envio el mensaje
            MPI_Probe(MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&Stat);
            // Segun quien lo envie, habra resuelto una parte diferente de la matriz
            inicio = Stat.MPI_SOURCE*(targets/numeroProcesos);
            int total;
            MPI_Get_count(&Stat,MPI_INT,&total);
            // Recibo la respuesta
            if ((numeroProcesos % 2) && (Stat.MPI_SOURCE == numeroProcesos-1)) {
                tam_a_enviar = tam_a_enviar+TAM;
            }
            MPI_Recv(&(respuesta[i]),
                        tam_a_enviar,
                            MPI_INT,
                                MPI_ANY_SOURCE,
                                    MPI_ANY_TAG,
                                        MPI_COMM_WORLD,
                                            MPI_STATUS_IGNORE);
        }


    } else {
        MPI_Send(&strength,1,MPI_INT,0,0,MPI_COMM_WORLD);
    }*/
    //MPI_Finalize();
    free(blancos);
    free(attacks);
    printf("Resultado:\n");
    printf("T%d -> Strength: %i\n", blancos->id, blancos->strength);
}
