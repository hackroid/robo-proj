#!/usr/bin/env python
# -*- coding: utf-8 -*-
from __future__ import print_function

# import roslib
# roslib.load_manifest('pokemon_search')
import sys
import rospy
import cv2
from std_msgs.msg import String
from sensor_msgs.msg import Image
from cv_bridge import CvBridge, CvBridgeError
from pokemon_searching.msg import pokemonImage
import numpy as np


class image_converter:

    def __init__(self):
        self.image_pub = rospy.Publisher("image_topic_2", pokemonImage)
        self.bridge = CvBridge()
        self.image_sub = rospy.Subscriber(
            "/camera/rgb/image_raw", Image, self.callback)


    def callback(self, data):
        try:
            cv_image = self.bridge.imgmsg_to_cv2(data, "bgr8")
            # depth_image
        except CvBridgeError as e:
            print(e)
        (rows, cols, channels) = cv_image.shape
        # print(rows, cols, channels) 
        grey = cv2.cvtColor(cv_image, cv2.COLOR_BGR2GRAY)
        bgr = cv2.split(cv_image.astype('int16'))

        ret,g0 = cv2.threshold(grey,110,255,cv2.THRESH_BINARY_INV)
        ret,g1 = cv2.threshold(grey,80,255,cv2.THRESH_BINARY)
        ret,dst0 = cv2.threshold(abs(bgr[2]-bgr[0]),15,255,cv2.THRESH_BINARY_INV)
        ret,dst1= cv2.threshold(abs(bgr[2]-bgr[1]),10,255,cv2.THRESH_BINARY_INV)
        ret,dst2= cv2.threshold(abs(bgr[0]-bgr[1]),10,255,cv2.THRESH_BINARY_INV)
    
        th = (g0&g1&dst0&dst1&dst2).astype('uint8')

        kernel = cv2.getStructuringElement(cv2.MORPH_ELLIPSE, (4,4))
        kernel1 = cv2.getStructuringElement(cv2.MORPH_ELLIPSE, (20,20))
        opening = cv2.morphologyEx(th, cv2.MORPH_OPEN, kernel)
        opening1 = cv2.morphologyEx(opening, cv2.MORPH_CLOSE, kernel1)

        
        # opening = cv2.GaussianBlur(opening,(3,3),0)
        out_binary,contours,hierachy = cv2.findContours(opening1, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
        # cv2.drawContours(cv_image, contours,-1,(0,255,0),2)
        x0,y0,x1,y1=0,0,0,0
        pokemon_find = False
        if contours is not None and len(contours):
            pokemon_find = True
            cnts = np.concatenate(contours)
            # cv2.fillPoly(opening1, pts=[cnts], color=127)
            x,y,w,h = cv2.boundingRect(cnts)
            cv2.rectangle(cv_image,(x,y),(x+w,y+h),(255,255,0),2)
            x0,y0,x1,y1=x,y,x+w,y+h
        # for contour in contours:
        #     hull = cv2.convexHull(contour)
        #     cv2.fillPoly(opening1, pts=[hull], color=255)
        # opening1 = cv2.morphologyEx(opening1, cv2.MORPH_CLOSE, kernel1)
        # out_binary,contours,hierachy = cv2.findContours(opening1, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
        # for contour in contours:
        #     hull = cv2.convexHull(contour)
        #     cv2.drawContours(cv_image, [hull],-1,(0,255,0),2)

        #     x,y,w,h = cv2.boundingRect(hull)
        #     cv2.rectangle(cv_image,(x,y),(x+w,y+h),(255,255,0),2)
            # rect = cv2.minAreaRect(hull)
            # box = cv2.boxPoints(rect)
            # box = np.int0(box)
            # cv2.cv2.drawContours(cv_image, [box],-1,(0,255,0),2)

        
        cv2.imshow("Image window",cv_image.astype('uint8'))
        cv2.imshow("Grey",opening1.astype('uint8'))
        cv2.waitKey(3)
        target = pokemonImage(rows, cols, x0,y0,x1,y1,pokemon_find)
        try:
            self.image_pub.publish(target)
        except CvBridgeError as e:
            print(e)

def merge_contours(cnts):
    if cnts is None: return
    areas = []
    centers = []
    for cnt in cnts:
        M = cv2.moments(cnt)
        cx = int(M['m10']/M['m00'])
        cy = int(M['m01']/M['m00'])
        areas.append(cv2.contourArea(cnt))
        centers.append((cx,cy))





def main(args):
    rospy.init_node('pokemon_test', anonymous=True)
    ic = image_converter()
    try:
        rospy.spin()
    except KeyboardInterrupt:
        print("Shutting down")
    cv2.destroyAllWindows()


if __name__ == '__main__':
    main(sys.argv)
