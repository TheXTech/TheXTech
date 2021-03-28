# TheXTech 

Motor SMBX, reescrito en C ++ a partir de VisualBasic 6. 

# Preguntas Frecuentes

## ¿Qué es esto? Es un puerto de un antiguo motor VB6, escrito puramente en C ++. Reproduce enteramente el antiguo motor SMBX 1.3 (aparte de su Editor), incluye varios de sus errores lógicos (errores críticos que hacen que el juego se bloquee o se congele). 

## ¿Por qué lo hiciste? Tengo diversos fines para realizarlo: 
- Es un modelo de investigación bastante conveniente que deseo usar en el desarrollo del motor PGE. - Proveer una copia plenamente compatible del motor antiguo para plataformas modernas, lo cual posibilita jugar niveles y episodios viejos con la misma sensación que si se hubieran jugado en el juego SMBX original con base en VB6. 
- Para que funcione sin la necesidad de usar Wine en plataformas que no sean Windows y que se encuentre disponible en plataformas que no sean x86 / x64. - Optimizándolo para utilizar menos recursos de hardware que el juego original con base en VB6. 

## Tienes PGE Engine, ¿por qué has pasado bastante más de un mes para producir esta cosa? 
Lo necesito para el desarrollo de PGE Engine de manera directa, es muchísimo más simple de piratear y examinar que un ámbito VB6 antiguo e inconveniente. 

## ¿Cuál es el futuro de PGE Engine ahora que excite TheXTech? 
Continuaré desarrollando el motor PGE debido a que aún tengo que conseguir el segundo objetivo del proyecto. 
A partir de su fundación, el Proyecto PGE poseía 2 fines: 1) rescatar SMBX; 2) dar un grupo de herramientas flexible para nuevos juegos de plataformas. La aperturadel código fuente de SMBX y la introducción de TheXTech ha resuelto el primer objetivo: SMBX se ha guardado y ahora es un software multiplataforma gratuito / de código abierto. PGE Engine se usará para el segundo objetivo: dar un grupo de herramientas para nuevos juegos. A diferencia de TheXTech, PGE Engine da un elevado nivel de flexibilidad que posibilita a cualquier persona edificar algo nuevo a partir de cero sin heredar una base de juego vieja. No obstante, TheXTech se necesita para PGE Engine como modelo de investigacion funcional para desarrollar el nuevo motor. Va a ser parecido a los puertos GZDoom y Chocolate Doom del juego Doom: GZDoom es un motor potente y servible, la mejor alternativa para los modders; Chocolate Doom es una adaptacion precisadel juego original a una plataforma modernadestinados a representar el juego original, integrados inclusive los errores. El motor PGE pretende ser como GZDoom, en lo que TheXTech es un semejante de Chocolate Doom para representar un juego original en plataformas modernas.

## ¿Puede LunaLua trabajar en esto?
No, LunaLua no funcionará: este proyecto es binario-incompatible con LunaLua. Esto también significa que el contenido SMBX2 es incompatible.


## ¿Por qué el código aquí es tan malo?
El autor original escribió la mayor parte del código en la carpeta "* src *" en VB6. Hice una conversión completa del código con un esfuerzo por lograr una reproducción precisa. Por lo tanto, gran parte del código es idéntico a lo que se escribió originalmente en VB6. La plataforma VB6 tenía muchos desafíos y limitaciones como:
- Todas las variables son globales y accesibles desde todos los módulos y se forman de forma predeterminada sin ninguna inclusión o importación. La razón por la que existe "globals.h": tiene una lista completa de variables disponibles globalmente.
- Soporte limitado e inconveniente para las clases, por lo tanto, el código tiende a abusar de una tonelada de variables y matrices globales (también una falta inicial de experiencia del autor original fue otro factor que llevó a este lío).
- Todas las funciones de todos los módulos son globales y se pueden llamar directamente desde cada módulo. Excepto las llamadas marcadas como "privadas". Por lo tanto, tuve un trabajo adicional para proporcionar inclusiones en archivos donde se solicitan estas llamadas.
- ¿Por qué tanto `if-elseif-elseif-elseif-elseif -....?` Sí, aquí probablemente sea correcto usar `switch ()` (en VB6 el operador `Select Case` analógico). Otro factor que muestra que el autor original tenía poca experiencia cuando codificó este proyecto.
- ¿Por qué la lasaña `if () {if {} if {....}}`? Dos razones: 1) inexperiencia del autor original, 2) solución para no verificar todas las condiciones de expresión que pueden causar un bloqueo. En C ++ con múltiples condiciones divididas por el operador `&&`, nunca se ejecuta cuando una de ellas obtiene un resultado falso. En VB6, TODAS las condiciones en la obtención de expresiones siempre se ejecutarán. Esta diferencia provocó la siguiente situación: en VB6, una expresión `if A <5 And Array (A) = 1 Then` provocará un bloqueo cuando A sea más de 5. En C ++ el mismo` if (A <5 && Array [ Una expresión] == 1) `nunca fallará porque una segunda verificación nunca se ejecutará si una primera verificación dio un resultado falso.
- ¿Por qué expresiones tan largas como `if (id == 1 || id == 3 || id == 4 || ... id == N)`? En lugar de hacer un montón de condiciones como esta, sería mejor usar clases con un polimorfismo y separar la lógica de cada objeto entre diferentes clases. También debería poder resolverse teniendo que usar punteros de función (que no son posibles en VB6 sin workaronds, pero son posibles en C ++). Pero, nuevamente, la inexperiencia del autor original combinada con un montón de límites de VB6 causaron estas construcciones.


