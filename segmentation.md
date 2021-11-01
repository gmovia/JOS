# Simulando Segmentación

En este ejercicio haremos uso del script de simulación *´segmentation.py´*, provisto por los autores del OSTEP. 

## Simulación de traducciones

**Corridas del script para cada padrón:**

**Padrón 102696**
    
    damian@damian-lenovo:~/UBA/SisOp$ ./segmentation.py  -a 64 -p 256 -s 102696 -n 2
    ARG seed 102696
    ARG address space size 64
    ARG phys mem size 256

    Segment register information:

    Segment 0 base  (grows positive) : 0x00000050 (decimal 80)
    Segment 0 limit                  : 17

    Segment 1 base  (grows negative) : 0x000000b7 (decimal 183)
    Segment 1 limit                  : 17

    Virtual Address Trace
    VA  0: 0x00000030 (decimal:   48) --> PA or segmentation violation?
    VA  1: 0x00000024 (decimal:   36) --> PA or segmentation violation?
    

**Padrón 102896**

    gmovia27@gmovia27-System-Product-Name:~/Escritorio/SegmentacionTP1$ ./segmentation.py  -a 64 -p 256 -s 102896 -n 2
    ARG seed 102896
    ARG address space size 64
    ARG phys mem size 256

    Segment register information:

    Segment 0 base  (grows positive) : 0x000000de (decimal 222)
    Segment 0 limit                  : 28

    Segment 1 base  (grows negative) : 0x000000c7 (decimal 199)
    Segment 1 limit                  : 19

    Virtual Address Trace
    VA  0: 0x00000033 (decimal:   51) --> PA or segmentation violation?
    VA  1: 0x00000011 (decimal:   17) --> PA or segmentation violation?


**1)** Tomar los dos accesos a memoria y decidir si producen una dirección física o bien generan un segmentation fault

**Padrón 102696**

Memoria Virtual:
- Segmento 0 (0 a 16)
- Segmento 1 (47 a 63)

Mapeos a Memoria Física:
- Segmento 0 -> (80 a 96)
- Segmento 1 -> (166 a 182)

Traducciones:
- VA 0: 0x030 (decimal 48) -> Segmento 1 (dentro, pos 1) -> PA: 0x0a7 (decimal 167)
- VA 1: 0x024 (decimal 36) -> Segmento 1 (fuera) -> Segmentation fault

**Padrón 102896**

Memoria Virtual:
- Segmento 0 (0 a 27)
- Segmento 1 (45 a 63)

Mapeos a Memoria Física:
- Segmento 0 -> (222 a 249)
- Segmento 1 -> (180 a 198)

Traducciones:
- VA 0: 0x033 (decimal 51) -> Segmento 1 (dentro, pos 6) -> PA: 0x0ba (decimal 186)
- VA 1: 0x011 (decimal 17) -> Segmento 0 (dentro, pos 17) -> PA: 0x0ef (decimal 239)


**2)** Explicar el razonamiento del punto anterior, indicando las cuentas realizadas

Primero planteamos el mapa de memoria, por el enunciado del ejercicio sabemos que en MV, el segmento 0 empezará siempre en la dirección 0, y el segmento 1 terminará siempre en la última dirección. Con esto en mente determinamos los segmentos según su tamaño (el segmento 1 crece invertido); luego planteamos el mapeo de cada segmento a memoria física según su base, con el mismo criterio de crecimiento para cada uno (la base del segmento-1 no está incluída en el mismo).

Luego cada dirección virtual la podemos traducir fácilmente, primero nos fijamos en el segmento correspondiente (bit más significativo) y si cae dentro (dir_inicio_segmento <= dir_virtual <= dir_fin_segmento), entonces tendrá una traducción física asociada que se determinará por la cuenta: $pa = base \pm pos$, donde *pos* estará dada por (dir_virtual - dir_inicio_segmento). Si la dirección virtual cae fuera entonces lanzará *segmentation fault*.

**3)** Validar los resultados volviendo a correr con el flag -c

