Responder:

* al terminar un proceso su función umain() ¿dónde retoma la ejecución el kernel? Describir la secuencia de llamadas desde que termina umain() hasta que el kernel dispone del proceso.

Tras finalizar la función umain, se retorna a libmain, la cual llamará posteriormente a la función exit() que utiliza la syscall "sys_env_destroy" con un valor de envid igual a 0. 
Utilizando el patrón dispatch, la función llama a syscall pasando como parámetros los valores "SYS_env_destroy, 1, 0, 0, 0, 0, 0" (observar que el valor de envid es nulo). A partir de allí, la función utiliza un switcheo del codigo, y como se recibio el parametro "SYS_env_destroy" entonces se llama a la syscall sys_env_destroy con el envid en 0. Como el envid es nulo entonces se debe destruir el proceso actual, y por lo tanto se llama a env_destroy, que a su vez llama a env_free y sched_yield, el cual llamará a sched_halt.
 
* ¿en qué cambia la función env_destroy() en este TP, respecto al TP anterior?
En el trabajo práctico anterior, la función env_destroy solamente destruía al único environment que existía, finalizando con la invocación al monitor del kernel. En el trabajo práctico actual, la función contempla el caso en el que el environment posea un estado "ENV_RUNNING", y analiza si el environment no es el curenv, modificando el estado a “ENV_DYING”. Si el environment es el curenv entonces se lo setea a NULL y luego se invoca a la función sched_yield.
 
Responder:
* ¿Qué ocurre en JOS si un proceso llama a sys_env_destroy(0)?.
Si un proceso llama a sys_env_destroy(0), entonces se destruirá al environment actual.
 
--


