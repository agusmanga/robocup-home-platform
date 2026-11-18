# RoboCup Home Platform

Plataforma desarrollada en **ROS 2 Humble** para un robot móvil omnidireccional con ruedas mecanum, orientado a RoboCup@Home.

El repositorio contempla dos entornos de ejecución:

- **Simulación:** Gazebo Sim, RViz, `ros2_control`, SLAM Toolbox y Nav2.
- **Robot físico:** motores STEADYWIN GIM6010-8 con drivers compatibles con ODrive, comunicación mediante USB-CAN (SocketCAN) y control mecanum a través de `ros2_control`.

Ambos entornos comparten la arquitectura ROS 2, pero utilizan descripciones y configuraciones de hardware diferentes.

## Estructura del repositorio

```text
robocup-home-platform/
└── ros2_ws/
    ├── README.md
    └── src/
        ├── robocup_description/
        │   ├── urdf/
        │   │   ├── robot.urdf.xacro
        │   │   └── odrive.urdf.xacro
        │   └── rviz/
        │       └── robocup.rviz
        ├── robocup_bringup/
        │   ├── launch/
        │   │   ├── display.launch.py
        │   │   └── odrive.launch.py
        │   ├── config/
        │   │   ├── gazebo_hardware_controllers.yaml
        │   │   ├── odrive_controllers.yaml
        │   │   ├── ros_gz_bridge.yaml
        │   │   └── nav2_params.yaml
        │   └── worlds/
        │       └── inco.world.sdf
        └── robocup_hardware/
            ├── src/
            │   ├── gazebo-hardware.cpp
            │   └── odrive_hardware_interface.cpp
            └── odrive_base/
```

### Paquetes

- **`robocup_description`:** modelos URDF/Xacro, links, joints, geometría del robot y configuración de RViz.
- **`robocup_bringup`:** archivos launch, parámetros de controladores y navegación, bridge ROS–Gazebo y mundos de simulación.
- **`robocup_hardware`:** interfaces de hardware para la simulación y para los drivers ODrive físicos mediante CAN.

## Requisitos

- Ubuntu 22.04 con **ROS 2 Humble**.
- `colcon`, `rosdep` y paquetes ROS 2 requeridos por el workspace.
- Para simulación: Gazebo Sim, `ros_gz_sim`, `ros_gz_bridge`, RViz, SLAM Toolbox y Nav2.
- Para robot físico: drivers compatibles con el protocolo CAN de ODrive, adaptador USB-CAN compatible con SocketCAN y una configuración funcional de los motores.

Instalar herramientas de compilación y gestión de dependencias:

```bash
sudo apt update
sudo apt install python3-colcon-common-extensions python3-rosdep
```

Cargar ROS 2:

```bash
source /opt/ros/humble/setup.bash
```

Inicializar `rosdep` (solo la primera vez) y actualizarlo:

```bash
sudo rosdep init
rosdep update
```

Desde el workspace, instalar las dependencias declaradas:

```bash
cd ~/robocup-home-platform/ros2_ws
rosdep install --from-paths src --ignore-src -r -y
```

> Algunas dependencias externas, como los paquetes de drivers de sensores, pueden requerir instalación adicional según el entorno. `rosdep` no garantiza instalar paquetes que no estén declarados o disponibles en sus fuentes.

## Compilar el proyecto

```bash
cd ~/robocup-home-platform/ros2_ws
source /opt/ros/humble/setup.bash
colcon build
source install/setup.bash
```

En cada terminal nueva que vaya a ejecutar comandos del proyecto:

```bash
source /opt/ros/humble/setup.bash
source ~/robocup-home-platform/ros2_ws/install/setup.bash
```

---

## Simulación con Gazebo

La simulación utiliza el modelo `robocup_description/urdf/robot.urdf.xacro` y la configuración de controladores `robocup_bringup/config/gazebo_hardware_controllers.yaml`.

Para iniciar el entorno:

```bash
ros2 launch robocup_bringup display.launch.py
```

Este launch:

1. Inicia Gazebo Sim con `inco.world.sdf`.
2. Publica la descripción y los TF del robot mediante `robot_state_publisher`.
3. Inserta el robot en Gazebo.
4. Inicia el bridge ROS–Gazebo.
5. Carga `joint_state_broadcaster` y `mecanum_drive_controller` después de insertar el robot.
6. Inicia RViz, SLAM Toolbox y Nav2, utilizando tiempo simulado (`use_sim_time`).

