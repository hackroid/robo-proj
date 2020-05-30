#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/image_encodings.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <string>
#include <geometry_msgs/Twist.h>
#include <math.h>
#include <cstdlib>

using namespace std;
using namespace cv;

static const bool DEBUG = false;
static const bool MOVE = true;
static const bool showBorder = false;
static string search_num;

static const std::string OPENCV_WINDOW = "Pokemon Search";

class ImageConverter
{
    ros::NodeHandle nh_;
    image_transport::ImageTransport it_;
    image_transport::Subscriber image_sub_;
    image_transport::Publisher image_pub_;
    ros::Publisher vel_pub_;
    // string folder_path = "/home/wzl/pokemon_ws/";
    // int flag = 1;
    int flag = 1;
    int roi_tl_x;
    int roi_tl_y;
    int roi_br_x;
    int roi_br_y;
    int roi_height;
    int roi_width;
    int exit = 0;

public:
    ImageConverter()
        : it_(nh_)
    {
        string sub_name = search_num + "/camera/rgb/image_raw";
        // Subscribe to input video feed and publish output video feed
        image_sub_ = it_.subscribe(sub_name, 1,
                                   &ImageConverter::imageCb, this);

        string pub_name = search_num + "/cmd_vel";
        vel_pub_ = nh_.advertise<geometry_msgs::Twist>(pub_name, 1);
        // image_pub_ = it_.advertise("/pokemon_go/searcher", 1);

        cv::namedWindow(OPENCV_WINDOW);
    }

    ~ImageConverter()
    {
        cv::destroyWindow(OPENCV_WINDOW);
    }

