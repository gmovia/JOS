# TP2: Procesos de usuario

## env_alloc

**¿Qué identificadores se asignan a los primeros 5 procesos creados? (Usar base hexadecimal)**

El identificador asignado resultará de la suma del número 0x01000 con la id actual del env (al ser la primera vez, las id serán 0), a la que luego se hace una operación OR (i.e., setea los últimos 3 nibbles) con el índice de cada env.

- 1°: 0x01000
- 2°: 0x01001
- 3°: 0x01002
- 4°: 0x01003
- 5°: 0x01004


**Supongamos que al arrancar el kernel se lanzan NENV procesos a ejecución. A continuación se destruye el proceso asociado a envs[630] y se lanza un proceso que cada segundo muere y se vuelve a lanzar (se destruye, y se vuelve a crear). ¿Qué identificadores tendrán esos procesos en las primeras cinco ejecuciones?**

Dado que no hay otros envs libres, el proceso estará asociado siempre al mismo env (índice 630), por lo que los últimos 3 nibbles serán 276 (630 en hexa) y serán los mismos para todas las ejecuciones. Luego el número 0x01000 se sumará en cada ejecución (ya que al liberar un env **no se resetea la id**):

- 631°: 0x01276 
- +0x01000: 0x02276
- +0x01000: 0x03276
- +0x01000: 0x04276
- +0x01000: 0x05276

---

## env_init_percpu

**¿Cuántos bytes escribe la función lgdt, y dónde?**

La funcion lgdt escribe 6 bytes en el Global Descriptor Table Register (GDTR)


**¿Qué representan esos bytes?**

Los 6 bytes que se escriben en la GDT se descomponen en dos campos, los primeros 2 bytes representan el tamaño de la GDT y los restantes 4 bytes representan la dirección base de la tabla.

---

## env_pop_tf

**Dada la secuencia de instrucciones assembly en la función, describir qué contiene durante su ejecución:**
- **el tope de la pila justo antes popal**
- **el tope de la pila justo antes iret**
- **el tercer elemento de la pila justo antes de iret**

Justo antes de popal (movl %0,%%esp). En el registro %esp (stack pointer) se asigna el contenido de %0 (tf), que contiene la dirección de memoria en la cual se encuentra tf_regs. Es decir en el tope de la pila se encontrará el primer registro EDI.

Justo antes de iret, en el registro %esp se encuentra la dirección de tf->tf_eip. Es decir en el tope de la pila se encontrará el contenido de EIP.

Justo antes de iret, como en el registro %esp se encuentra la dirección de tf->tf_eip, entonces el tercer elemento de la pila será el contenido de EFLAGS.

**¿Cómo determina la CPU (en x86) si hay un cambio de ring (nivel de privilegio)? Ayuda: Responder antes en qué lugar exacto guarda x86 el nivel de privilegio actual. ¿Cuántos bits almacenan ese privilegio?**

Para determinar si hay un cambio de ring, la CPU analiza los 2 bits menos significativos de tf_cs y los compara con los ultimos 2 bits del contenido actual del registro %cs. A partir de ello, si ambos contenidos no son iguales, entonces podemos determinar que hubo un cambio de ring.


## gdb_hello

**Se pide mostrar una sesión de GDB con los siguientes pasos:**

**1) Poner un breakpoint en env_pop_tf() y continuar la ejecución hasta allí.**

    damian@damian-lenovo:~/UBA/SisOp/tp1$ make gdb
    gdb -q -s obj/kern/kernel -ex 'target remote 127.0.0.1:26000' -n -x .gdbinit
    Reading symbols from obj/kern/kernel...
    Remote debugging using 127.0.0.1:26000
    warning: No executable has been specified and target does not support
    determining executable automatically.  Try using the "file" command.
    0x0000fff0 in ?? ()
    (gdb) b env_pop_tf
    Breakpoint 1 at 0xf0102ffd: file kern/env.c, line 457.
    (gdb) c
    Continuing.
    The target architecture is assumed to be i386
    => 0xf0102ffd <env_pop_tf>:	endbr32 

    Breakpoint 1, env_pop_tf (tf=0xf01d1000) at kern/env.c:457
    457	{
    (gdb) 

**2) En QEMU, entrar en modo monitor (Ctrl-a c), y mostrar las cinco primeras líneas del comando**

    (qemu) info registers
    EAX=003bc000 EBX=00010094 ECX=f03bc000 EDX=000001ef
    ESI=00010094 EDI=00000000 EBP=f0118fd8 ESP=f0118fbc
    EIP=f0102f36 EFL=00000092 [--S-A--] CPL=0 II=0 A20=1 SMM=0 HLT=0
    ES =0010 00000000 ffffffff 00cf9300 DPL=0 DS   [-WA]
    CS =0008 00000000 ffffffff 00cf9a00 DPL=0 CS32 [-R-]