**Padrón 102696**

    damian@damian-lenovo:~/UBA/SisOp$ ./segmentation.py  -a 64 -p 256 -s 102696 -n 2 -c
    ARG seed 102696
    ARG address space size 64
    ARG phys mem size 256

    Segment register information:

    Segment 0 base  (grows positive) : 0x00000050 (decimal 80)
    Segment 0 limit                  : 17

    Segment 1 base  (grows negative) : 0x000000b7 (decimal 183)
    Segment 1 limit                  : 17

    Virtual Address Trace
    VA  0: 0x00000030 (decimal:   48) --> VALID in SEG1: 0x000000a7 (decimal:  167)
    VA  1: 0x00000024 (decimal:   36) --> SEGMENTATION VIOLATION (SEG1)

**Padrón 102896**

    gmovia27@gmovia27-System-Product-Name:~/Escritorio/SegmentacionTP1$ ./segmentation.py  -a 64 -p 256 -s 102896 -n 2 -c
    ARG seed 102896
    ARG address space size 64
    ARG phys mem size 256

    Segment register information:

    Segment 0 base  (grows positive) : 0x000000de (decimal 222)
    Segment 0 limit                  : 28

    Segment 1 base  (grows negative) : 0x000000c7 (decimal 199)
    Segment 1 limit                  : 19

    Virtual Address Trace
    VA  0: 0x00000033 (decimal:   51) --> VALID in SEG1: 0x000000ba (decimal:  186)
    VA  1: 0x00000011 (decimal:   17) --> VALID in SEG0: 0x000000ef (decimal:  239)

---
## Traducciones inversas

Se busca lograr que las direcciones virtuales de la primera corrida se traduzcan a las direcciones físicas de la segunda corrida (o bien, pasen a dar segmentation fault); y viceversa. Para ésto, será necesario encontrar parámetros nuevos para segmento-0 y segmento-1.

**1)** Para cada corrida modificada, determinar los valores de cada segmento, y especificarlos mediante los cuatro flags: -b, -l, -B y -L. Existe la posibilidad de que no se necesiten todos los valores para lograr el objetivo, o bien que el objetivo no sea posible. Si hay más de una solución; elegir la que tenga los límites de segmentos más pequeños. En caso de que no sea posible; explicar por qué.

**Corridas modificadas:** 

**Padrón 102696**
- VA 0: 0x030 (decimal 48) -> SEG1 -> PA: 0x0ba (decimal 186)
- VA 1: 0x024 (decimal 36) -> SEG1 -> PA: 0x0ef (decimal 239)

En este caso **no será posible el objetivo**, ya que las direcciones virtuales pertenecen al segmento-1 (esto está dado por el bit más significativo), y las direcciones físicas tienen una distancia mayor a nuestro espacio de direcciones virtuales, por lo que un segmento no podría nunca abarcar tal rango.

**Padrón 102896**
- VA 0: 0x033 (decimal 51) -> SEG1 -> PA: 0x0a7 (decimal 167)
- VA 1: 0x011 (decimal 17) -> SEG0 -> SEG FAULT

Dado el espacio y mapeo generado por el padrón (*seed*):
- SEG0: VM(0 a 28) --> PM(222 a 250) - size 28
- SEG1: VM(45 a 64) --> PM(180 a 199) - size 19

Los cambios necesarios para obtener la corrida modificada serán:
- Modificar la base del segmento-1 de forma que la primera posición mapee a la dirección física 167. Para ésto debemos configurar el segmento de tamaño mínimo (i.e., límite segmento-1 = 64 - 51 = 13, base segmento-1 = 167 + 13 = 180)
- Modificar el límite del segmento-0 de forma que la dirección virtual 17 caiga fuera del segmento, lo que se puede conseguir con un segmento mínimo de 1 entrada (i.e., límite segmento-0 = 1)

Estas modificaciones se logran con los siguientes flags: -B 180 -L 13 -l 1


