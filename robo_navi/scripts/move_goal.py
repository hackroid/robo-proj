#!/usr/bin/env python
# -*- coding: utf-8 -*-
import csv
import roslib;
import rospy
import actionlib
import roslaunch
import time,subprocess as sp
from tf.transformations import quaternion_from_euler
from actionlib_msgs.msg import *
from geometry_msgs.msg import Pose, PoseWithCovarianceStamped, Point, Quaternion, Twist
from move_base_msgs.msg import MoveBaseAction, MoveBaseGoal

# class GoalManager(object):
#     def __init__(self):

def read_data(file_name):
    dt = []
    with open(file_name) as csvfile:
        spamreader = csv.reader(csvfile, delimiter=',')
        for row in spamreader:
            rr = []
            for num in row:
                rr.append(float(num))
            dt.append(rr)
    return dt

def move_to(pose, robot_name):
    goal_x, goal_y, goal_z, goal_R, goal_P, goal_Y = pose
    # robot_name = rospy.get_param('~robot_name','tb3_0')
    # goal_x= float(rospy.get_param('x','0',))
    # goal_y= float(rospy.get_param('~y','0'))
    # goal_z= float(rospy.get_param('~z','0'))
    # goal_Y= float(rospy.get_param('~Y','0'))
    # goal_P= float(rospy.get_param('~P','0'))
    # goal_R= float(rospy.get_param('~R','0'))
    # goal_d= float(rospy.get_param('/d','0')
    goal_Y = goal_Y #- 1.57

    # 节点初始化
    
    # 订阅move_base服务器的消息
    move_base = actionlib.SimpleActionClient(robot_name+"/move_base", MoveBaseAction)
    rospy.loginfo("Waiting for move_base action server...")

    # 等待连接服务器，5s等待时间限制
    while move_base.wait_for_server(rospy.Duration(5.0)) == 0:
        rospy.loginfo("Connected to move base server")

    # 设定目标点
    target = Pose(Point(goal_x, goal_y, goal_z), Quaternion(*quaternion_from_euler(goal_R, goal_P, goal_Y)))
    # target = Pose(Point(goal_x, goal_y, goal_z), )
    goal = MoveBaseGoal()
    goal.target_pose.pose = target
    goal.target_pose.header.frame_id = 'map'
    goal.target_pose.header.stamp = rospy.Time.now()

    rospy.loginfo("Going to: " + str(target))
    # goal_x= float(rospy.get_param('/x','0',))
    # goal_y= float(rospy.get_param('/y','0'))
    # goal_z= float(rospy.get_param('/z','0'))
    # goal_Y= float(rospy.get_param('/Y','0'))
    # goal_P= float(rospy.get_param('/P','0'))
    # goal_R= float(rospy.get_param('/R','0'))
    # 向目标进发
    move_base.send_goal(goal)

    # 五分钟时间限制
    finished_within_time = move_base.wait_for_result(rospy.Duration(300))

    # 查看是否成功到达
    if not finished_within_time:
        move_base.cancel_goal()
        rospy.loginfo("Timed out achieving goal")
    else:
        state = move_base.get_state()
        if state == GoalStatus.SUCCEEDED:
            rospy.loginfo("Goal succeeded!")
        else:
            rospy.loginfo("Goal failed！ ")

if __name__ == "__main__":
    rospy.init_node('move_goal', anonymous=True)
    goals_file = rospy.get_param('~goals_file')
    robot_name = rospy.get_param('~robot_name','tb3_0')
    wait_time = rospy.get_param('~wait_time','0')
    dt = read_data(goals_file)
    time.sleep(float(wait_time))
    for goal in dt:
        move_to(goal,robot_name)
        # Capture
        # uuid = roslaunch.rlutil.get_or_generate_uuid(None, False)
        # roslaunch.configure_logging(uuid)
        # search_launch = roslaunch.parent.ROSLaunchParent(uuid, ["/home/robot2019/catkin_ws/src/robo_proj/pokemon_go/launch/pokemon.launch"])
        # search_launch.start()
        sp.check_output("rosrun pokemon_go pokemon_searching _search_num:=\"{}\"".format(robot_name), shell=True)
        time.sleep(3)
        