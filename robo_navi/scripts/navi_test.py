import rospy
from geometry_msgs.msg import Twist
import sys, select, os

if __name__=="__main__":
    if os.name != 'nt':
        settings = termios.tcgetattr(sys.stdin)

    rospy.init_node('turtlebot3_teleop')
    pub = rospy.Publisher('/robot1/move_base/goal', Twist, queue_size=10)

    turtlebot3_model = rospy.get_param("model", "burger")