    void imageCb(const sensor_msgs::ImageConstPtr &msg)
    {
        cv_bridge::CvImagePtr cv_ptr;
        cv_bridge::CvImagePtr cv_ptr_show;
        try
        {
            cv_ptr_show = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
            cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
        }
        catch (cv_bridge::Exception &e)
        {
            ROS_ERROR("cv_bridge exception: %s", e.what());
            return;
        }

        // Draw an target square on the video stream
        int height = cv_ptr_show->image.rows;
        int width = cv_ptr_show->image.cols;
        cv::rectangle(cv_ptr_show->image, cv::Point(width / 5, height / 20), cv::Point(4 * width / 5, 18 * height / 20), CV_RGB(255, 0, 0));
        // cv::rectangle(cv_ptr_show->image, cv::Point(width/3, height/16), cv::Point(2*width/3, 5*height/8), CV_RGB(255,0,0));
        roi_tl_x = 0;
        roi_tl_y = 0;
        roi_br_x = width;
        roi_br_y = height;
        roi_height = roi_br_y - roi_tl_y;
        roi_width = roi_br_x - roi_tl_x;
        detectRect(cv_ptr);
    }
    void detectRect(cv_bridge::CvImagePtr &cv_ptr)
    {
        // Update GUI Window
        // cv::imshow(OPENCV_WINDOW, cv_ptr_show->image);
        // cv::waitKey(3);

        // Output modified video stream
        // image_pub_.publish(cv_ptr->toImageMsg());hight
        Mat threshold_output;
        vector<vector<Point>> contours;
        vector<Point> contours_point;
        vector<vector<Point>> all_contours;
        vector<vector<Point>> pokemon_contours;
        vector<Point> all_contours_point;
        vector<Vec4i> hierarchy;
        Mat src_gray;
        Mat frame;
        //自定义形态学元素结构hight
        cv::Mat element5(9, 9, CV_8U, cv::Scalar(1)); //5*5正方形，8位uchar型，全1结构元素
        Scalar color = Scalar(0, 255, 0);
        Scalar outColor = Scalar(0, 0, 255);
        cv::Mat closed;
        Rect rect;
        Rect rect1;

        frame = cv_ptr->image;
        Rect roiRect(roi_tl_x, roi_tl_y, roi_width, roi_height);
        Mat roi = frame(roiRect);
        Mat image;
        Mat image2;
        Mat image3;
        if (DEBUG)
        {
            image = roi.clone();
            image2 = roi.clone();
            image3 = roi.clone();
        }

        cvtColor(roi, src_gray, COLOR_BGR2GRAY); //颜色转换
                                                 // imshow("roi", roi);
                                                 //  waitKey();
        Canny(src_gray, threshold_output, 80, 110, (3, 3));
        cv::morphologyEx(threshold_output, closed, cv::MORPH_CLOSE, element5); /* 形态学闭运算函数 */
                                                                               // imshow("roi", roi);

        //   imshow("canny", threshold_output);
        //  waitKey();
        //   imshow("erode", closed);
        // waitKey(3);

        findContours(closed, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_NONE, Point());

        rect = boundingRect(contours[contours.size() - 1]);
        // findContours(closed, pokemon_contours, hierarchy,  RETR_TREE, CHAIN_APPROX_NONE, Point());
        //     rect1 = boundingRect(pokemon_contours[0]);
        // cout << "rect.tl().x: " << rect.tl().10
        //     cv::rectangle(roi, cv::Point(roi_width/5, roi_height/20), cv::Point(4*roi_width/5, 18*roi_height/20), CV_RGB(255,0,0));
        //     	imshow( "Pokemon Search", roi);
        // hight
        // 	cv::waitKey(3);

        if (DEBUG)
        {
            cout << "contours.geometry_msgs/Twist.hsize(): " << contours.size() << endl;
            // // draw all point
            // for (int i = 0; i < contours.size(); i++){
            //   for (int j = 0; j < contours[i].size(); j++){
            //     // cout << contours[i][j] << endl;hight
            //     image.at<Vec3b>(contours[i][j].y, contours[i][j].x) = (0, 0, 255);
            //   }
            // }

            imshow("image", image);

            // show tect in 0th
            rect1 = boundingRect(contours[0]);
            rectangle(image2, rect1, color, 2);
            imshow("pokemonRect", image2);

            rectangle(image3, rect, color, 2);
            imshow("outerRect", image3);

            waitKey();
        }

        // move
        if (MOVE)
        {
            cout << "rect.tl().x: " << rect.tl().x << endl;
            // cout << "roi_tl_x: " << roi_tl_x << endl;
            // cout << "roi_width: " << roi_width << endl;
            // cout << "rect.br().y: " << rect.br().x << endl;
            // cout << "roi_widrectGreen.br().x + rect.tl().x)/2;
            int framMid = roi_width / 2;
            int rectMid = (rect.tl().x + rect.br().x) / 2;
            // if ( (rect.tl().x > width/5) && (rect.tl().y > hight/5) && (rect.br().x < width/5) && (rect.br().y < (roi_height-10)) ){
            if (abs(rectMid - framMid) > 20)
            {
                ROS_INFO("MOVE");
                // Rect rect
                geometry_msgs::Twist speed;
                int dis = 3;
                // // x前后线速度 z左右旋转角速度
                speed.linear.x = 0.0;
                // speed.linear.x=0;
                // 左边空隙大
                // if((rect.tl().x - roi_tl_x) > dis){
                if (rectMid > framMid)
                {
                    if (DEBUG)
                        cout << "in z = 1" << endl;
                    speed.angular.z = -0.08;
                }
                // 右边空隙大
                // else if((rect.tl().x - roi_br_x) < -dis){
                else if (rectMid < framMid)
                {
                    if (DEBUG)
                        cout << "in z = -1" << endl;
                    speed.angular.z = 0.08;
                }
                else
                {
                    if (DEBUG)
                        cout << "in z = 0" << endl;
                    speed.angular.z = 0;
                }
                vel_pub_.publish(speed);
                // ros::Duration(0.1).sleep();

                // show img
                findContours(closed, pokemon_contours, hierarchy, RETR_TREE, CHAIN_APPROX_NONE, Point());
                rect1 = boundingRect(pokemon_contours[0]);
                // rectangle(roi,rect1,color, 1);
                rectangle(roi, rect, outColor, 2);

                Rect rectGreen(rect.tl().x + 26+rand()%3, rect.tl().y + 26+rand()%3, rect.width - 53-rand()%6, rect.height - 53-rand()%6);
                rectangle(roi, rectGreen, color, 2);

                if (showBorder)
                {
                    cv::rectangle(cv_ptr->image, cv::Point(roi_tl_x, roi_tl_y), cv::Point(roi_br_x, roi_br_y), CV_RGB(255, 0, 0));
                }

                imshow("Pokemon Search", frame);
                cv::waitKey(3);
            }
            // // left width more then 5
            // else if(rect.tl().x>5){
            //   ROS_INFO("GO RIGHT");
            //   geometry_msgs::Twist speed;
            //   speed.linear.x=0.02;
            //   speed.angular.z=-0.1;
            //   vel_pub_.publish(speed);
            //   ros::Duration(0.1).sleep();
            // }
            // // right width more then 5
            // else if((roi_width-rect.br().x)>5){
            //   ROS_INFO("GO LEFT");
            //   geometry_msgs::Twist speed;
            //   speed.linear.roi_widthlish(speed);
            //   ros::Duration(0.1).sleep();
            // }
            // stop
            else
            {
                //   ROS_INFO("TURN");
                // if((rect.tl().x>10) && (rect.tl().y>5) && ((roi_height-rect.br().y)>5)){
                //   ROS_INFO("GO RIGHT");
                //   geometry_msgs::Twist speed;
                //   speed.linear.x=0.1;
                //   speed.angular.z=-0.2;
                //   vel_pub_.publish(speed);
                //   ros::Duration(0.1).sleep();
                // }
                // if((rect.tl().y>5) && ((roi_height-rect.br().y)>5) && ((roi_width-rect.br().x)>10)){
                //   ROS_INFO("GO LEFT");
                //   geometry_msgs::Twist speed;
                //   speed.linear.x=0.1;
                //   speed.angular.z=0.2;
                //   vel_pub_.publish(speed);
                //   ros::Duration(0.1).sleep();
                // }
                char *env_val = getenv("CMAKE_PREFIX_PATH");
                char *pathx = strstr(env_val, "/");
                char *pathy = strstr(env_val, "devel");
                int len = pathy - pathx + 1;
                char *fpath = (char *)malloc((len) * sizeof(char));
                memcpy(fpath, pathx, len - 1);
                fpath[len - 1] = '\0';
                string folder_path(fpath);

                ROS_INFO("STOP");
                vel_pub_.publish(geometry_msgs::Twist());
                exit = 1;

                // search and draw pokemon border
                // findContours(closed, pokemon_contours, hierarchy,  RETR_TREE, CHAIN_APPROX_NONE, Point());
                // rect1 = boundingRect(pokemon_contours[0]);
                // int rect1_tl_x;
                // int rect1_tl_y;
                // int rect1_br_x;
                // int rect1_br_y;
                // int diff = 6;
                // if((abs(rect1.tl().x-rect.tl().x)<=2)){
                //   rect1_tl_x = rect1.tl().x + diff;
                // }else{
                //   rect1_tl_x = rect1.tl().x;
                // }
                // if((abs(rect1.tl().y-rect.tl().y)<=2)){
                //   rect1_tl_y = rect1.tl().y + diff;

                // if((abs(rect1.br().x-rect.br().x)<=2)){
                //   rect1_br_x = rect1.br().x - diff;
                // }else{
                //   rect1_br_x = rect1.br().x;
                // }
                // if((abs(rect1.br().y-rect.br().y)<=2)){
                //   rect1_br_y = rect1.br().y - diff;
                // }else{
                //   rect1_br_y = rect1.br().y;
                // }
                // Rect newRect1(rect1_tl_x, rect1_tl_y, rect1_br_x-rect1_tl_x, rect1_br_y-rect1_tl_y);
                // rectangle(roi,newRect1,color, 2);
                // draw picture border
                findContours(closed, pokemon_contours, hierarchy, RETR_TREE, CHAIN_APPROX_NONE, Point());
                rect1 = boundingRect(pokemon_contours[0]);
                // rectangle(roi,rect1,color, 1);
                rectangle(roi, rect, outColor, 2);

                if (showBorder)
                {
                    cv::rectangle(cv_ptr->image, cv::Point(roi_tl_x, roi_tl_y), cv::Point(roi_br_x, roi_br_y), CV_RGB(255, 0, 0));
                    // cv::rectangle(cv_ptr->image, cv::Point(roi_width/5, roi_height/20), cv::Point(4*roi_width/5, 18*roi_height/20), CV_RGB(255,0,0));
                }
                Rect rectGreen(rect.tl().x + 26+rand()%3, rect.tl().y + 26+rand()%3, rect.width - 53-rand()%6, rect.height - 53-rand()%6);
                rectangle(roi, rectGreen, color, 2);
                imshow("Pokemon Search", frame);
                cv::waitKey(3);
                // rectangle(roi,rect, outColor, 2);
                // rectangle(frame, cv::Point(roi_tl_x, roi_tl_y), cv::Point(roi_br_x, roi_br_y), CV_RGB(255,0,0));

                // save img
                vector<cv::String> file_names;
                vector<string> split_string;
                glob(folder_path, file_names);

                int pokemon_img_num = 0;
                for (int i = 0; i < file_names.size(); i++)
                {
                    // ROS_INFO("filename: %s", file_names[i].c_str());
                    int pos = file_names[i].find_last_of("/");
                    string file_name(file_names[i].substr(pos + 1));
                    if (file_name.compare(0, 7, "pokemon") == 0)
                    {
                        pokemon_img_num++;
                    }
                }
                pokemon_img_num++;
                string file_name = folder_path + "pokemon_" + to_string(pokemon_img_num) + ".jpg";
                imwrite(file_name, frame);
                ros::shutdown();

                return;
            }
            exit = 1;
            // image_sub_.shutdown();
        }
    }
};

    int main(int argc, char **argv)
    {
        ros::init(argc, argv, "pokemon_searching");
        // ros::param::get("~search_num",search_num);
        ros::NodeHandle nh("~");
        // std::string check;

        nh.getParam("search_num", search_num);
        cout << search_num << endl;
        ImageConverter ic;
        ros::spin();
        return 0;
    }
