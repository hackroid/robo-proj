#!/usr/bin/env python 
# -*- coding: utf-8 -*-

import csv
import roslib
import rospy
import roslaunch
import actionlib
import os, subprocess as sp
import numpy as np
import time, thread
import move_goal
from tf.transformations import quaternion_from_euler
from actionlib_msgs.msg import *  
from geometry_msgs.msg import Pose, PoseWithCovarianceStamped, Point, Quaternion, Twist  
from move_base_msgs.msg import MoveBaseAction, MoveBaseGoal  

def read_data():
    dt = []
    with open('../maps/x.csv') as csvfile:
        spamreader = csv.reader(csvfile, delimiter=',')
        for row in spamreader:
            rr = []
            for num in row:
                rr.append(float(num))
            dt.append(rr)
    return np.array(dt)


def main():
    print("==== Auto Pokemon Sta15262ing ====")
    # Launching sim.launch
    uuid = roslaunch.rlutil.get_or_generate_uuid(None, False)
    roslaunch.configure_logging(uuid)
    sim_launch = roslaunch.parent.ROSLaunchParent(uuid, ["../../robo_spawn/launch/sim.launch"])
    sim_launch.start()
    rospy.loginfo("started")
    time.sleep(5)
    uuid = roslaunch.rlutil.get_or_generate_uuid(None, False)
    roslaunch.configure_logging(uuid)
    multi_launch = roslaunch.parent.ROSLaunchParent(uuid, ["../../robo_navi/launch/multi_nav_bringup.launch"])
    multi_launch.start()
    time.sleep(5)

    # sp.call('rosrun robo_navi move_goal.py x:=0 y:=1')
    try:
        thread.start_new_thread(move_goal.move_to, (0,1,0,0,0,0,'tb3_0',))    #函数名和它的两个参数
        thread.start_new_thread(move_goal.move_to, (0,-1,0,0,0,0,'tb3_1',))
    except:
        print "Error: unable to start thread"
    print("==== 1 ====")
    # sp.call('rosrun robo_navi move_goal.py robot_name:=\'tb3_1\'x:=0 y:=0.5')
    print("==== 2 ====")
    
    time.sleep(15)
    print("==== Auto Pokemon Finished ====")
    multi_launch.shutdown()
    sim_launch.shutdown()

if __name__ == "__main__":
    main()
    # read_data()