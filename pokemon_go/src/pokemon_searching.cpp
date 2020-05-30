#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/image_encodings.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <string>


using namespace std;

static const std::string OPENCV_WINDOW = "Pokemon Search";

class ImageConverter
{
  ros::NodeHandle nh_;
  image_transport::ImageTransport it_;
  image_transport::Subscriber image_sub_;
  image_transport::Publisher image_pub_;
  // string folder_path = "/home/wzl/pokemon_ws/";
  // int flag = 1;
public:
  ImageConverter()
    : it_(nh_)
  {
    // Subscribe to input video feed and publish output video feed
    image_sub_ = it_.subscribe("/robot1/camera/rgb/image_raw", 1,
      &ImageConverter::imageCb, this);
    image_pub_ = it_.advertise("/pokemon_go/searcher", 1);

    cv::namedWindow(OPENCV_WINDOW);
  }

  ~ImageConverter()
  {
    cv::destroyWindow(OPENCV_WINDOW);
  }

  void imageCb(const sensor_msgs::ImageConstPtr& msg)
  {
    cv_bridge::CvImagePtr cv_ptr;
    cv_bridge::CvImagePtr cv_ptr_show;
    try
    {
      cv_ptr_show = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
      cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
    }
    catch (cv_bridge::Exception& e)
    {
      ROS_ERROR("cv_bridge exception: %s", e.what());
      return;
    }
    // if(flag==1){
    //   vector<cv::String> file_names;
    //   vector<string> split_string;
    //   glob(folder_path, file_names);
    //   int pokemon_img_num = 0;
    //   for (int i = 0; i < file_names.size(); i++) {
        
    //     ROS_INFO("filename: %s", file_names[i].c_str());
    //     int pos = file_names[i].find_last_of("/");
    //     string file_name(file_names[i].substr(pos + 1));
    //     cout << file_name << endl;
    //     if(file_name.compare(0, 7, "pokemon")){
    //       pokemon_img_num++;
    //     }
    //   }
    //   cout << "pokemon num: " << pokemon_img_num << endl;
    //   flag = 2;
    // }
    // Draw an target square on the video stream
    int height = cv_ptr_show->image.rows;
    int width = cv_ptr_show->image.cols;
    cv::rectangle(cv_ptr_show->image, cv::Point(width/3, height/16), cv::Point(2*width/3, 5*height/8), CV_RGB(255,0,0));

    // Update GUI Window
    cv::imshow(OPENCV_WINDOW, cv_ptr_show->image);
    cv::waitKey(3);

    // Output modified video stream
    image_pub_.publish(cv_ptr->toImageMsg());
  }
};

int main(int argc, char** argv)
{
  ros::init(argc, argv, "pokemon_searching");
  ImageConverter ic;
  ros::spin();
  return 0;
}
