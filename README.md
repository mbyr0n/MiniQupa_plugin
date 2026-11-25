# MiniQupa para ARGoS3

Plugin de simulación de un robot diferencial MiniQupa, con geometría visual derivada de su CAD e interfaces C++ y Lua. Incluye ocho sensores IR, cámara panorámica de blobs, sensor RGB inferior, difusor RGB superior, encoder ideal, simple radios, range-and-bearing y tag superior opcional.

El repositorio contiene el plugin y un ejemplo mínimo. No necesitas el workspace utilizado durante su desarrollo ni los archivos CAD originales para compilarlo o ejecutarlo.

## Contenido

- [Requisitos](#requisitos)
- [Compilar e instalar](#compilar-e-instalar)
- [Ejecutar el ejemplo](#ejecutar-el-ejemplo)
- [Dispositivos e interfaces](#dispositivos-e-interfaces)
- [Configurar el robot](#configurar-el-robot)
- [Usarlo desde C++](#usarlo-desde-c)
- [Integración dentro del árbol ARGoS](#integración-dentro-del-árbol-argos)
- [Estructura](#estructura)
- [Límites del modelo](#límites-del-modelo)
- [Solución de problemas](#solución-de-problemas)
- [Atribución y licencia](#atribución-y-licencia)

## Requisitos

- Linux y compilador C++17. La búsqueda de recursos utiliza `dladdr` de Linux/Unix; Windows no está validado.
- CMake 3.16 o superior y Python 3 para generar la geometría durante la compilación.
- ARGoS3 instalado con headers, bibliotecas y configuración CMake. Verificado con **3.0.0-beta59**.
- Componentes ARGoS: `entities`, `genericrobot`, `dynamics2d` y `media`.
- Para el ejemplo Lua: ARGoS compilado con soporte Lua y su controlador `lua_controller`.
- Para visualizar: ARGoS con Qt/OpenGL y las dependencias de desarrollo correspondientes. Puede omitirse la visualización al compilar.

## Compilar e instalar

Clona este repositorio y abre una terminal en su carpeta raíz:

```bash
git clone https://github.com/mbyr0n/MiniQupa_plugin.git
cd MiniQupa_plugin
```

También puedes descargarlo como ZIP y abrir su carpeta raíz. Los comandos siguientes funcionan desde esa carpeta, independientemente del nombre del usuario o del repositorio.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j4
cmake --install build --prefix "$PWD/install"
```

No requiere instalar el plugin globalmente. CMake detecta ARGoS mediante su configuración. Si ARGoS está en un prefijo personalizado, añádelo al configurar:

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH="/ruta/al/prefijo-de-argos"
```

Sustituye ese marcador por tu prefijo real. Para omitir la visualización, configura con `-DMINIQUPA_WITH_QTOPENGL=OFF` y ejecuta ARGoS con `-z`.

La instalación genera:

```text
install/
├── lib/argos3/libargos3plugin_simulator_miniqupa.so
├── include/argos3/plugins/robots/miniqupa/
│   ├── control_interface/
│   └── simulator/
└── share/argos3/robots/miniqupa/models/
```

Puedes mover la carpeta `install` completa. La biblioteca busca los modelos respecto a su ubicación instalada. Si cambias el código o la geometría, vuelve a compilar e instalar.

## Ejecutar el ejemplo

Desde la raíz del repositorio, en la misma terminal:

```bash
export MINIQUPA_PREFIX="$PWD/install"
export ARGOS_PLUGIN_PATH="$MINIQUPA_PREFIX/lib/argos3${ARGOS_PLUGIN_PATH:+:$ARGOS_PLUGIN_PATH}"
export LD_LIBRARY_PATH="$MINIQUPA_PREFIX/lib/argos3${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
argos3 -c examples/demo.argos
```

Para ejecutar sin ventana:

```bash
argos3 -c examples/demo.argos -z
```

El robot avanza dos segundos a 2 cm/s, se detiene y mantiene el difusor azul. También emite datos por simple radios y RAB; al haber un solo robot, no hay receptores en este ejemplo. A los cinco segundos aparece `MINIQUPA_DEMO_DONE`. El XML activa todos los dispositivos e incluye sus medios. La ruta del Lua es relativa al directorio de trabajo; ejecuta estos comandos desde la raíz del repositorio.

Para usar un prefijo trasladado, actualiza `MINIQUPA_PREFIX` y vuelve a exportar las dos rutas. Las bibliotecas de ARGoS deben ser accesibles en el entorno habitual de esa instalación.

## Dispositivos e interfaces

Los dispositivos solo se activan cuando el controlador los declara en su XML. `examples/demo.argos` muestra los nombres e implementaciones exactos.

| Función | Dispositivo XML | Interfaz Lua | Unidades o comportamiento |
|---|---|---|---|
| Ruedas | `differential_steering` | `robot.wheels.set_velocity(izq, der)` | cm/s; API genérica ARGoS |
| IR ×8 | `miniqupa_ir` | `robot.miniqupa_ir[i]` | Proximidad normalizada 0–1; IDs IR1–IR8 |
| Encoder | `miniqupa_encoder` | `robot.wheels.encoder` | `distance_left`, `distance_right`: m por paso; `axis_length`: m |
| Color inferior | `miniqupa_floor_color` | `robot.floor_color` | `valid` y `color` RGB ideal del suelo |
| Cámara de blobs | `miniqupa_camera` | `robot.camera.blobs` | Color, azimut y ancho angular; sin ID ni distancia métrica |
| Difusor | `miniqupa_diffuser` | `robot.diffuser.set_color(r,g,b)` | RGB 0–255 común al difusor |
| Radio | `simple_radios` | `robot.simple_radios.espnow` | `send({bytes})` y `recv`; API nativa |
| RAB | `range_and_bearing` | `robot.range_and_bearing` | Formato nativo ARGoS; distancia en cm, ángulos en radianes |
| Tag superior | Atributo `tag_id` de la entidad | Detectado por un sensor de tags externo | ID numérico y representación visual |

IR usa `min_range="0.04"` y alcance de entidad `ir_range="0.40"` por defecto. `noise_level` añade ruido uniforme opcional y `show_rays` permite ver los rayos. IR2 e IR3 miran hacia el frente. `distance_m` de IR es diagnóstico ideal, no la salida de proximidad del hardware.

Los medios deben coincidir entre entidad/controlador y `<media>`: `leds` para cámara/difusor, `radios` para radio, `rab` para RAB y `tags` si se usa tag. La simulación del suelo del ejemplo es blanca; el sensor inferior consulta el color del suelo configurado en ARGoS.

## Configurar el robot

```xml
<miniqupa id="mq0"
          ir_range="0.40"
          radio_medium="radios" radio_range="1.0"
          rab_range="3.0" rab_data_size="10"
          tag_id="7">
  <body position="0,0,0" orientation="0,0,0"/>
  <controller config="smoke"/>
</miniqupa>
```

`config="smoke"` corresponde al controlador del ejemplo. Usa el ID de tu controlador en tus escenas. Añade más entidades con IDs distintos y poses separadas; puedes asignar un `tag_id` distinto a cada una. Si omites `tag_id`, no se crea el marcador.

| Atributo de entidad | Valor inicial | Significado |
|---|---|---|
| `ir_range` | 0,40 m | Alcance máximo IR |
| `radio_medium` | Sin medio | Activa la radio `espnow` al asignar un medio |
| `radio_range` | 1 m | Alcance simple radios |
| `radio_position` | `0,0,0` m | Referencia virtual de la antena |
| `rab_range` | 3 m | Alcance RAB |
| `rab_data_size` | 10 bytes | Tamaño de datos RAB |
| `rab_position` | `0,0,0.1` m | Referencia virtual RAB |
| `tag_id` | Ausente | Entero entre 0 y 4294967295 |
| `tag_medium` | `tags` | Medio del tag |
| `tag_side_length` | 0,06 m | Lado negro del marcador |
| `tag_position` | `0,0,0.1676` m | Centro del tag |
| `tag_yaw` | 0° | Giro del tag alrededor de Z |

Las posiciones son locales: X hacia delante, Y hacia la izquierda y Z hacia arriba, con origen en el suelo bajo el centro del eje. Radio de rueda: 0,016 m; separación: 0,0795 m. La geometría CAD está en `config/miniqupa_geometry.json`; sus archivos fuente se enumeran como referencias de procedencia, no como dependencias de ejecución.

## Usarlo desde C++

Los controladores propios enlazan `argos3plugin_simulator_miniqupa`, además de sus dependencias ARGoS. Con el prefijo del plugin en una variable CMake `MINIQUPA_PREFIX`:

```cmake
find_package(ARGoS REQUIRED COMPONENTS genericrobot)
find_library(MINIQUPA_LIBRARY NAMES argos3plugin_simulator_miniqupa
  PATHS "${MINIQUPA_PREFIX}/lib/argos3" NO_DEFAULT_PATH REQUIRED)
target_include_directories(mi_controlador PRIVATE
  "${MINIQUPA_PREFIX}/include" ${ARGOS_INCLUDE_DIRS})
target_link_libraries(mi_controlador PRIVATE "${MINIQUPA_LIBRARY}")
```

Este fragmento complementa el CMake habitual de un controlador ARGoS, con el target `mi_controlador` ya creado y sus dependencias configuradas. Por ejemplo, desde un método del controlador:

```cpp
#include <argos3/plugins/robots/miniqupa/control_interface/ci_miniqupa_encoder_sensor.h>
// En Init():
auto* encoder = GetSensor<argos::CCI_MiniQupaEncoderSensor>("miniqupa_encoder");
// En ControlStep():
const auto& lectura = encoder->GetReading();
```

Las interfaces propias están en `control_interface/`. Ruedas, simple radios y RAB usan las interfaces genéricas de ARGoS. No hay que incluir los fuentes del simulador dentro del controlador.

## Integración dentro del árbol ARGoS

Esta es una alternativa a la compilación externa anterior. Copia el contenido del repositorio a `src/plugins/robots/miniqupa` del código fuente de ARGoS. En `src/plugins/robots/CMakeLists.txt`, añade una sola vez:

```cmake
if(ARGOS_BUILD_FOR_SIMULATOR)
  add_subdirectory(miniqupa)
endif()
```

Desde la raíz del repositorio ARGoS:

```bash
cmake -S src -B build_simulator
cmake --build build_simulator --target argos3plugin_simulator_miniqupa -j4
```

Ejecuta con el entorno de bibliotecas de esa misma compilación ARGoS. Mezclar plugins de dos compilaciones diferentes de ARGoS en un proceso puede producir registros duplicados y errores al cerrar.

## Estructura

```text
.
├── CMakeLists.txt
├── README.md
├── LICENSE                  # Licencia MIT
├── ARGoS_LICENSE.txt
├── control_interface/        # Interfaces C++ y Lua propias
├── simulator/                # Entidad, dynamics2d, sensores, actuadores y Qt
│   └── models/               # OBJ, MTL y registro de preparación
├── config/                   # Geometría derivada del CAD
├── cmake/                    # Generadores de headers geométricos
└── examples/                 # Escenario y controlador Lua mínimos
```

## Límites del modelo

- Física únicamente `dynamics2d`. Colisión circular conservadora; la malla visual no es una colisión detallada.
- Masa 0,4 kg, fricción 0,7, fuerza máxima 1,5 y torque máximo 1,5: parámetros provisionales, configurables como `mass`, `friction`, `max_force` y `max_torque`; no mediciones del robot físico.
- IR con respuesta tipo foot-bot reescalada a 4–40 cm; sin calibración del sensor real.
- Cámara geométrica de blobs, sin imagen renderizada, calibración real del espejo ni decodificación de tags.
- RGB inferior ideal; no reproduce cuentas crudas, iluminación ni electrónica TCS34725.
- Encoder equivalente de rodadura calculado a partir del movimiento físico del cuerpo; no simula pulsos de ejes ni deslizamiento independiente.
- El difusor tiene un color común. Sus muestras ópticas virtuales no equivalen a posiciones medidas de los cinco LEDs internos.
- Simple radios permite intercambiar bytes; no reproduce el protocolo completo ESP-NOW. El ejemplo usa directamente la API nativa, sin una capa de direccionamiento adicional.
- Tag nativo geométrico con patrón visual ilustrativo; no es una imagen AprilTag válida para un detector externo.
- Sin IMU, LEDs inferiores ni piso fotosensible. Los parámetros físicos y ópticos siguen pendientes de calibración real.

## Solución de problemas

- **CMake no encuentra ARGoS:** instala sus archivos de desarrollo o configura `CMAKE_PREFIX_PATH` con su prefijo.
- **No encuentra el plugin:** revisa `ARGOS_PLUGIN_PATH`; para sus dependencias revisa `LD_LIBRARY_PATH`. Exporta las variables en la terminal donde ejecutas ARGoS.
- **No encuentra `demo.lua`:** ejecuta desde la raíz del repositorio.
- **No aparece la visualización MiniQupa:** confirma que ARGoS y el plugin se compilaron con Qt/OpenGL; sin ello usa `-z`.
- **Faltan modelos:** reinstala el prefijo completo. Opcionalmente, `ARGOS_MINIQUPA_MODEL_DIR` puede apuntar a una carpeta que contenga `miniqupa.obj` y `miniqupa.mtl`.
- **Un dispositivo o medio no existe:** compara el XML con `examples/demo.argos`, incluidos nombres e implementaciones.

## Atribución y licencia

Este proyecto se distribuye bajo la **licencia MIT**; consulta [LICENSE](LICENSE). Incluye el código del plugin, su documentación, ejemplos y las mallas MiniQupa incluidas en este repositorio.

La implementación reutiliza patrones y componentes de ARGoS, desarrollado por Carlo Pinciroli y colaboradores. Se conserva su aviso de licencia en [ARGoS_LICENSE.txt](ARGoS_LICENSE.txt). Las dimensiones y mallas MiniQupa proceden del CAD de este robot.
