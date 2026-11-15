import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():

    description_pkg = get_package_share_directory(
        'robocup_description'
    )

    bringup_pkg = get_package_share_directory(
        'robocup_bringup'
    )

    urdf_path = os.path.join(
        description_pkg,
        'urdf',
        'odrive_test.urdf.xacro'
    )

    controllers_path = os.path.join(
        bringup_pkg,
        'config',
        'odrive_controllers.yaml'
    )

    with open(urdf_path, 'r') as file:
        robot_description = file.read()

    return LaunchDescription([

        Node(
            package='robot_state_publisher',
            executable='robot_state_publisher',
            parameters=[{
                'robot_description': robot_description,
                'use_sim_time': False
            }],
            output='screen'
        ),

        Node(
            package='controller_manager',
            executable='ros2_control_node',
            parameters=[
                {'robot_description': robot_description},
                controllers_path
            ],
            remappings=[
                            ('/mecanum_drive_controller/reference_unstamped','/cmd_vel'),
                            ('/mecanum_drive_controller/tf_odometry' , '/tf'),
                            ('/mecanum_drive_controller/odometry','/odom')
                        ],
            output='screen'
        ),

        Node(
            package='controller_manager',
            executable='spawner',
            arguments=[
                'mecanum_drive_controller',
                '--controller-manager',
                '/controller_manager',
                '--param-file',
                controllers_path,
                
            ],
            
            output='screen'
        )
    ])