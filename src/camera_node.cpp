#include <rclcpp/rclcpp.hpp>
#include <nodelet/loader.h>

#include "uvc_camera/camera.h"

int main (int argc, char **argv) {
  rclcpp::init(argc, argv, "uvc_camera");

  uvc_camera::Camera camera(rclcpp::Node::SharedPtr(), rclcpp::Node::SharedPtr("~"));

  ros::spin();
  return 0;
}

