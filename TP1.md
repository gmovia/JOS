TP1: Memoria virtual en JOS
===========================

boot_alloc_pos
--------------

a. Un cálculo manual de la primera dirección de memoria que devolverá boot_alloc() tras el arranque. Se puede calcular a partir del binario compilado (obj/kern/kernel), usando los comandos readelf y/o nm y operaciones matemáticas.

boot_alloc es una función que debe devolver la dirección de memoria virtual de la próxima página libre al kernel. El kernel se encuentra almacenado en la dirección de memoria virtual 0xF0100000, la cual representa la suma entre el KERNBASE y 1MB. Utilizando el comando nm sobre el binario podemos obtener la dirección del símbolo *end*, el cual apunta al final de la imagen del kernel cargada en memoria. 

    f0118320 b cons
    f0118528 b crt_pos
    f011852c b crt_buf
    f0118530 b addr_6845
    f0118534 b serial_exists
    f0118538 b nextfree.1419
    f011853c b page_free_list
    f0118540 b buf
    f0118940 B panicstr
    f0118944 B npages
    f0118948 B kern_pgdir
    f011894c B pages
    f0118950 B end


Con esta dirección, y como las páginas poseen un tamaño de 4KB, entonces podemos concluir que la direccion de memoria virtual de la próxima página libre será 0xF0119000.



b.Una sesión de GDB en la que, poniendo un breakpoint en la función boot_alloc(), se muestre el valor devuelto en esa primera llamada, usando el comando GDB finish.

    $ make gdb
    gdb -q -s obj/kern/kernel -ex 'target remote 127.0.0.1:26000' -n -x .gdbinit
    Reading symbols from obj/kern/kernel...
    Remote debugging using 127.0.0.1:26000
    warning: No executable has been specified and target does not support
    determining executable automatically.  Try using the "file" command.
    0x0000fff0 in ?? ()
    (gdb) b boot_alloc
    Breakpoint 1 at 0xf0100a55: file kern/pmap.c, line 98.
    (gdb) c
    Continuing.
    The target architecture is assumed to be i386
    => 0xf0100a55 <boot_alloc>:	cmpl   $0x0,0xf0118538

    Breakpoint 1, boot_alloc (n=4096) at kern/pmap.c:98
    98		if (!nextfree) {
    (gdb) finish
    Run till exit from #0  boot_alloc (n=4096) at kern/pmap.c:98
    => 0xf0102671 <mem_init+26>:	mov    %eax,0xf0118948
    mem_init () at kern/pmap.c:142
    142		kern_pgdir = (pde_t *) boot_alloc(PGSIZE);
    Value returned is $1 = (void *) 0xf0119000
    (gdb) 


page_alloc
----------

Responder: ¿en qué se diferencia page2pa() de page2kva()?

- page2pa : La función page2pa recibe por parámetro un puntero a un elemento PageInfo que representa una página física. Teniendo un puntero global al inicio del arreglo, se puede hallar el índice del elemento mediante la resta de las direcciones; el índice resultante estará asociado a una dirección física, dada por la cuenta ($pa = índice * PGSIZE$). Con ésto, la función devuelve la dirección física asociada a un elemento PageInfo.

- page2kva : La función page2kva realiza el mismo procedimiento que la función page2pa con la salvedad de que retorna la dirección de memoria virtual asociada a la página física representada por la estructura. Para ello aprovecha la organización del espacio virtual de *JOS*, el cual mediante una simple suma (pa + KERNBASE) realiza esta traducción usando el macro KADDR().

map_region_large
----------------

¿cuánta memoria se ahorró de este modo? (en KiB)

Cuando queremos utilizar páginas físicas de 4KB, entonces necesitaremos un page directory de 1024 entradas, cada una con una posible page table de 1024 entradas (4KB). Esto significa que cada page table ocupará una página física, dando un total de 4MB con todas las entradas del directorio ocupadas.

Usando páginas grandes, podríamos entonces ahorrarnos estos 4MB ya que no usaremos page tables (4KB por cada page table que no usemos), aunque dependerá dado que nuestra implementación usa páginas grandes oportunísiticamente.


¿es una cantidad fija, o depende de la memoria física de la computadora?

Como mencionamos, estaremos ahorrando 4KB por cada página grande que usemos, con un máximo de 4MB. Esto es así debido a la implementación de usar un sólo page directory, por lo que no podremos ahorrar más que 4MB.