# TP3: Multitarea con desalojo


## env_return

**Al terminar un proceso su función umain() ¿dónde retoma la ejecución el kernel? Describir la secuencia de llamadas desde que termina umain() hasta que el kernel dispone del proceso.**

Tras finalizar la función umain, se retorna a libmain, la cual llamará posteriormente a la función exit() que utiliza la syscall "sys_env_destroy" con un valor de envid igual a 0. 
Utilizando el patrón dispatch, la función llama a syscall pasando como parámetros los valores "SYS_env_destroy, 1, 0, 0, 0, 0, 0" (observar que el valor de envid es nulo). A partir de allí, la función utiliza un switcheo del codigo, y como se recibio el parametro "SYS_env_destroy" entonces se llama a la syscall sys_env_destroy con el envid en 0. Como el envid es nulo entonces se debe destruir el proceso actual, y por lo tanto se llama a env_destroy, que a su vez llama a env_free y sched_yield, el cual llamará a sched_halt.

**¿En qué cambia la función env_destroy() en este TP, respecto al TP anterior?**

En el trabajo práctico anterior, la función env_destroy solamente destruía al único environment que existía, finalizando con la invocación al monitor del kernel. En el trabajo práctico actual, la función contempla el caso en el que el environment posea un estado "ENV_RUNNING", y analiza si el environment no es el curenv, modificando el estado a “ENV_DYING”. Si el environment es el curenv entonces se lo setea a NULL y luego se invoca a la función sched_yield.

## sys_yield

**Leer y estudiar el código del programa `user/yield.c`. Cambiar la función `i386_init()` para lanzar tres instancias de dicho programa, y mostrar y explicar la salida de `make qemu-nox`**


La función de `yield.c` simplemente realiza un ciclo for en el que se desaloja y luego de retomar el control del CPU, imprime por salida estándar un mensaje para indicar que retomó la ejecución, su `PID` y el número de iteración en la que se encuentra. Al correr el comando indicado se obtuvo la siguiente salida:

    SMP: CPU 0 found 1 CPU(s)
    enabled interrupts: 1 2
    [00000000] new env 00001000
    [00000000] new env 00001001
    [00000000] new env 00001002
    Hello, I am environment 00001000.
    Hello, I am environment 00001001.
    Hello, I am environment 00001002.
    Back in environment 00001000, iteration 0.
    Back in environment 00001001, iteration 0.
    Back in environment 00001002, iteration 0.
    Back in environment 00001000, iteration 1.
    Back in environment 00001001, iteration 1.
    Back in environment 00001002, iteration 1.
    Back in environment 00001000, iteration 2.
    Back in environment 00001001, iteration 2.
    Back in environment 00001002, iteration 2.
    Back in environment 00001000, iteration 3.
    Back in environment 00001001, iteration 3.
    Back in environment 00001002, iteration 3.
    Back in environment 00001000, iteration 4.
    All done in environment 00001000.
    [00001000] exiting gracefully
    [00001000] free env 00001000
    Back in environment 00001001, iteration 4.
    All done in environment 00001001.
    [00001001] exiting gracefully
    [00001001] free env 00001001
    Back in environment 00001002, iteration 4.
    All done in environment 00001002.
    [00001002] exiting gracefully
    [00001002] free env 00001002
    No runnable environments in the system!

Se puede observar como se crean los tres procesos y en cada iteración cada proceso se desaloja intencionalmente. Primero arranca `00001000`, se desaloja, luego por `round-robin` irá el proceso siguiente `00001001`, entra al ciclo y se desaloja, por último empieza el ciclo el proceso `00001002` que se desaloja y por la política de `round-robin` ahora le tocará al proceso `00001000` nuevamente e imprime el primer mensaje por pantalla `"Back in enviroment..."`. Así se repiten todos los ciclos hasta que el primer proceso llega al último y podemos observar como el kernel cede el CPU a otros procesos recién cuando el proceso actual muere (ya que no hay llamadas explícitas a `schied_yield()` y no están habilitadas las interrupciones del timer).

---

## envid2env

**Responder qué ocurre en `JOS` si un proceso llama a `sys_env_destroy(0)`**

Cuando se hace el llamado `sys_env_destroy(0)`, lo primero que hace la syscall es pasar de `envid` a `struct Env *`, con lo que se hace llamado a `envid2env(0)`. Dicha función si se invoca con 0 devuelve el proceso actual, es decir `curenv`. Luego se llama a `env_destroy()`, con lo que está destruyendo el proceso actual (en dicha función se agrega la comprobación de: si se está destruyendo al proceso actual se hace llamado a `schied_yield` para correr otro programa `RUNNABLE`).

---

## dumbfork

Tras leer con atención el código, se pide responder las siguientes preguntas:

**Si una página no es modificable en el padre ¿lo es en el hijo? En otras palabras: ¿se preserva, en el hijo, el flag de solo-lectura en las páginas copiadas?**