**3) De vuelta a GDB, imprimir el valor del argumento tf:**

    (gdb) p tf
    $1 = (struct Trapframe *) 0xf01c7000

**4) Imprimir, con x/Nx tf tantos enteros como haya en el struct Trapframe donde N = sizeof(Trapframe) / sizeof(int).**

    (gdb) print sizeof(struct Trapframe)/sizeof(int)
    $2 = 17
    (gdb) x/17x tf
    0xf01c7000:	0x00000000	0x00000000	0x00000000	0x00000000
    0xf01c7010:	0x00000000	0x00000000	0x00000000	0x00000000
    0xf01c7020:	0x00000023	0x00000023	0x00000000	0x00000000
    0xf01c7030:	0x00800020	0x0000001b	0x00000000	0xeebfe000
    0xf01c7040:	0x00000023

**5) Avanzar hasta justo después del movl ...,%esp, usando si M para ejecutar tantas instrucciones como sea necesario en un solo paso:**

    (gdb) disas
    Dump of assembler code for function env_pop_tf:
    => 0xf0102ffd <+0>:	endbr32 
    0xf0103001 <+4>:	push   %ebp
    0xf0103002 <+5>:	mov    %esp,%ebp
    0xf0103004 <+7>:	sub    $0xc,%esp
    0xf0103007 <+10>:	mov    0x8(%ebp),%esp
    0xf010300a <+13>:	popa   
    0xf010300b <+14>:	pop    %es
    0xf010300c <+15>:	pop    %ds
    0xf010300d <+16>:	add    $0x8,%esp
    0xf0103010 <+19>:	iret   
    0xf0103011 <+20>:	push   $0xf0105a15
    0xf0103016 <+25>:	push   $0x1d3
    0xf010301b <+30>:	push   $0xf01059ce
    0xf0103020 <+35>:	call   0xf01000ad <_panic>
    End of assembler dump.
    (gdb) si 5
    => 0xf010300a <env_pop_tf+13>:	popa   
    0xf010300a in env_pop_tf (tf=0x0) at kern/env.c:458
    458		asm volatile("\tmovl %0,%%esp\n"

**6) Comprobar, con x/Nx $sp que los contenidos son los mismos que tf (donde N es el tamaño de tf).**

    (gdb) x/17x $sp
    0xf01c7000:	0x00000000	0x00000000	0x00000000	0x00000000
    0xf01c7010:	0x00000000	0x00000000	0x00000000	0x00000000
    0xf01c7020:	0x00000023	0x00000023	0x00000000	0x00000000
    0xf01c7030:	0x00800020	0x0000001b	0x00000000	0xeebfe000
    0xf01c7040:	0x00000023

**7) Describir cada uno de los valores. Para los valores no nulos, se debe indicar dónde se configuró inicialmente el valor, y qué representa.**

Los 17 valores se corresponden con la estructura de Trapframe:

    estructura Trapframe:
        tf_regs; // 8 registros de prop. gral.
        tf_es;
        tf_ds;
        tf_trapno;
        tf_err;
        tf_eip;
        tf_cs;
        tf_eflags;
        tf_esp;
        tf_ss;

Los primeros 8 son los valores de los registros de propósito general (EAX, EBX, ECX, EDX, ESI, EDI, EBP, ESP) los cuales se inicializan en 0. 