**2)** En caso de que haya solución, mostrar además la corrida de la simulación que cumple con los requisitos.

**Padrón 102896**

    gmovia27@gmovia27-System-Product-Name:~/Escritorio/SegmentacionTP1$ ./segmentation.py  -a 64 -p 256 -s 102896 -A 51,17 -c -B 180 -L 13 -l 1
    ARG seed 102896
    ARG address space size 64
    ARG phys mem size 256

    Segment register information:

    Segment 0 base  (grows positive) : 0x000000c2 (decimal 194)
    Segment 0 limit                  : 1

    Segment 1 base  (grows negative) : 0x000000b4 (decimal 180)
    Segment 1 limit                  : 13

    Virtual Address Trace
    VA  0: 0x00000033 (decimal:   51) --> VALID in SEG1: 0x000000a7 (decimal:  167)
    VA  1: 0x00000011 (decimal:   17) --> SEGMENTATION VIOLATION (SEG0)



## Límites de segmentación

Utilizando un espacio de direcciones virtuales de 5-bits; y un espacio de direcciones físicas de 7-bits. Responder:

**1)** ¿Cuál es el tamaño (en número de direcciones) de cada espacio (físico y virtual)?

- Memoria Virtual: 32 direcciones
- Memoria Física: 128 direcciones

**2)** ¿Es posible configurar la simulación de segmentación para que dos direcciones virtuales se traduzcan en la misma dirección física? Explicar, y de ser posible brindar un ejemplo de corrida de la simulación.

Si bien el script hace un chequeo de solapamiento entre los mapeos de los segmentos, aún así es posible configurarlo de la siguiente manera:
- SEG0: VM(0 a 7) -> PM(24 a 31)
- SEG1: VM(24 a 31) -> PM(17 a 24)
De forma que las direcciones virtuales 0 y 31, ambas se traduzcan a la dirección física 24.

Corrida en la simulación:

    $ ./segmentation.py  -a 32 -p 128 -l 8 -L 8 -B 25 -b 24 -A 0,31 -c
    ARG seed 0
    ARG address space size 32
    ARG phys mem size 128

    Segment register information:

    Segment 0 base  (grows positive) : 0x00000018 (decimal 24)
    Segment 0 limit                  : 8

    Segment 1 base  (grows negative) : 0x00000019 (decimal 25)
    Segment 1 limit                  : 8

    Virtual Address Trace
    VA  0: 0x00000000 (decimal:    0) --> VALID in SEG0: 0x00000018 (decimal:   24)
    VA  1: 0x0000001f (decimal:   31) --> VALID in SEG1: 0x00000018 (decimal:   24)


**3)** ¿Es posible que (aproximadamente) el 90% del espacio de direcciones virtuales esté mapeado de forma válida? Explicar, y de ser posible, dar una configuración de segmentos que de tal resultado.