## ¿Como usar esto?
Aquí hay muchas formas de jugar con él:
- hay algunos paquetes listos para usar, simplemente tómelos y juegue como lo hizo con SMBX.
- [usuarios de macOS, omita esto]: use de la misma manera que un juego original: coloque un archivo ejecutable en la carpeta raíz del juego con un "thextech.ini" que contenga el siguiente texto:
```
[Main]
force-portable = true
```
, music.ini, sounds.ini y una carpeta adicional "graphics/ui". Una nota importante: todos los gráficos predeterminados deben convertirse a PNG, use la herramienta GIFs2PNG de PGE Project en su carpeta "gráficos" con un interruptor "-d". No use el interruptor "-r" para mantener los GIF originales junto con los PNG recién creados si planea continuar usando SMBX original escrito en VB6.
- utilícelo para el modo de depuración: en su directorio de inicio, cree la carpeta ".PGE_Project/thextech" (en macOS, la "`~/Library/Application Support/PGE Project/thextech`") donde debe colocar un juego completo recursos y cosas de mundos, esta carpeta funcionará como la raíz de un juego en el juego original. Este modo le permite ejecutar un archivo ejecutable desde cualquier ubicación de carpeta de su computadora y usar la misma ubicación de recursos para todas las compilaciones (excepto que estos están marcados como portátiles por un archivo INI).


## ¿Cómo agregar episodios personalizados para la versión de macOS?
Si tiene una compilación integrada de TheXTech, todos los recursos predeterminados están dentro de su .app: "Content/Resources/assets/". Puedes modificar el contenido, ¡pero no es recomendable! En cambio, después de la primera ejecución de un juego, en su directorio de inicio aparecerá el siguiente directorio:
''
   ~/TheXTech Episodes
''
En este directorio, encontrará una carpeta vacía de "batalla" y "mundos" para poner sus cosas personalizadas. En los registros de ruta "`~/Library/Application Support/PGE Project/thextech` ", se almacenarán las configuraciones y las partidas guardadas.
Si desea reemplazar los activos predeterminados por los suyos, puede modificar el contenido del paquete de aplicaciones o compilar una nueva compilación con los argumentos de CMake necesarios que necesitaban para empaquetar la raíz y el icono de sus activos personalizados en el nuevo paquete o crear los activos. -sin construir (si no da argumentos, resultará la compilación sin activos). Por lo tanto, debes poner el contenido completo de la raíz del juego en la carpeta "`~/Library/Application Support/PGE Project/thextech` ", incluir recursos predeterminados (gráficos, música, sonidos, niveles de introducción y salida, batalla predeterminada y carpetas de mundos).


