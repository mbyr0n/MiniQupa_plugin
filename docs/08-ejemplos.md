# Ejemplos ejecutables y verificación

[Índice](README.md)

Todos los comandos se ejecutan desde la raíz del repositorio. Primero instala el plugin y exporta las rutas como explica [primer controlador](01-primer-controlador.md). Añade `-z` para ejecutar sin ventana. Los escenarios duran cinco segundos simulados a 20 Hz; el tiempo real puede ser menor cuando no hay visualización.

## 1. Movimiento básico

```bash
argos3 -c examples/demo.argos -z
```

[Lua](../examples/demo.lua) · [XML](../examples/demo.argos)

Avanza a 2 cm/s durante dos segundos y se detiene. El difusor queda azul. Envía radio/RAB sin receptores; termina con `MINIQUPA_DEMO_DONE`. Sirve como punto de partida, no verifica recepción entre vecinos.

## 2. Respuesta a un obstáculo

```bash
argos3 -c examples/avoidance.argos -z
```

[Lua](../examples/avoidance.lua) · [XML](../examples/avoidance.argos)

Una caja está delante de IR2/IR3. Si la proximidad frontal supera 0,15, ordena giro a la izquierda y difusor rojo; en otro caso avanza y usa verde. Al final comprueba que hubo respuesta de giro y al menos 1 cm de recorrido estimado por encoder. Termina con `MINIQUPA_AVOIDANCE_OK`.

Es un controlador reactivo didáctico. No garantiza navegación a un destino ni ausencia de colisiones con cualquier geometría. Sus umbrales se eligieron para esta escena. Para experimentar, cambia el umbral o la caja y observa las lecturas.

## 3. Dos robots: luz, radio y RAB

```bash
argos3 -c examples/neighbors.argos -z
```

[Lua](../examples/neighbors.lua) · [XML](../examples/neighbors.argos)

Coloca dos robots quietos a 30 cm entre sus referencias. `mq0` emite rojo y usa ID lógico 1; `mq1` emite azul y usa ID lógico 2. Llevan tags 7 y 8, independientes de esos IDs lógicos.

Cada robot verifica:

- Recepción de la tabla radio del otro robot con contenido conocido.
- Recepción RAB de diez bytes, incluidos el byte cambiado por índice y un valor 255.
- Distancia RAB de 30 **cm** y presencia de ángulos numéricos.
- Observación de blobs del color del vecino, no del propio difusor.
- Ancho angular/muestras positivos y suelo blanco cuando la lectura es válida.

Deben aparecer **dos** mensajes: `MINIQUPA_NEIGHBORS_OK mq0` y `MINIQUPA_NEIGHBORS_OK mq1`. No es suficiente que solo uno termine. El escenario no valida detección aérea del tag, radio física ni fidelidad de ESP-NOW.

## 4. Controlador C++

Compila y exporta su biblioteca como indica [C++](07-cpp.md), después:

```bash
argos3 -c examples/cpp.argos -z
```

[Fuente](../examples/cpp/controller.cpp) · [CMake](../examples/cpp/CMakeLists.txt) · [XML](../examples/cpp.argos)

Consulta las interfaces propias y genéricas, envía un paquete binario por simple radios, configura RAB y avanza/parada. Comprueba ocho IR, suelo blanco válido, ausencia de vecinos, exclusión de su propio difusor y un desplazamiento de encoder entre 3 y 5 cm. Termina con `MINIQUPA_CPP_OK`.

Este XML carga `libminiqupa_manual_controller`, no el controlador Lua. Su biblioteca es un ejemplo de aplicación separado del plugin del robot; compilar el robot no compila automáticamente este controlador.

## Errores y reinicio

Los ejemplos nuevos contienen comprobaciones `assert` en Lua o excepciones C++. Si una falla, revisa el mensaje y el marcador final, además del código de salida de ARGoS. No elimines la comprobación para ocultar una configuración incorrecta. Si cambias el escenario deliberadamente, revisa también sus expectativas.

`reset()` reinicia el estado interno de los ejemplos; ARGoS reinicia sus dispositivos. Si cambias duración o frecuencia, adapta los contadores y el paso final 100; de lo contrario puede no aparecer el marcador aun si el programa es válido.

## Alcance de la revisión de este manual

Los ejemplos se compilaron/ejecutaron con ARGoS 3.0.0-beta59, instalación local del plugin y sin visualización. También se comprobaron los enlaces locales de Markdown y la ausencia de rutas de usuario. Las APIs se contrastaron con los fuentes de las interfaces propias y genéricas instaladas.

Esto verifica los ejemplos y sus condiciones declaradas; no es una nueva calibración física. Las herramientas instaladas Crapper, Dryer y Mutator no admiten C++ ni Lua: se ejecutaron para comprobar su alcance y no se presenta una puntuación CRAP o cobertura de mutación de estos ejemplos. Las comprobaciones de ejecución y dos pruebas negativas de fallo respaldan su comportamiento.
