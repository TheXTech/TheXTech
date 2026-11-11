<p align="center">
<a href="https://github.com/Wohlstand/TheXTech/releases"><img src="https://raw.githubusercontent.com/Wohlstand/TheXTech/master/resources/icon/thextech_512.png" alt="TheXTech"></a>
</p>

# TheXTech
Un motor de juegos multiplataforma para juegos de plataformas al estilo de Mario. Es una adaptación a C++ y una continuación directa del [motor SMBX](https://wohlsoft.ru/pgewiki/Super_Mario_Bros._X), desarrollado originalmente por Andrew Spinks en Visual Basic 6 entre 2009 y 2011. El proyecto fue fundado por Vitaliy Novichkov (conocido como «Wohlstand») y se publicó por primera vez el 14 de marzo de 2020, después de que el código fuente del motor original se hiciera público el 2 de febrero de 2020.

En comparación con el motor original de Andrew Spinks, se han añadido numerosas funciones y se han realizado varias mejoras en la física, el rendimiento y la estabilidad. Además, este motor conserva un sólido sistema de compatibilidad (incluida la posibilidad de volver a activar errores antiguos cuando sea necesario para recuperar mecánicas que hayan dejado de funcionar), lo que permite jugar contenido creado para la versión original con las mínimas diferencias posibles.

Es un proyecto hermano de [Moondust Project](https://wohlsoft.ru/projects/Moondust/), que se desarrolla y mantiene en paralelo.

| <!-- -->      | <!-- -->        | <!-- -->        |
|:-------------:|:---------------:|:---------------:|
| [English](README.md) |  [Русский](README.RUS.md)  | Español |

---


## ¿Cómo compilarlo?
Puedes consultar una guía detallada para compilar el proyecto a partir del código fuente en cada plataforma compatible: https://github.com/Wohlstand/TheXTech/wiki/Building-the-game

Para compilarlo, necesitas lo siguiente:
- CMake 3.5 o una versión posterior.
- Un conjunto de compiladores compatibles con C y C++ (GCC, Clang, MinGW-w64 o MSVC 2015, 2017, 2019 y 2022).
- Git (para clonar el repositorio de desarrollo).
- (Opcional) Ninja (para acelerar el proceso de compilación).
- (Opcional) Dependencias instaladas en el sistema: SDL2, libFreeImageLite (una implementación modificada de FreeImage), la biblioteca de sonido MixerX y el conjunto de bibliotecas AudioCodecs. Tenerlas instaladas en el sistema acelera considerablemente la compilación. También es posible compilarlas junto con el proyecto, aunque esto aumenta el tiempo necesario.


## Localización
El propio motor TheXTech (el iniciador de Android, los menús del juego, la interfaz del editor, los mensajes del sistema, etc.) se puede traducir a numerosos idiomas. Puedes ayudar a traducir el proyecto mediante la plataforma Weblate: https://hosted.weblate.org/projects/thextech/

Para traducir contenido, como niveles y episodios, consulta esta guía: https://github.com/TheXTech/TheXTech/wiki/Translate-episodes

## Software utilizado
- GCC, MinGW-w64 y Clang: los principales compiladores utilizados durante el desarrollo del juego. MSVC se utiliza ocasionalmente, sobre todo para compilaciones de Windows destinadas a procesadores ARM64.
- [Qt Creator](https://www.qt.io/product/development-tools): entorno de desarrollo integrado (IDE) del conjunto de herramientas Qt, utilizado principalmente durante el desarrollo.
- [JetBrains CLion](https://www.jetbrains.com/ru-ru/clion/): IDE para C y C++ basado en IntelliJ IDEA, gratuito para el [desarrollo de código abierto](https://www.jetbrains.com/ru-ru/community/opensource/).
- [PVS-Studio](https://pvs-studio.com/es/pvs-studio/?utm_source=website&utm_medium=github&utm_campaign=open_source): analizador estático para código C, C++, C# y Java. Se utiliza periódicamente para detectar posibles errores. Es gratuito para el [desarrollo de código abierto](https://pvs-studio.com/es/order/open-source-license/).

## Firma de código
La firma gratuita de los archivos binarios para Windows es proporcionada por [SignPath.io](https://about.signpath.io/), con un certificado de [SignPath Foundation](https://signpath.org/).


---

# Preguntas frecuentes
En esta sección, yo (Vitaliy Novichkov) respondo algunas de las preguntas más frecuentes sobre el proyecto.

## ¿Qué es esto?
Es una continuación directa del motor SMBX 1.3. Originalmente se escribió en VB6 para Windows y, posteriormente, se adaptó y reescribió en C++ para convertirse en un motor multiplataforma. Reproduce fielmente el antiguo motor SMBX 1.3 (excepto su editor), incluidos muchos de sus errores de lógica. Se corrigieron los errores críticos que provocaban bloqueos o cierres inesperados y también se añadieron numerosas actualizaciones y funciones nuevas.


## ¿Por qué lo creaste?
Tengo varios objetivos al desarrollar este proyecto:
- Contar con un modelo de investigación práctico para utilizarlo durante el desarrollo de Moondust Engine.
- Proporcionar una réplica totalmente compatible del motor antiguo para plataformas modernas, de modo que los niveles y episodios clásicos se puedan jugar con la misma sensación que en el SMBX original basado en VB6.
- Permitir que funcione sin necesidad de Wine en sistemas distintos de Windows y que esté disponible en plataformas que no utilicen arquitecturas x86 o x64.
- Optimizar el consumo de recursos de hardware respecto del juego original basado en VB6.
- Disponer de un motor funcional y listo para utilizarse en proyectos reales, que otras personas y yo podamos emplear para desarrollar contenido mientras preparo la primera versión de Moondust Engine lista para su uso.
- No me gusta la forma en que está construido el proyecto principal SMBX2 (mediante modificaciones del ejecutable x86 de 32 bits del motor SMBX original). Quiero una alternativa independiente, compilada a partir de su código fuente y no basada en modificar un archivo binario propietario. Los motores basados en código fuente ofrecen mucho más control y posibilidades, incluida la compatibilidad multiplataforma nativa.


## Si ya tienes Moondust Engine, ¿por qué dedicaste más de un mes a crear esto?
Lo necesito directamente para desarrollar Moondust Engine: es mucho más sencillo modificar e investigar este motor que trabajar en el antiguo e incómodo entorno de VB6. Además, quiero conservar este motor para mantener la compatibilidad con el contenido antiguo, mientras que Moondust Engine podrá desarrollarse con mayor facilidad sin cargar con todo el legado de Redigit.

También quiero disponer de dos motores diferentes: uno que sea el sucesor directo de SMBX, que conserve su legado y compatibilidad en plataformas modernas, incorporando nuevas funciones y mejoras; y otro motor moderno, creado desde cero, compatible con los formatos de contenido, pero que no herede todos los errores de física del motor original. ~~¡En su lugar, tendrá sus propios errores!~~


## ¿Cuál es el futuro de Moondust Engine ahora que existe TheXTech?
Seguiré desarrollando Moondust Engine porque todavía queda por cumplir el segundo objetivo del proyecto.

Desde su fundación, Moondust Project ha tenido dos objetivos: 1) salvar SMBX y 2) ofrecer un conjunto de herramientas flexible para crear nuevos juegos de plataformas. La publicación del código fuente de SMBX y la aparición de TheXTech han cumplido el primer objetivo: SMBX se ha preservado y ahora es un programa gratuito, de código abierto y multiplataforma.

Moondust Engine se centrará en el segundo objetivo: proporcionar herramientas para crear juegos nuevos. A diferencia de TheXTech, Moondust Engine ofrece un alto grado de flexibilidad para que cualquiera pueda crear algo desde cero sin depender de la estructura de un juego antiguo. No obstante, TheXTech también resulta necesario como modelo de investigación funcional para desarrollar el nuevo motor.

La relación será parecida a la de GZDoom y Chocolate Doom, dos adaptaciones del juego Doom. GZDoom es un motor potente y funcional, ideal para quienes crean modificaciones. Chocolate Doom es una adaptación fiel del juego original a plataformas modernas, cuyo propósito es reproducirlo tal como era, incluidos sus errores. Moondust Engine pretende cumplir un papel similar al de GZDoom, mientras que TheXTech equivale a Chocolate Doom al reproducir el juego original en sistemas modernos.


## ¿Se utilizó alguna inteligencia artificial para desarrollar este proyecto?
No se utilizó ninguna inteligencia artificial (como ChatGPT, Grok, Claude, etc.) para desarrollar este proyecto ni sus dependencias. Además, el uso de inteligencia artificial está prohibido dentro del ámbito de este proyecto y sus dependencias, incluidas las contribuciones. Consulta el archivo [AGENTS.md](AGENTS.md) para obtener más información.


## ¿Cómo se utiliza?
Puedes encontrar todos los detalles en esta guía: https://github.com/TheXTech/TheXTech/wiki/Running-and-playing-the-game


## ¿Cómo crear niveles y episodios propios?
El método oficial recomendado para desarrollar niveles y episodios para TheXTech consiste en utilizar Moondust Devkit junto con el paquete de configuración «TheXTech SDK».

Puedes consultar todos los detalles en esta guía: https://github.com/TheXTech/TheXTech/wiki/Create-levels-and-episodes-for-TheXTech


## ¿En qué se diferencia de la versión original creada en VB6?
La siguiente lista podría estar incompleta, pero incluye las diferencias más importantes:

- Está escrito en C++, mientras que el motor original está escrito en VB6.
- Antes de la versión 1.3.6, no tenía editor. Desde la versión 1.3.6, incluye un editor integrado para dispositivos móviles que funciona a pantalla completa. Se puede controlar mediante teclado y ratón, mando o pantalla táctil. También tiene una integración profunda con Moondust Editor, que permite utilizar el juego con funciones equivalentes a las del editor original. Se conserva la función «mano mágica» para modificar un nivel en tiempo real durante las pruebas.
- Admite completamente UTF-8 en rutas de archivos y textos internos (el juego original solo admitía codificación ANSI de 8 bits).
- Utiliza la biblioteca SDL2 para gráficos y controles, mientras que el juego original recurría a llamadas de WinAPI y a la biblioteca GDI.
- Utiliza la biblioteca PGE-FL, que ofrece mayor compatibilidad con formatos de archivo.
- Admite mapas de mundo WLDX, que permiten incluir una cantidad ilimitada de líneas de créditos y música personalizada sin necesidad de utilizar `music.ini` para sustituir las pistas musicales.
- Ahora funcionan algunas características exclusivas de LVLX: desplazamiento vertical circular entre los extremos de una sección, teletransportes bidireccionales, mensajes personalizados de «estrellas necesarias», eventos al entrar en teletransportes, posibilidad de ocultar las estrellas indicadas en determinadas puertas de episodios HUB y de desactivar las escenas de transición al pasar a otro nivel mediante un teletransporte.
- Incluye compatibilidad integrada con archivos `music.ini` y `sounds.ini` a nivel de episodio o de nivel para reemplazar la música y los efectos de sonido predeterminados.
- Los mapas de mundo pueden utilizar un directorio propio para almacenar recursos específicos, como bloques gráficos, elementos de escenario, caminos y niveles, sin llenar la carpeta principal del episodio con archivos del mapa.
- El formato de configuración predeterminado es INI. El antiguo formato `config.dat` ya no es compatible, principalmente por las diferencias entre los códigos de teclas (`SDL_Scancode` frente a la enumeración `VirtualKeys` de la API de Windows).
- Las partidas guardadas utilizan el formato SAVX en lugar del clásico SAV. Si tienes una partida antigua, puedes seguir jugándola con el motor nuevo. La siguiente vez que guardes, se creará un archivo SAVX y el archivo SAV original permanecerá intacto.
- Admite PNG de forma nativa, tanto para gráficos personalizados como predeterminados. Los GIF con máscaras siguen siendo compatibles por motivos de retrocompatibilidad, sin realizar conversiones automáticas inesperadas como las de SMBX-38A.
- Los puntos de control ahora admiten múltiples ubicaciones. Puedes colocar tantos como quieras en tus niveles.
- Utiliza un algoritmo de descompresión diferida para acelerar la carga del juego y reducir el uso de memoria.
- Se ha rediseñado por completo el algoritmo de búsqueda de objetos, lo que soluciona los problemas de ralentización que aparecían en equipos lentos después de mover capas horizontalmente. Consulta la explicación del [problema DrPepper](https://wohlsoft.ru/pgewiki/DrPepper_Problem).
- Utiliza la biblioteca MixerX para reproducir una gran variedad de formatos de música y efectos de sonido.
- No incluye gráficos integrados en el ejecutable: no hay gráficos fijados directamente en el código; todos se almacenan como recursos externos.
- Se han ampliado algunos límites internos, como la cantidad de secciones y objetos colocados, y se han eliminado las restricciones de tamaño de las texturas para fondos y elementos del mapa de mundo.
- Incluye la función SoundFX, que permite aplicar efectos de reverberación o eco en tiempo real a secciones individuales de los niveles.
- Incorpora una grabadora de GIF que se activa con F11 (F10 en macOS, porque F11 está reservada por la interfaz del sistema para la acción «mostrar escritorio»).
- Se inicia más rápido: la carga es prácticamente instantánea, dependiendo del equipo y del rendimiento de su disco duro o SSD.
- Consume menos memoria RAM: aproximadamente entre 80 y 150 MB, frente a los 600-800 MB habituales del motor original. Además, evita las fugas de memoria provocadas por la interfaz MCI utilizada por SMBX en VB6.
- No sobrecarga la CPU. El problema original se debía a una forma ineficiente de procesar bucles infinitos. También corregí ese problema en la versión VB6, en mi rama `smbx-experiments`.
- Puede funcionar incluso en equipos muy modestos, en los que SMBX basado en VB6 no podría ejecutarse.
- Es completamente multiplataforma: no depende de Windows ni de procesadores x86. También puede funcionar en arquitecturas ARM y MIPS. El SMBX basado en VB6 no puede ejecutarse de forma nativa en ARM y, mediante un emulador x86, puede llegar a funcionar unas 20 veces más lento de lo normal.


## ¿Funcionan los niveles y episodios con LunaDLL Autocode?
Sí. Desde la versión 1.3.6, TheXTech incorpora una implementación del lenguaje LunaDLL Autocode llamada LunaScript. Este sistema permite ejecutar episodios que utilizan LunaDLL en cualquier tipo de hardware, incluidas arquitecturas de procesador distintas de x86.


## ¿Funciona LunaLua?
No. LunaLua no funciona porque este proyecto es incompatible con sus archivos binarios. Esto también significa que el contenido de SMBX2 es incompatible. El sistema de scripts basado en Lua que está previsto no garantiza compatibilidad. Por tanto, si llega a incorporarse la compatibilidad con scripts Lua, será razonable adaptar el contenido existente o crearlo desde cero específicamente para TheXTech.


## ¿Por qué el código de este proyecto es tan malo?
El autor original escribió en VB6 la mayor parte del código que se encuentra en la carpeta `src`. Realicé una conversión completa intentando reproducir su funcionamiento con la mayor fidelidad posible. Por eso, gran parte del código es prácticamente idéntica a la versión original de VB6. Esta plataforma presentaba numerosas dificultades y limitaciones:

- Todas las variables son globales y, de forma predeterminada, se puede acceder a ellas desde cualquier módulo o formulario sin incluir archivos ni importar dependencias. Por eso existe `globals.h`: contiene la lista completa de variables disponibles globalmente.
- La compatibilidad con clases era limitada y poco práctica, lo que favoreció el uso excesivo de variables y matrices globales. La falta de experiencia inicial del autor original también contribuyó a esta situación.
- Todas las funciones de los módulos son globales y pueden llamarse directamente desde otros módulos, salvo las declaradas como privadas. Por eso tuve que añadir inclusiones en los archivos donde se necesitaban esas llamadas.
- ¿Por qué hay tantas cadenas de `if-elseif-elseif-elseif-elseif-...`? Probablemente sería más apropiado utilizar `switch()` (equivalente a `Select Case` en VB6). Es otro ejemplo de la poca experiencia del autor original cuando programó este proyecto.
- ¿Por qué hay tantas estructuras anidadas del tipo `if() { if {} if { ... } }`? Hay dos motivos: 1) la inexperiencia del autor original y 2) la necesidad de evitar evaluar condiciones que podrían provocar un fallo. En C++, cuando varias condiciones se unen mediante `&&`, las siguientes no se evalúan si una anterior resulta falsa. En VB6, en cambio, se evalúan todas las condiciones de una expresión. Por ejemplo, en VB6, `if A < 5 And Array(A) = 1 Then` podría provocar un fallo si A supera los límites permitidos del arreglo. En C++, la expresión `if(A < 5 && Array[A] == 1)` no evalúa la segunda condición si la primera es falsa, por lo que evita ese problema.
- ¿Por qué existen expresiones tan largas como `if(id == 1 || id == 3 || id == 4 || ... id == N)`? En lugar de acumular tantas condiciones, sería mejor usar clases y polimorfismo para separar la lógica de los distintos objetos. También podrían utilizarse punteros a funciones, que son posibles en C++, pero no están disponibles directamente en VB6 sin soluciones alternativas. Una vez más, estas construcciones son consecuencia de la combinación entre la inexperiencia del autor original y las limitaciones de VB6.

Sin embargo, desde 2020 se han ido reorganizando y optimizando muchas de estas estructuras, por lo que este código heredado está desapareciendo gradualmente.
