from launch import LaunchDescription
from launch_ros.actions import LifecycleNode
from launch.actions import EmitEvent, RegisterEventHandler
from launch_ros.events.lifecycle import ChangeState
from launch_ros.event_handlers import OnStateTransition
from lifecycle_msgs.msg import Transition
from launch.events import matches_action


def generate_launch_description():
    # fake_imu 노드
    fake_imu_node = LifecycleNode(
        package='ros_day3',
        executable='fake_imu',
        name='fake_imu',
        namespace='',
        output='screen',
    )

    # imu_watchdog 노드
    imu_watchdog_node = LifecycleNode(
        package='ros_day3',
        executable='imu_watchdog',
        name='imu_watchdog',
        namespace='',
        output='screen',
    )

    # 1) fake_imu: 실행되자마자 configure 이벤트 보내기
    fake_imu_configure = EmitEvent(
        event=ChangeState(
            lifecycle_node_matcher=matches_action(fake_imu_node),
            transition_id=Transition.TRANSITION_CONFIGURE,
        )
    )

    # 2) fake_imu가 Inactive가 되면 -> activate 이벤트 보내기
    fake_imu_activate = RegisterEventHandler(
        OnStateTransition(
            target_lifecycle_node=fake_imu_node,
            goal_state='inactive',
            entities=[
                EmitEvent(
                    event=ChangeState(
                        lifecycle_node_matcher=matches_action(fake_imu_node),
                        transition_id=Transition.TRANSITION_ACTIVATE,
                    )
                )
            ],
        )
    )

    # 3) imu_watchdog: 실행되자마자 configure 이벤트 보내기
    imu_watchdog_configure = EmitEvent(
        event=ChangeState(
            lifecycle_node_matcher=matches_action(imu_watchdog_node),
            transition_id=Transition.TRANSITION_CONFIGURE,
        )
    )

    # 4) imu_watchdog이 Inactive가 되면 -> activate 이벤트 보내기
    imu_watchdog_activate = RegisterEventHandler(
        OnStateTransition(
            target_lifecycle_node=imu_watchdog_node,
            goal_state='inactive',
            entities=[
                EmitEvent(
                    event=ChangeState(
                        lifecycle_node_matcher=matches_action(imu_watchdog_node),
                        transition_id=Transition.TRANSITION_ACTIVATE,
                    )
                )
            ],
        )
    )

    return LaunchDescription([
        fake_imu_node,
        imu_watchdog_node,
        fake_imu_activate,
        imu_watchdog_activate,
        fake_imu_configure,
        imu_watchdog_configure,
    ])
