# TP4 : Sistema de archivos y interprete de comandos

1. ¿Qué es super->s_nblocks?

super->s_nblocks representa la cantidad de bloques totales que hay en el sistema de archivos.

2. ¿Dónde y cómo se configura este bloque especial?

En el archivo fsformat.c se le da formato al sistema de archivos, con la funcion opendisk() se configura la estructura del bloque super. Luego en fs_init() del archivo fs.c se apunta a dicha estructura.