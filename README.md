# parallel-battle-field
Proyecto 3 para Sistemas de Operación 3 - Sept-Dic 2015

### Para compilar:

        mpicc battle-field.c -o battle

### Para correr las pruebas:

1. Prueba 1:
        mpirun --disable-hostname-propagation -n 4 -machinefile maquinas.txt ./battle ./tests/test
2. Prueba 2:
        mpirun --disable-hostname-propagation -n 9 -machinefile maquinas.txt ./battle ./tests/test2
3. Prueba 3:
        mpirun --disable-hostname-propagation -n 22 -machinefile maquinas.txt ./battle ./tests/test3


### Resultados:

##### Prueba 1

| Tipo de Blanco                       | Resultado |
| ------------------------------------ | :-------: |
| Military Targets totally destroyed   |     2     |
| Military Targets partially destroyed |     0     |
| Military Targets not affected        |     0     |
| Civilian Targets totally destroyed   |     0     |
| Civilian Targets partially destroyed |     2     |
| Civilian Targets not affected        |     0     |


##### Prueba 2

| Tipo de Blanco                       | Resultado |
| ------------------------------------ | :-------: |
| Military Targets totally destroyed   |     1     |
| Military Targets partially destroyed |     4     |
| Military Targets not affected        |     0     |
| Civilian Targets totally destroyed   |     1     |
| Civilian Targets partially destroyed |     2     |
| Civilian Targets not affected        |     1     |


##### Prueba 3

| Tipo de Blanco                       | Resultado |
| ------------------------------------ | :-------: |
| Military Targets totally destroyed   |     4     |
| Military Targets partially destroyed |     8     |
| Military Targets not affected        |     1     |
| Civilian Targets totally destroyed   |     4     |
| Civilian Targets partially destroyed |     3     |
| Civilian Targets not affected        |     2     |
