#include <rclcpp/rclcpp.hpp>

#include "uvc_cam/uvc_cam.h"
#include <sensor_msgs/msg/image.hpp>
#include <sensor_msgs/image_encodings.hpp>
#include <sensor_msgs/msg/camera_info.hpp>
#include <camera_info_manager/camera_info_manager.hpp>
#include <image_transport/image_transport.hpp>

#include "uvc_camera/stereocamera.h"

using namespace sensor_msgs;

/* Rotate an 8-bit, 3-channel image by 180 degrees. */
static inline void rotate(unsigned char *dst_chr, unsigned char *src_chr, int pixels) {
  struct pixel_t {
    unsigned char r, g, b;
  };

  struct pixel_t *src = (pixel_t *) src_chr;
  struct pixel_t *dst = &(((pixel_t *) dst_chr)[pixels - 1]);

  for (int i = pixels; i != 0; --i) {
    *dst = *src;
    src++;
    dst--;
  }
}

namespace uvc_camera {

StereoCamera::StereoCamera(rclcpp::Node::SharedPtr comm_nh, rclcpp::Node::SharedPtr param_nh) :
  node(comm_nh), pnode(param_nh), it(comm_nh),
  left_info_mgr(rclcpp::Node::SharedPtr(comm_nh, "left"), "left_camera"),
  right_info_mgr(rclcpp::Node::SharedPtr(comm_nh, "right"), "right_camera") {

  /* default config values */
  width = 640;
  height = 480;
  fps = 10;
  skip_frames = 0;
  frames_to_skip = 0;
  left_device = "/dev/video0";
  right_device = "/dev/video1";
  frame = "camera";
  rotate_left = false;
  rotate_right = false;

  /* set up information managers */
  std::string left_url, right_url;

  pnode.getParam("left/camera_info_url", left_url);
  pnode.getParam("right/camera_info_url", right_url);

  left_info_mgr.loadCameraInfo(left_url);
  right_info_mgr.loadCameraInfo(right_url);

  /* pull other configuration */
  pnode.getParam("left/device", left_device);
  pnode.getParam("right/device", right_device);

  pnode.getParam("fps", fps);
  pnode.getParam("skip_frames", skip_frames);

  pnode.getParam("left/rotate", rotate_left);
  pnode.getParam("right/rotate", rotate_right);

  pnode.getParam("width", width);
  pnode.getParam("height", height);

  pnode.getParam("frame_id", frame);

  /* advertise image streams and info streams */
  left_pub = it.advertise("left/image_raw", 1);
  right_pub = it.advertise("right/image_raw", 1);

  left_info_pub = node.advertise<CameraInfo>("left/camera_info", 1);
  right_info_pub = node.advertise<CameraInfo>("right/camera_info", 1);

  /* initialize the cameras */
  cam_left =
      new uvc_cam::Cam(left_device.c_str(), uvc_cam::Cam::MODE_RGB,
		       width, height, fps);
  cam_left->set_motion_thresholds(100, -1);
  cam_right =
      new uvc_cam::Cam(right_device.c_str(), uvc_cam::Cam::MODE_RGB,
		       width, height, fps);
  cam_right->set_motion_thresholds(100, -1);

  /* and turn on the streamer */
  ok = true;
  image_thread = std::thread(std::bind(&StereoCamera::feedImages, this));
}

void StereoCamera::sendInfo(rclcpp::Time time) {
  CameraInfoPtr info_left(new CameraInfo(left_info_mgr.getCameraInfo()));
  CameraInfoPtr info_right(new CameraInfo(right_info_mgr.getCameraInfo()));

  info_left->header.stamp = time;
  info_right->header.stamp = time;
  info_left->header.frame_id = frame;
  info_right->header.frame_id = frame;

  left_info_pub.publish(info_left);
  right_info_pub.publish(info_right);
}


void StereoCamera::feedImages() {
  unsigned int pair_id = 0;
  while (ok) {
    unsigned char *frame_left = NULL, *frame_right = NULL;
    uint32_t bytes_used_left, bytes_used_right;

    rclcpp::Time capture_time = rclcpp::Time::now();

    int left_idx = cam_left->grab(&frame_left, bytes_used_left);
    int right_idx = cam_right->grab(&frame_right, bytes_used_right);

    /* Read in every frame the camera generates, but only send each
     * (skip_frames + 1)th frame. This reduces effective frame
     * rate, processing time and network usage while keeping the
     * images synchronized within the true framerate.
     */
    if (skip_frames == 0 || frames_to_skip == 0) {
      if (frame_left && frame_right) {
	ImagePtr image_left(new Image);
	ImagePtr image_right(new Image);

	image_left->height = height;
	image_left->width = width;
	image_left->step = 3 * width;
	image_left->encoding = image_encodings::RGB8;
	image_left->header.stamp = capture_time;
	image_left->header.seq = pair_id;

	image_right->height = height;
	image_right->width = width;
	image_right->step = 3 * width;
	image_right->encoding = image_encodings::RGB8;
	image_right->header.stamp = capture_time;
	image_right->header.seq = pair_id;

	image_left->header.frame_id = frame;
	image_right->header.frame_id = frame;

	image_left->data.resize(image_left->step * image_left->height);
	image_right->data.resize(image_right->step * image_right->height);

	if (rotate_left)
	  rotate(&image_left->data[0], frame_left, width * height);
	else
	  memcpy(&image_left->data[0], frame_left, width * height * 3);

	if (rotate_right)
	  rotate(&image_right->data[0], frame_right, width * height);
	else
	  memcpy(&image_right->data[0], frame_right, width * height * 3);

	left_pub.publish(image_left);
	right_pub.publish(image_right);

	sendInfo(capture_time);

	++pair_id;
      }

      frames_to_skip = skip_frames;
    } else {
      frames_to_skip--;
    }

    if (frame_left)
      cam_left->release(left_idx);
    if (frame_right)
      cam_right->release(right_idx);
  }
}

StereoCamera::~StereoCamera() {
  ok = false;
  image_thread.join();
  if (cam_left)
    delete cam_left;
  if (cam_right)
    delete cam_right;
}

};