**Mostrar, con código en espacio de usuario, cómo podría dumbfork() verificar si una dirección en el padre es de solo lectura, de tal manera que pudiera pasar como tercer parámetro a duppage() un booleano llamado readonly que indicase si la página es modificable o no:**

    envid_t dumbfork(void) {
        // ...
        for (addr = UTEXT; addr < end; addr += PGSIZE) {
            bool readonly;
            //
            // TAREA: dar valor a la variable readonly
            //
            duppage(envid, addr, readonly);
        }
        // ...

**Supongamos que se desea actualizar el código de duppage() para tener en cuenta el argumento readonly: si este es verdadero, la página copiada no debe ser modificable en el hijo. Es fácil hacerlo realizando una última llamada a sys_page_map() para eliminar el flag PTE_W en el hijo, cuando corresponda:**

    void duppage(envid_t dstenv, void *addr, bool readonly) {
        // Código original (simplificado): tres llamadas al sistema.
        sys_page_alloc(dstenv, addr, PTE_P | PTE_U | PTE_W);
        sys_page_map(dstenv, addr, 0, UTEMP, PTE_P | PTE_U | PTE_W);

        memmove(UTEMP, addr, PGSIZE);
        sys_page_unmap(0, UTEMP);

        // Código nuevo: una llamada al sistema adicional para solo-lectura.
        if (readonly) {
            sys_page_map(dstenv, addr, dstenv, addr, PTE_P | PTE_U);
        }
    }

**Esta versión del código, no obstante, incrementa las llamadas al sistema que realiza duppage() de tres, a cuatro. Se pide mostrar una versión en el que se implemente la misma funcionalidad readonly, pero sin usar en ningún caso más de tres llamadas al sistema.**

---

## multicore_init

**1. ¿Qué código copia, y a dónde, la siguiente línea de la función boot_aps()?**


    memmove(code, mpentry_start, mpentry_end - mpentry_start);

La línea en cuestión, es ejecutada por BSP, y lo que hace es copiar código que servirá de entry-point para los APs. Dicho código, ubicado en `mpentry.S`, presenta los tags `mpentry_start` y `mpentry_end`, que sirve para ubicarlo y determinar su tamaño. El mismo es copiado en la dirección física `MPENTRY_PADDR`, que no estará previamente en uso.


**2. ¿Para qué se usa la variable global mpentry_kstack? ¿Qué ocurriría si el espacio para este stack se reservara en el archivo kern/mpentry.S, de manera similar a bootstack en el archivo kern/entry.S?**

Previo a que un AP se inicialice con la función `lapic_startap()`, el BSP setea una variable global que es un puntero al kernel stack del cpu próximo a inicializar.

El espacio para ese stack no puede reservarse en el archivo `mpentry.S`, ya que como arranca en modo real, no tiene ninguna referencia del page directory ya creado del kernel.


**3. En el archivo kern/mpentry.S se puede leer:**

    # We cannot use kern_pgdir yet because we are still
    # running at a low EIP.
    movl $(RELOC(entry_pgdir)), %eax


**a) ¿Qué valor tendrá el registro %eip cuando se ejecute esa línea? Responder con redondeo a 12 bits, justificando desde qué región de memoria se está ejecutando este código.**

Esa línea pertenece al código entry point de un AP, dicho código fué mapeado a la dirección `MPENTRY_PADDR` con `memmove()` en `boot_aps()`. Esa dirección es `0x7000` (es una dirección física). Por lo tanto, el registro `%eip` cuando pasa por esa instrucción, redondeada a 12 bits, es `0x7000`.

---

## ipc_recv


Una vez implementada la función, resolver este ejercicio:

Un proceso podría intentar enviar el valor númerico -E_INVAL vía ipc_send(). ¿Cómo es posible distinguir si es un error, o no?

    envid_t src = -1;
    int r = ipc_recv(&src, 0, NULL);

    if (r < 0)
        if (/* ??? */)
            puts("Hubo error.");
        else
            puts("Valor negativo correcto.");


La función `ipc_recv` fue llamada con un valor de `from_env_store` distinto de NULL, por lo que de fallar la syscall dicho valor será puesto a cero. Entonces el código para diferenciar un error de un valor negativo enviado podría ser:


    envid_t src = -1;
    int r = ipc_recv(&src, 0, NULL);

    if (r < 0) 
        if (!src)
            puts("Hubo error.");
        else
            puts("Valor negativo correcto.");


---

## sys_ipc_try_send

**Se pide ahora explicar cómo se podría implementar una función sys_ipc_send() (con los mismos parámetros que sys_ipc_try_send()) que sea bloqueante, es decir, que si un proceso A la usa para enviar un mensaje a B, pero B no está esperando un mensaje, el proceso A sea puesto en estado ENV_NOT_RUNNABLE, y despertado una vez B llame a ipc_recv() (cuya firma no debe ser cambiada).**


Es posible que surjan varias alternativas de implementación; para cada una, indicar:

qué cambios se necesitan en struct Env para la implementación (campos nuevos, y su tipo; campos cambiados, o eliminados, si los hay)
qué asignaciones de campos se harían en sys_ipc_send()
qué código se añadiría en sys_ipc_recv()
Responder, para cada diseño propuesto:

¿existe posibilidad de deadlock?
¿funciona que varios procesos (A₁, A₂, …) puedan enviar a B, y quedar cada uno bloqueado mientras B no consuma su mensaje? ¿en qué orden despertarían?


Se podria usar un mecanismo similar al que utiliza `sys_ipc_recv` agregando un flag del tipo `bool env_ipc_sending;` en el `struct Env`. De esta manera ambas syscalls primero validarán errores y luego en caso del send si el proceso pasado por parámetro no está dormido en receiving, el sender se va a dormir. Así el proceso que recibe (ahora la syscall recibirá el id del proceso que espera recibir), comprobará si este está intentando enviar datos con el flag propuesto. Como este es el caso, mapeará y tomará el dato necesario y despertará al sender y retornará. El caso análogo en el que el receive llega primero y este se pone en NOT RUNNEABLE ya lo conocemos y es el implementado hasta ahora.