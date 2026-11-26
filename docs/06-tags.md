# Tags y escenarios con varios robots

[Índice](README.md)

## Asignar un tag distinto a cada robot

```xml
<miniqupa id="mq0" tag_id="7" tag_medium="tags">
  <body position="-0.15,0,0" orientation="0,0,0"/>
  <controller config="smoke"/>
</miniqupa>
<miniqupa id="mq1" tag_id="8" tag_medium="tags">
  <body position="0.15,0,0" orientation="0,0,0"/>
  <controller config="smoke"/>
</miniqupa>
```

En `<media>`:

```xml
<tag id="tags" grid_size="10,10,5"/>
```

`tag_id` acepta un entero de 0 a 4294967295. Su ausencia significa que no se crea un tag; no equivale a ID 0. Usa identificadores distintos si tu aplicación exige unicidad: el escenario es quien los asigna. Tampoco confundas el `id` de entidad, el ID del controlador, el ID lógico de comunicaciones y el `tag_id`.

## Posición y dimensiones

| Atributo | Inicial | Significado |
|---|---|---|
| `tag_medium` | `tags` | ID del medio compartido |
| `tag_side_length` | 0,06 m | Lado del cuadrado negro |
| `tag_position` | `0,0,0.1676` m | Referencia local del tag, encima de la placa CAD |
| `tag_yaw` | 0° | Rotación local alrededor de Z |

El margen blanco visual ocupa 0,075 m con el tamaño inicial, sobre una placa de 0,08 m. El tag apunta hacia arriba. Posición, tamaño y yaw se pueden sobrescribir en la entidad; no son parámetros de la cámara panorámica.

## Qué robot lo detecta

El tag es un componente de la entidad, no un actuador ni un sensor de lectura propia. No hay `robot.tag` en Lua MiniQupa ni necesitas `GetSensor()` para llevar un marcador.

Un dron u otro observador necesita su **sensor de cámara con algoritmo detector de tags nativos ARGoS**, configurado con el mismo medio y orientado hacia la placa. Esa cámara externa no se añade en este plugin. Su campo de visión, alcance, orientación del tag y oclusiones afectan la detección. La cámara panorámica MiniQupa solo devuelve blobs de emisores LED; no devuelve tags.

El patrón dibujado representa el ID de forma ilustrativa; **no es una imagen de una familia AprilTag física**. Un detector AprilTag de ROS/OpenCV aplicado a una captura de pantalla no es la vía prevista para detectarlo. El detector nativo trabaja con las entidades geométricas del simulador.

[neighbors.argos](../examples/neighbors.argos) muestra dos tags diferentes y permite verlos en Qt. Sus comprobaciones automáticas validan comunicación y blobs, no una cámara aérea; la detección aérea requiere un controlador/sensor externo y no se incluye como ejemplo ejecutable aquí.

## Modificar el escenario

- Separa cuerpos al crear varios robots: diámetro de colisión aproximado 0,10841 m.
- Ajusta el tamaño/centro de la arena y los índices de medios cuando amplíes el escenario.
- Declara en cada controlador solo los dispositivos necesarios; un robot puede emitir luz sin leer cámara, o recibir radio sin controlar ruedas.
- `mass`, `friction`, `max_force` y `max_torque` son atributos de entidad para la física provisional. El plugin implementa dynamics2d; no basta con cambiar el nombre del motor a dynamics3d.
