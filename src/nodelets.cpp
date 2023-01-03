#include <rclcpp/rclcpp.hpp>

#include <pluginlib/class_list_macros.hpp>
#include <nodelet/nodelet.h>

#include "uvc_camera/camera.h"
#include "uvc_camera/stereocamera.h"

namespace uvc_camera {

class CameraNodelet : public nodelet::Nodelet {
  public:
    CameraNodelet() {}

    void onInit() {
      rclcpp::Node::SharedPtr node = getNodeHandle();
      rclcpp::Node::SharedPtr pnode = getPrivateNodeHandle();

      camera = new Camera(node, pnode);
    }

    ~CameraNodelet() {
      if (camera) delete camera;
    }

  private:
    Camera *camera;
};

class StereoNodelet : public nodelet::Nodelet {
  public:
    StereoNodelet() {}

    void onInit() {
      rclcpp::Node::SharedPtr node = getNodeHandle();
      rclcpp::Node::SharedPtr pnode = getPrivateNodeHandle();

      stereo = new StereoCamera(node, pnode);
    }

    ~StereoNodelet() {
      if (stereo) delete stereo;
    }

  private:
    StereoCamera *stereo;
};

};

PLUGINLIB_EXPORT_CLASS(uvc_camera::CameraNodelet, nodelet::Nodelet);
PLUGINLIB_EXPORT_CLASS(uvc_camera::StereoNodelet, nodelet::Nodelet);