Teniendo en cuenta sólamente la simulación, es posible mapear *todas* las direcciones virtuales, asignando un límite de 16 para cada segmento. El problema es que en la práctica será necesario mapear ciertas regiones a otros componentes (dispositivos E/S, ROM, etc); a continuación dejamos como ejemplo un mapeo completo de la MV.

    $ ./segmentation.py  -a 32 -p 128 -l 16 -L 16 -A 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31 -c
    ARG seed 0
    ARG address space size 32
    ARG phys mem size 128

    Segment register information:

    Segment 0 base  (grows positive) : 0x0000006c (decimal 108)
    Segment 0 limit                  : 16

    Segment 1 base  (grows negative) : 0x00000045 (decimal 69)
    Segment 1 limit                  : 16

    Virtual Address Trace
    VA  0: 0x00000000 (decimal:    0) --> VALID in SEG0: 0x0000006c (decimal:  108)
    VA  1: 0x00000001 (decimal:    1) --> VALID in SEG0: 0x0000006d (decimal:  109)
    VA  2: 0x00000002 (decimal:    2) --> VALID in SEG0: 0x0000006e (decimal:  110)
    VA  3: 0x00000003 (decimal:    3) --> VALID in SEG0: 0x0000006f (decimal:  111)
    VA  4: 0x00000004 (decimal:    4) --> VALID in SEG0: 0x00000070 (decimal:  112)
    VA  5: 0x00000005 (decimal:    5) --> VALID in SEG0: 0x00000071 (decimal:  113)
    VA  6: 0x00000006 (decimal:    6) --> VALID in SEG0: 0x00000072 (decimal:  114)
    VA  7: 0x00000007 (decimal:    7) --> VALID in SEG0: 0x00000073 (decimal:  115)
    VA  8: 0x00000008 (decimal:    8) --> VALID in SEG0: 0x00000074 (decimal:  116)
    VA  9: 0x00000009 (decimal:    9) --> VALID in SEG0: 0x00000075 (decimal:  117)
    VA 10: 0x0000000a (decimal:   10) --> VALID in SEG0: 0x00000076 (decimal:  118)
    VA 11: 0x0000000b (decimal:   11) --> VALID in SEG0: 0x00000077 (decimal:  119)
    VA 12: 0x0000000c (decimal:   12) --> VALID in SEG0: 0x00000078 (decimal:  120)
    VA 13: 0x0000000d (decimal:   13) --> VALID in SEG0: 0x00000079 (decimal:  121)
    VA 14: 0x0000000e (decimal:   14) --> VALID in SEG0: 0x0000007a (decimal:  122)
    VA 15: 0x0000000f (decimal:   15) --> VALID in SEG0: 0x0000007b (decimal:  123)
    VA 16: 0x00000010 (decimal:   16) --> VALID in SEG1: 0x00000035 (decimal:   53)
    VA 17: 0x00000011 (decimal:   17) --> VALID in SEG1: 0x00000036 (decimal:   54)
    VA 18: 0x00000012 (decimal:   18) --> VALID in SEG1: 0x00000037 (decimal:   55)
    VA 19: 0x00000013 (decimal:   19) --> VALID in SEG1: 0x00000038 (decimal:   56)
    VA 20: 0x00000014 (decimal:   20) --> VALID in SEG1: 0x00000039 (decimal:   57)
    VA 21: 0x00000015 (decimal:   21) --> VALID in SEG1: 0x0000003a (decimal:   58)
    VA 22: 0x00000016 (decimal:   22) --> VALID in SEG1: 0x0000003b (decimal:   59)
    VA 23: 0x00000017 (decimal:   23) --> VALID in SEG1: 0x0000003c (decimal:   60)
    VA 24: 0x00000018 (decimal:   24) --> VALID in SEG1: 0x0000003d (decimal:   61)
    VA 25: 0x00000019 (decimal:   25) --> VALID in SEG1: 0x0000003e (decimal:   62)
    VA 26: 0x0000001a (decimal:   26) --> VALID in SEG1: 0x0000003f (decimal:   63)
    VA 27: 0x0000001b (decimal:   27) --> VALID in SEG1: 0x00000040 (decimal:   64)
    VA 28: 0x0000001c (decimal:   28) --> VALID in SEG1: 0x00000041 (decimal:   65)
    VA 29: 0x0000001d (decimal:   29) --> VALID in SEG1: 0x00000042 (decimal:   66)
    VA 30: 0x0000001e (decimal:   30) --> VALID in SEG1: 0x00000043 (decimal:   67)
    VA 31: 0x0000001f (decimal:   31) --> VALID in SEG1: 0x00000044 (decimal:   68)



**4)** ¿Es posible que (aproximadamente) el 90% del espacio de direcciones físicas esté mapeado de forma válida? Explicar, y de ser posible, dar una configuración de segmentos que de tal resultado.

No, ya que tenemos un máximo de 32 direcciones virtuales; lo máximo que podremos mapear serán 32 direcciones físicas cuando no exista ningún solapamiento. Por eso, contando con 128 direcciones físicas, sería imposible mapear el 90% de ellas.