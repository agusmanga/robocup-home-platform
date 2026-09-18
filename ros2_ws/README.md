# RoboCup Home Platform

Plataforma ROS 2 para el desarrollo de un robot móvil omnidireccional orientado a RoboCup@Home.

El proyecto utiliza actualmente **ROS 2 Humble**.

## Estructura del repositorio

```text
robocup-home-platform/
├── README.md
└── ros2_ws/
    └── src/
        ├── robocup_description/
        ├── robocup_bringup/
        └── robocup_hardware/
```

### `robocup_description`

Contiene la descripción del robot:

- URDF / Xacro
- links y joints
- propiedades visuales
- geometrías de colisión
- propiedades inerciales
- configuración de RViz

### `robocup_bringup`

Contiene los archivos `launch` utilizados para levantar los distintos componentes del robot.

Actualmente permite iniciar la descripción del robot y RViz.

### `robocup_hardware`

Contendrá la interfaz de hardware de `ros2_control` encargada de comunicarse con los motores y drivers físicos.

---

## Requisitos

El proyecto utiliza **ROS 2 Humble** sobre Ubuntu 22.04.

Se asume que ROS 2 Humble ya está instalado.

Las dependencias del workspace se instalan mediante `rosdep`:

```bash
sudo apt update
sudo rosdep init   # solo la primera vez
rosdep update

cd ros2_ws
rosdep install --from-paths src --ignore-src -r -y

---

## Compilar el proyecto

Primero ir al workspace:

```bash
cd ~/robocup-home-platform/ros2_ws
```

Cargar ROS 2 Humble:

```bash
source /opt/ros/humble/setup.bash
```

Compilar todos los paquetes:

```bash
colcon build
```

Después de compilar, cargar los paquetes del workspace:

```bash
source install/setup.bash
```

---

## Levantar la visualización del robot

Para iniciar `robot_state_publisher` y RViz:

```bash
ros2 launch robocup_bringup display.launch.py
```

El launch procesa el archivo Xacro del robot, genera el URDF y lo pasa a `robot_state_publisher`.