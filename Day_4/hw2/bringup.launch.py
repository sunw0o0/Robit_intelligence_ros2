from launch import LaunchDescription
from launch.actions import RegisterEventHandler
from launch.event_handlers import OnProcessExit
from launch.substitutions import Command, FindExecutable, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():
    # ── 1. robot_description을 mock 하드웨어 설정으로 생성 ──
    # display.launch.py와 비슷한 방식으로 xacro를 실행하되,
    # eclipse.xacro가 받는 인자(use_mock_*, with_*)를 전부 true로 넘겨줌.
    # robot_description 패키지 자체는 전혀 수정하지 않고, 여기서 인자만 넘기는 것.
    urdf_file = Command(
        [
            PathJoinSubstitution([FindExecutable(name="xacro")]),
            " ",
            PathJoinSubstitution(
                [FindPackageShare("robot_description"), "urdf", "eclipse.xacro"]
            ),
            " use_mock_base:=true",
            " use_mock_arm:=true",
            " use_mock_ct:=true",
            " with_base:=true",
            " with_arm:=true",
            " with_camera_tower:=true",
        ]
    )
    robot_description = {"robot_description": ParameterValue(urdf_file, value_type=str)}

    # 우리 패키지 안에 있는 컨트롤러 YAML 경로
    controllers_yaml = PathJoinSubstitution(
        [FindPackageShare("eclipse_bringup_sunwoo"), "config", "eclipse_controllers.yaml"]
    )

    # rviz 설정은 robot_description에 있던 기존 파일을 그대로 재사용
    # (robot_description 패키지는 수정하지 않고 "읽기"만 하는 거라 규칙 위반 아님)
    rviz_config_file = PathJoinSubstitution(
        [FindPackageShare("robot_description"), "config", "eclipse.rviz"]
    )

    # ── 2. robot_state_publisher: URDF를 바탕으로 /tf, /robot_description을 뿌려줌 ──
    robot_state_publisher_node = Node(
        package="robot_state_publisher",
        executable="robot_state_publisher",
        parameters=[robot_description],
        output="screen",
    )

    # ── 3. controller_manager (ros2_control_node): Part 2에서 배운
    #    "read() -> update() -> write()" 루프를 실제로 도는 핵심 노드 ──
    controller_manager_node = Node(
        package="controller_manager",
        executable="ros2_control_node",
        parameters=[robot_description, controllers_yaml],
        output="screen",
    )

    # ── 4. rviz2: 로봇을 시각적으로 확인 ──
    rviz_node = Node(
        package="rviz2",
        executable="rviz2",
        arguments=["-d", rviz_config_file],
        output="screen",
    )

    # ── 5. 컨트롤러 spawner들: controller_manager에게
    #    "이 컨트롤러를 로드하고 active로 activate 해줘"라고 대신 요청하는 도구 ──
    # (지난 슬라이드에서 배운 "spawner가 load -> configure -> activate를 대신 해줌"이 이 부분)
    joint_state_broadcaster_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["joint_state_broadcaster"],
        output="screen",
    )

    arm_controller_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["arm_controller"],
        output="screen",
    )

    camera_tower_controller_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["camera_tower_controller"],
        output="screen",
    )

    base_drive_controller_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["base_drive_controller"],
        output="screen",
    )

    flipper_controller_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["flipper_controller"],
        output="screen",
    )

    # ── 6. 실행 순서 제어 ──
    # joint_state_broadcaster가 먼저 완전히 뜬 다음에 나머지 컨트롤러들을 띄우도록
    # RegisterEventHandler로 "얘가 끝나면(spawner는 작업 끝나면 프로세스 종료됨) 다음 걸 실행해"라고 예약.
    # 이렇게 안 해도 대부분 동작은 하지만, 순서를 명확히 해서 경합(race condition)을 줄임.
    delay_after_jsb = RegisterEventHandler(
        event_handler=OnProcessExit(
            target_action=joint_state_broadcaster_spawner,
            on_exit=[
                arm_controller_spawner,
                camera_tower_controller_spawner,
                base_drive_controller_spawner,
                flipper_controller_spawner,
            ],
        )
    )

    return LaunchDescription(
        [
            robot_state_publisher_node,
            controller_manager_node,
            rviz_node,
            joint_state_broadcaster_spawner,
            delay_after_jsb,
        ]
    )
