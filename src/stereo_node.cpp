#include <rclcpp/rclcpp.hpp>
#include <nodelet/loader.h>

#include "uvc_camera/stereocamera.h"

int main (int argc, char **argv) {
  rclcpp::init(argc, argv, "uvc_camera_stereo");

  uvc_camera::StereoCamera stereo(rclcpp::Node::SharedPtr(), rclcpp::Node::SharedPtr("~"));

  ros::spin();
  return 0;
}