## ¿Qué es diferente con esto en comparación con la versión VB6 original?
- En primer lugar, está escrito en C ++, mientras que el original (como ya sabemos) está escrito en VB6.
- No tiene editor. En cambio, tiene una integración profunda con PGE Editor que permite usarlo con la misma funcionalidad que en el editor original (la funcionalidad de "mano mágica" se mantuvo para permitir la edición en tiempo real del nivel durante la prueba, es necesario usar la comunicación IPC con PGE Editor para tener la capacidad de usarlo mejor).
- Soporte completo de UTF-8 en rutas de nombre de archivo y datos de texto internos (el juego original tenía el único soporte ANSI de 8 bits).
- Para gráficos y control, usa una biblioteca SDL2, mientras que el juego original ha usado llamadas WinAPI y biblioteca GDI.
- Utiliza PGE-FL que tiene un mejor soporte de formatos de archivo.
- Un soporte para mapas del mundo WLDX permite líneas de crédito ilimitadas y música personalizada sin que sea necesario usar un music.ini para reemplazos de música.
- Algunas características exclusivas de LVLX ahora funcionan: envoltura de sección vertical, deformaciones bidireccionales, mensaje personalizado de "estrella necesaria", evento de entrada de deformación, capacidad para deshabilitar la impresión de estrellas en episodios HUB para puertas específicas, capacidad para deshabilitar la visualización entre escenas al pasar a otro nivel a través de una deformación.
- Soporte integrado para episodios y niveles de music.ini y sounds.ini para anular los activos de música y sonidos predeterminados.
- Los mapas del mundo ahora admiten un directorio personalizado para almacenar recursos específicos como mosaicos / escenas / rutas / niveles personalizados y ya no enviar spam a la carpeta raíz del episodio con recursos del mapa del mundo.
- El formato de configuración predeterminado es INI, el antiguo formato config.dat ya no es compatible, principalmente debido a valores de código clave incompatibles (SDL_Scancode versus enumeración VirtualKeys de la API de Windows).
- El juego se guarda ahora usando el formato SAVX en lugar de un SAV clásico. Sin embargo, si ya tiene un guardado de juegos antiguo, aún puede reanudar su juego usando un nuevo motor ahora (el próximo intento de guardado de juegos dará como resultado un archivo SAVX, el guardado de juegos antiguo en formato SAV se mantendrá intacto).
- Soporte PNG incorporado para gráficos personalizados y predeterminados. Sin embargo, los GIF enmascarados todavía son compatibles por compatibilidad con versiones anteriores, sin realizar una conversión automática inesperada como lo hace SMBX-38A.
- ¡Los puntos de control ahora tienen multipuntos! ¡Puedes usarlos en tus niveles varias veces sin límites!
- Utiliza un algoritmo de descompresión diferida para acelerar la carga de un juego y reducir el uso de memoria.
- ¡Para música y SFX, la biblioteca MixerX se usa para brindar soporte para una amplia cantidad de formatos de sonido y música!
- No incorpora ningún gráfico: NO hay gráficos realmente codificados, ¡ahora todo está representado por gráficos externos!
- Se han ampliado algunos límites internos.
- Grabador GIF integrado con la tecla F11 (F10 en macOS, F11 está reservado por la interfaz de usuario del sistema para una acción de "mostrar escritorio")
- Comienza más rápido: la carga del juego es casi instantánea (depende de una computadora y del rendimiento de su HDD / SSD).
- Utiliza menos RAM (80 ... 150 MB en lugar de 600 ... 800 MB como normalmente), y está libre de pérdidas de memoria dadas por la interfaz MCI utilizada originalmente por VB6 SMBX.
- no sobrecarga la CPU (la razón fue una mala manera de procesar bucles infinitos, también hice la corrección de la compilación de VB6 en mi rama "smbx-experiment")
- Puede funcionar en "tostadora" (una computadora débil) mientras que VB6-SMBX no funcionará.
- es completamente multiplataforma y no depende de Windows, y ya no depende del procesador x86: también puede funcionar en procesadores ARM y MIPS (VB6-SMBX no funcionará en ARM en absoluto, con el emulador x86 sí 20 veces más lento de lo habitual).


## ¿Cómo construirlo?
Puede leer una guía sobre cómo construir este proyecto desde la fuente aquí: https://github.com/Wohlstand/TheXTech/wiki/Building-the-game

Para construirlo, necesita tener lo siguiente:
- CMake
- Ninja opcionalmente (para acelerar el proceso de construcción)
- Compilador C / C ++ compatible (GCC, Clang, MinGW, MSVC 2017 y 2019, posiblemente también se construirá en 2015, pero no se probó)
- Git (necesario para extraer submódulos y clonar la fuente de las bibliotecas dependientes para construirlos en su lugar)
- Mercurial (necesario para clonar un repositorio oficial de SDL2 para construirlo en su lugar aquí)
- Opcionalmente: dependencias instaladas en todo el sistema: SDL2, libFreeImageLite (una implementación modificada de FreeImage), biblioteca de sonido MixerX, colección de bibliotecas AudioCodecs. Tenerlos instalados en un sistema aumenta la velocidad de construcción. Sin embargo, es posible construir todas estas dependencias en su lugar aquí con un costo adicional de tiempo de construcción agregado.