Los siguientes 2 valores corresponden al ES y al DS respectivamente, los cuales se configuran en **env_alloc()** con el valor (GD_UD | 3) -> (23 en hexa).

Los valores de tf_trapno y tf_err son nulos ya que en este punto no se ha lanzado ninguna interrupción.

El siguiente valor corresponde a tf_eip el cual representa la dirección de entry point del env y se configura en **load_icode()**.

Luego el valor de CS, configurado también en **env_alloc()** con el valor (GD_UT | 3) -> (1b en hexa).

Luego los EFLAGS (flags del environment) que serán nulos.

Y por último los valores de ESP y SS, que también son configurados en **env_alloc()** con los valores USTACKTOP y (GD_UD | 3) respectivamente.



**8) Continuar hasta la instrucción iret, sin llegar a ejecutarla. Mostrar en este punto, de nuevo, las cinco primeras líneas de info registers en el monitor de QEMU. Explicar los cambios producidos.**

    (qemu) info registers
    EAX=00000000 EBX=00000000 ECX=00000000 EDX=00000000
    ESI=00000000 EDI=00000000 EBP=00000000 ESP=f01c7030
    EIP=f0102f49 EFL=00000096 [--S-AP-] CPL=0 II=0 A20=1 SMM=0 HLT=0
    ES =0023 00000000 ffffffff 00cff300 DPL=3 DS   [-WA]
    CS =0008 00000000 ffffffff 00cf9a00 DPL=0 CS32 [-R-]


Con la instrucción **popal** se actualizan los registros de propósito general (con valor 0).

Luego se actualizan los registros ES y DS respectivamente con las intrucciones **popl**.

Y finalmente se actualiza el stack pointer (%esp + 8) para saltear los valores de tf_trapno y tf_err dado que al momento no se utilizan.


**9) Ejecutar la instrucción iret. En ese momento se ha realizado el cambio de contexto y los símbolos del kernel ya no son válidos.**

- **imprimir el valor del contador de programa con p $pc o p $eip**

        (gdb) p $eip
        $4 = (void (*)()) 0x800020


- **cargar los símbolos de hello con el comando add-symbol-file**

        (gdb) add-symbol-file obj/user/hello 0x800020
        add symbol table from file "obj/user/hello" at
            .text_addr = 0x800020
        (y or n) y
        Reading symbols from obj/user/hello...

- **volver a imprimir el valor del contador de programa**


        (gdb) p $pc
        $5 = (void (*)()) 0x800020 <_start>

**Mostrar una última vez la salida de info registers en QEMU, y explicar los cambios producidos.**

    (qemu) info registers
    EAX=00000000 EBX=00000000 ECX=00000000 EDX=00000000
    ESI=00000000 EDI=00000000 EBP=00000000 ESP=eebfe000
    EIP=00800020 EFL=00000002 [-------] CPL=3 II=0 A20=1 SMM=0 HLT=0
    ES =0023 00000000 ffffffff 00cff300 DPL=3 DS   [-WA]
    CS =001b 00000000 ffffffff 00cffa00 DPL=3 CS32 [-R-]

Al ejecutar la instrucción iret se completan los registros con los valores dados por el Trapframe del environment. También como se puede ver se cambia de modo kernel a modo usuario.