La configuración de navegación se encuentra en `config/nav2_params.yaml`.

> El launch define un bloque para `sllidar_ros2`, pero actualmente **no lo incluye** en la lista de acciones retornadas. No debe asumirse que ese driver se inicia al ejecutar `display.launch.py`.

Para verificar el sistema:

```bash
ros2 control list_controllers
ros2 topic list
ros2 topic echo /odom
```

---

## Robot físico con ODrive y USB-CAN

El soporte de control físico fue incorporado en el commit [`bc034e8`](https://github.com/agusmanga/robocup-home-platform/commit/bc034e8f15daa95dd474da6a60e0c19dc6ea295c).

La comunicación sigue este esquema:

```text
/cmd_vel
   |
   v
mecanum_drive_controller
   |
   v
ros2_control / ODriveHardwareInterface
   |
   v
SocketCAN (can0)
   |
   v
Adaptador USB-CAN
   |
   v
Bus CAN ---> Driver ODrive 1 ---> Rueda delantera izquierda
        |--> Driver ODrive 2 ---> Rueda delantera derecha
        |--> Driver ODrive 3 ---> Rueda trasera izquierda
        `--> Driver ODrive 4 ---> Rueda trasera derecha
```

**La configuración por USB-C se realiza inicialmente en cada driver.** Una vez que los motores están configurados y calibrados, el uso habitual del robot se hace mediante CAN y el launch `odrive.launch.py`.

### 1. Configuración inicial de los drivers por USB-C

Antes del primer arranque:

1. Conectar cada driver al equipo por USB-C y suministrar la alimentación apropiada.
2. Utilizar ODrive Tool para completar la configuración y calibración necesarias del motor, encoder y driver.
3. Verificar que cada motor funciona correctamente.
4. Asignar un **Node ID CAN único** a cada driver.
5. Guardar la configuración para que persista después de apagarlo.

Activar primero el entorno virtual de Python utilizado para ODrive Tool (creado durante la instalación inicial):
```bash
source ~/odrive-venv/bin/activate
```

Una vez activo el entorno virtual (el prompt debe mostrar (odrive-venv)), abrir la consola ODrive con el driver conectado mediante USB-C:

Abrir la consola ODrive:

```bash
odrivetool
```

Una vez que el dispositivo está disponible como `odrv0`, asignar su ID. Por ejemplo, **Node ID 2**:

```python
odrv0.axis0.config.can.node_id = 2
odrv0.save_configuration()
```

Repetir con cada driver, respetando la siguiente asignación utilizada en `odrive.urdf.xacro`:

| Rueda | Joint ROS 2 | Node ID |
| --- | --- | ---: |
| Delantera izquierda | `front_left_wheel_joint` | 1 |
| Delantera derecha | `front_right_wheel_joint` | 2 |
| Trasera izquierda | `rear_left_wheel_joint` | 3 |
| Trasera derecha | `rear_right_wheel_joint` | 4 |

Comprobar el valor configurado:

```python
odrv0.axis0.config.can.node_id
```

> La correspondencia entre Node ID y rueda se define en el URDF físico. Si se cambia la asignación, debe actualizarse allí también. Todos los drivers deben utilizar el **mismo bitrate CAN (500 kbit/s)**.

No es necesario dejar los drivers manualmente en modo velocidad ni en `CLOSED_LOOP_CONTROL` antes de cada arranque de ROS 2: **la interfaz `ODriveHardwareInterface` solicita esos modos cuando se activa el controlador**. La calibración y configuración persistente inicial siguen siendo necesarias.

### 2. Conexión física del bus CAN

Conectar los drivers al mismo bus CAN y el adaptador USB-CAN al equipo Ubuntu.

- Respetar CAN-H, CAN-L y la referencia de masa cuando corresponda al hardware.
- Colocar la terminación apropiada en **ambos extremos** del bus CAN.
- Confirmar alimentación y polaridad de todos los drivers.
- Utilizar un adaptador que exponga una interfaz compatible con **SocketCAN** en Linux.

**Seguridad:** realizar las primeras pruebas con las ruedas levantadas del suelo, sin personas ni objetos cerca del mecanismo, y con una forma de detener el sistema rápidamente.

### 3. Activar SocketCAN

Verificar que el adaptador está detectado:

```bash
ip link show
ip -details link show can0
```

Levantar la interfaz a **500000 bit/s**:

```bash
sudo ip link set can0 up type can bitrate 500000
```

Si `can0` ya estaba activa con otra configuración:

```bash
sudo ip link set can0 down
sudo ip link set can0 up type can bitrate 500000
```

Opcionalmente, aumentar la cola de transmisión (útil también para herramientas de ODrive):

```bash
sudo ip link set dev can0 txqueuelen 256
```

Verificar el estado y la velocidad:

```bash
ip -details -statistics link show can0
```

Para observar el tráfico CAN durante el diagnóstico, instalar `can-utils` si hace falta y ejecutar:

```bash
sudo apt install can-utils
candump can0
```

Si aparecen tramas CAN, significa que se está recibiendo tráfico en la interfaz, aunque esto por sí solo no garantiza que cada motor esté correctamente configurado.

> El launch de ROS 2 **no crea ni configura `can0`**. La interfaz debe estar operativa antes de ejecutarlo.

### 4. Lanzar el robot físico

Desde el workspace compilado:

```bash
cd ~/robocup-home-platform/ros2_ws
source /opt/ros/humble/setup.bash
source install/setup.bash
ros2 launch robocup_bringup odrive.launch.py
```

Este launch utiliza:

- `robocup_description/urdf/odrive.urdf.xacro`: descripción física de la base y configuración de los Node IDs CAN.
- `robocup_bringup/config/odrive_controllers.yaml`: configuración de `controller_manager` y del controlador mecanum.
- `robocup_hardware/src/odrive_hardware_interface.cpp`: implementación de comunicación CAN y gestión del modo de control.

El launch inicia:

1. `robot_state_publisher`.
2. `ros2_control_node` con `ODriveHardwareInterface`, que abre `can0`.
3. El `spawner` de `mecanum_drive_controller`.

**Configuración automática de los motores:** cuando el controlador activa las interfaces de velocidad, `ODriveHardwareInterface` envía por CAN los comandos necesarios para seleccionar `VELOCITY_CONTROL`, usar `INPUT_MODE_PASSTHROUGH`, limpiar errores y solicitar `CLOSED_LOOP_CONTROL`. Cuando el hardware se desactiva, solicita `IDLE`.

La interfaz convierte internamente las consignas de velocidad entre **rad/s (ROS 2)** y **vueltas/s (ODrive)**.

> Este launch es para el movimiento de la base física. A diferencia de `display.launch.py`, **no inicia Gazebo, RViz, SLAM ni Nav2**. Tampoco lanza explícitamente el `joint_state_broadcaster`, aunque el controlador está declarado en el YAML.

### 5. Cinemática mecanum

El controlador físico se configura en `config/odrive_controllers.yaml`:

```yaml
mecanum_drive_controller:
  ros__parameters:
    front_left_wheel_command_joint_name: "front_left_wheel_joint"
    front_right_wheel_command_joint_name: "front_right_wheel_joint"
    rear_left_wheel_command_joint_name: "rear_left_wheel_joint"
    rear_right_wheel_command_joint_name: "rear_right_wheel_joint"
    base_frame_id: "base_footprint"
    odom_frame_id: "odom"
    enable_odom_tf: true
    reference_timeout: 5.0
    kinematics:
      base_frame_offset: {x: 0.0, y: 0.0, theta: 0.0}
      wheels_radius: 0.075
      sum_of_robot_center_projection_on_X_Y_axis: 0.54
    use_stamped_vel: false
```

El radio de cada rueda es **0.075 m**. Para la geometría modelada, la distancia longitudinal desde el centro a cada eje es 0.275 m y la lateral hasta la rueda es 0.265 m, por lo que:

```text
0.275 m + 0.265 m = 0.54 m
```

La cinemática se calcula usando los valores del **YAML del controlador**. La geometría, masas e inercias del URDF no son utilizadas directamente por este controlador para calcular las velocidades de rueda; el URDF sí describe la estructura del robot y las interfaces de hardware.

### 6. Controlar el robot por `/cmd_vel`

El launch remapea:

| Interfaz ROS 2 | Uso |
| --- | --- |
| `/cmd_vel` | Entrada de velocidades lineales y angulares de la base (`geometry_msgs/msg/Twist`) |
| `/odom` | Odometría calculada por el controlador mecanum |
| `/tf` | Transformación de odometría cuando está habilitada |

Comprobar los controladores:

```bash
ros2 control list_controllers
```

Verificar los tópicos:

```bash
ros2 topic list
ros2 topic echo /odom
```

Con las ruedas **levantadas** y luego de verificar que el controlador está activo, probar un comando pequeño y único:

```bash
ros2 topic pub --once /cmd_vel geometry_msgs/msg/Twist \
  '{linear: {x: 0.05}, angular: {z: 0.0}}'
```

Detener las consignas de movimiento con un comando de velocidad cero:

```bash
ros2 topic pub --once /cmd_vel geometry_msgs/msg/Twist \
  '{linear: {x: 0.0, y: 0.0, z: 0.0}, angular: {x: 0.0, y: 0.0, z: 0.0}}'
```

> Un comando de velocidad cero **no sustituye una parada de emergencia**. Comprobar los sentidos de giro de las cuatro ruedas antes de apoyar el robot en el suelo. El controlador tiene `reference_timeout: 5.0`, por lo que no se debe depender del timeout como mecanismo de seguridad.

### 7. Diagnóstico rápido

| Problema | Qué revisar |
| --- | --- |
| No aparece `can0` | Conexión USB, soporte del adaptador, driver SocketCAN |
| `can0` no se levanta | Estado previo de la interfaz, bitrate, permisos de `sudo` |
| No llegan tramas CAN | Alimentación, CAN-H/CAN-L, terminaciones, bitrate de los drivers |
| Un motor no responde | Node ID asignado por USB-C frente al ID declarado en el URDF |
| El driver queda en `IDLE` | Estado de `mecanum_drive_controller`, activación de interfaces y posibles errores del driver |
| Los motores giran en sentido incorrecto | Orientación física, signos de los ejes y configuración de cada motor |
| El robot gira o se desplaza incorrectamente | Orden de las ruedas, radio y distancias de la cinemática, signos de velocidad |
| Odometría incorrecta | Estimaciones del encoder, recepción de tramas CAN, geometría del controlador |

Comandos útiles:

```bash
# Estado de SocketCAN
ip -details -statistics link show can0

# Tráfico CAN
candump can0

# Estado de los controladores
ros2 control list_controllers

# Interfaces disponibles
ros2 control list_hardware_interfaces

# Odometría
ros2 topic echo /odom
```

Para inspeccionar errores de un driver conectado individualmente mediante `odrivetool`:

```python
dump_errors(odrv0)
```

## Flujo de trabajo resumido

**Primera puesta en marcha (una vez por driver):**

1. Configurar y calibrar el motor con ODrive Tool mediante USB-C.
2. Asignar un Node ID único (`1`, `2`, `3` o `4`).
3. Ejecutar `odrv0.save_configuration()`.
4. Confirmar bitrate y comunicación CAN.

**Arranque habitual:**

```bash
# 1. Activar SocketCAN
sudo ip link set can0 up type can bitrate 500000

# 2. Cargar ROS 2 y el workspace
source /opt/ros/humble/setup.bash
source ~/robocup-home-platform/ros2_ws/install/setup.bash

# 3. Iniciar control de la base física
ros2 launch robocup_bringup odrive.launch.py
```

En cambio, para trabajar en **simulación**:

```bash
source /opt/ros/humble/setup.bash
source ~/robocup-home-platform/ros2_ws/install/setup.bash
ros2 launch robocup_bringup display.launch.py
```

---

### Instalación del driver RPLIDAR

El proyecto integró opcionalmente el paquete `sllidar_ros2` de SLAMTEC
para la integración del sensor RPLIDAR C1.

Este paquete es una dependencia externa y no se incluye
directamente en el repositorio.

Si se integra, es necesario agregar 'lidar_launch' a la Launch Description en display.launch.py

Para instalarlo, desde el workspace:

```bash
cd ~/robocup-home-platform/ros2_ws/src

git clone https://github.com/Slamtec/sllidar_ros2.git

cd ~/robocup-home-platform/ros2_ws

colcon build --symlink-install

source install/setup.bash
```

---

## Referencias

- [ROS 2 Humble](https://docs.ros.org/en/humble/)
- [ros2_control](https://control.ros.org/humble/)
- [mecanum_drive_controller](https://control.ros.org/humble/doc/ros2_controllers/mecanum_drive_controller/doc/userdoc.html)
- [ODrive Documentation](https://docs.odriverobotics.com/)
- [Commit de integración ODrive](https://github.com/agusmanga/robocup-home-platform/commit/bc034e8f15daa95dd474da6a60e0c19dc6ea295c)
