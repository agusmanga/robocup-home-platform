from ament_index_python import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node
import xacro

def generate_launch_description():
    description_pkg = get_package_share_directory('robocup_description')

    xacro_file = description_pkg + '/urdf/robot.urdf.xacro'

    robot_description = xacro.process_file(xacro_file).toxml()

    rviz = Node(
        package='rviz2',
        executable='rviz2',
        name='rviz2',
        arguments=['-d', description_pkg + '/rviz/robocup.rviz'],
        output='screen'
    )
    robot_state_publisher = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        name='robot_state_publisher',
        output='screen',
            parameters=[{'robot_description': robot_description}]
        )
    return LaunchDescription([rviz, robot_state_publisher])