**10) Poner un breakpoint temporal (tbreak, se aplica una sola vez) en la función syscall() y explicar qué ocurre justo tras ejecutar la instrucción int $0x30. Usar, de ser necesario, el monitor de QEMU.**

    (gdb) tbreak syscall
    Temporary breakpoint 2 at 0x800a3b: syscall. (2 locations)
    (gdb) c
    Continuing.
    => 0x800a3b <syscall+17>:	mov    0x8(%ebp),%ecx

    Temporary breakpoint 2, syscall (num=0, check=-289415544, a1=4005551752, 
        a2=13, a3=0, a4=0, a5=0) at lib/syscall.c:23
    23		asm volatile("int %1\n"
    (gdb) si 4
    => 0x800a47 <syscall+29>:	int    $0x30
    0x00800a47	23		asm volatile("int %1\n"
    (gdb) si
    => 0xf010392c <trap_48+2>:	push   $0x30
    0xf010392c in trap_48 () at kern/trapentry.S:67
    67	TRAPHANDLER_NOEC(trap_48, T_SYSCALL)

La instrucción int $0x30 genera una interrupción de tipo 48 -> T_SYSCALL, la cual se usa para llamadas a funciones de sistema y cambia de modo usuario a modo kernel, lo cual se puede ver en los registros a continuación.

Registros en el QEMU monitor:

    (qemu) info registers
    EAX=00000000 EBX=00000000 ECX=0000000d EDX=eebfde88
    ESI=00000000 EDI=00000000 EBP=eebfde40 ESP=efffffe8
    EIP=f010392c EFL=00000096 [--S-AP-] CPL=0 II=0 A20=1 SMM=0 HLT=0
    ES =0023 00000000 ffffffff 00cff300 DPL=3 DS   [-WA]
    CS =0008 00000000 ffffffff 00cf9a00 DPL=0 CS32 [-R-]



---------

## kern_idt

**Cómo decidir si usar TRAPHANDLER o TRAPHANDLER_NOEC? ¿Qué pasaría si se usara solamente la primera?**

Debemos analizar, para cada vector, si el CPU realiza un push del código de error o no. En el caso de que no lo haga, se debe utilizar la macro TRAPHANDLER_NOEC, la cual pusheará un 0 en lugar del EC. De no hacer esto, en los casos donde el CPU no haga push del código de error, el struct basado en el trapframe quedaría con un formato inválido, desencadenando en errores.

**¿Qué cambia, en la invocación de handlers, el segundo parámetro (istrap) de la macro SETGATE? ¿Por qué se elegiría un comportamiento u otro durante un syscall?**

En el caso de utilizar el valor 0 para el parámetro istrap, la CPU deshabilitara las interrupciones cuando estemos en el modo kernel, mientras que en el caso de utilizar el valor 1 para el parametro istrap, la CPU no desactivara las interrupciones al estar en modo kernel.

**Leer user/softint.c y ejecutarlo con make run-softint-nox. ¿Qué interrupción trata de generar? ¿Qué interrupción se genera? Si son diferentes a la que invoca el programa… ¿cuál es el mecanismo por el que ocurrió esto, y por qué motivos? ¿Qué modificarían en JOS para cambiar este comportamiento?**

El programa intenta generar un Page Fault pero JOS lanza una interrupción General Protection, esto se debe a que la interrupción fue configurada en **trap_init()** con modo kernel (sólamente el kernel podrá generarla). Para obtener este comportamiento entonces se deberá configurar la interrupción con modo usuario (i.e., último argumento de SETGATE en 3).

---------

## user_evilhello

**¿En qué se diferencia el código de la versión en evilhello.c mostrada arriba?**

El programa evilhello.c intenta imprimir el contenido de la dirección 0xf010000c (no válida para user mode), usando la syscall **sys_cputs()**. La versión alternativa intenta imprimir el primer caracter del contenido, pero usando una dirección válida para el llamado de la syscall.

**¿En qué cambia el comportamiento durante la ejecución? ¿Por qué? ¿Cuál es el mecanismo?**

En la versión alternativa se intenta acceder al contenido de la dirección "prohibida" sin usar la syscall, lo que genera una interrupción en modo usuario. En la versión evilhello.c se usa una syscall la cual se ejecuta en modo kernel y no chequea (por ahora) los permisos de memoria, por lo que puede salirse con la suya.

**Listar las direcciones de memoria que se acceden en ambos casos, y en qué ring se realizan. ¿Es esto un problema? ¿Por qué?**

En ambas versiones se intenta acceder a la dirección 0xf010000c, pero en la versión alternativa se intenta acceder desde modo usuario, mientras que en evilhello.c se llama a la syscall la cual se ejecuta en modo kernel. Esto es un problema ya que esa dirección debería estar protegida, por lo que es necesario implementar un chequeo (user_mem_assert).