#include <rclcpp/rclcpp.hpp>

#include "uvc_cam/uvc_cam.h"
#include <sensor_msgs/msg/image.hpp>
#include <sensor_msgs/image_encodings.hpp>
#include <sensor_msgs/msg/camera_info.hpp>
#include <camera_info_manager/camera_info_manager.hpp>
#include <image_transport/image_transport.hpp>

namespace uvc_camera {

class StereoCamera {
  public:
    StereoCamera(rclcpp::Node::SharedPtr comm_nh, rclcpp::Node::SharedPtr param_nh);
    void onInit();
    void sendInfo(rclcpp::Time time);
    void feedImages();
    ~StereoCamera();

  private:
    rclcpp::Node::SharedPtr node, pnode;
    image_transport::ImageTransport it;
    bool ok;

    uvc_cam::Cam *cam_left, *cam_right;
    int width, height, fps, skip_frames, frames_to_skip;
    std::string left_device, right_device, frame;
    bool rotate_left, rotate_right;

    camera_info_manager::CameraInfoManager left_info_mgr, right_info_mgr;

    image_transport::Publisher left_pub, right_pub;
    ros::Publisher left_info_pub, right_info_pub;

    std::thread image_thread;
};

};

