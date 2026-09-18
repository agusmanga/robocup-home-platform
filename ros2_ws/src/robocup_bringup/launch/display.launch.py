import os

from ament_index_python import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
import xacro

def generate_launch_description():
    description_pkg = get_package_share_directory('robocup_description')

    ros_gz_sim_pkg = get_package_share_directory('ros_gz_sim')
    bringup_pkg = get_package_share_directory('robocup_bringup')

    controllers_file = os.path.join(
        bringup_pkg,
        'config',
        'gazebo_hardware_controllers.yaml'
    )

    world_file = os.path.join(
        bringup_pkg,
        'worlds',
        'inco.world.sdf'
    )
    

    xacro_file = description_pkg + '/urdf/robot.urdf.xacro'

    robot_description = xacro.process_file(xacro_file,mappings={
        'controllers_file': controllers_file
    }).toxml()

    gazebo= IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(ros_gz_sim_pkg, 'launch', 'gz_sim.launch.py')
        ),
        launch_arguments={'gz_args': f'-r {world_file}'}.items()
    )

    rviz = Node(
        package='rviz2',
        executable='rviz2',
        name='rviz2',
        arguments=['-d', description_pkg + '/rviz/robocup.rviz'],
        parameters=[{'use_sim_time': True}],
        output='screen'
    )
    robot_state_publisher = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        name='robot_state_publisher',
        output='screen',
            parameters=[{'robot_description': robot_description}]
        )
    wheel_controller_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=['mecanum_drive_controller',
            '--param-file',
            controllers_file,
        ],
    )

    joint_state_broadcaster_spawner = Node(
    package="controller_manager",
    executable="spawner",
    arguments=["joint_state_broadcaster"],
)

    bridge = Node(
        package='ros_gz_bridge',
        executable='parameter_bridge',
        parameters=[{
            'config_file': os.path.join(bringup_pkg, 'config', 'ros_gz_bridge.yaml'),
        }],
        output='screen'
    )



    robot = Node(
        package='ros_gz_sim',
        executable='create',
        name='robot',
        output='screen',
        arguments=[
            '-topic', '/robot_description',
            '-name', 'robocup_robot',
            '-z', '0.2'
        ]
    )
    
    return LaunchDescription([rviz,bridge, gazebo, robot_state_publisher, robot,wheel_controller_spawner,joint_state_broadcaster_spawner])