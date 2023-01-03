#include <rclcpp/rclcpp.hpp>
#include <image_transport/image_transport.hpp>
#include "uvc_cam/uvc_cam.h"
#include <camera_info_manager/camera_info_manager.hpp>
#include <builtin_interfaces/msg/time.hpp>
#include <std_msgs/msg/float64.hpp>
#include <libv4l2.h>
#include <camera_calibration_parsers/parse_yml.hpp>
#include <fstream>
#include <yaml-cpp/yaml.h>
#include <geometry_msgs/msg/point.hpp>
#include <std_msgs/msg/bool.hpp>
#include <sensor_msgs/msg/imu.hpp>

using namespace std;

#define		M_PI				3.14159265358979323846
#define		HALF_PI				(M_PI / 2)
#define		DEG2RAD				(M_PI / 180.f)
#define		RAD2DEG				(180.f / M_PI)

//	1.2.131.652 is the last firmware version of Tara that doesn't support auto exposure.
#define 		MajorVersion_t		1
#define 		MinorVersion1_t	2
#define 		MinorVersion2_t	131
#define 		MinorVersion3_t	652

#define sampleFreq     119.0f                                   // sample frequency in Hz
#define gyroMeasError  0.1                                      // gyroscope measurement error in rad/s
#define betaDef        sqrt(3.0f / 4.0f) * gyroMeasError        // compute beta

namespace uvc_camera {

	void Sleep(unsigned int TimeInMilli);

	class taraCamera {
		public:

			IMUCONFIG_TypeDef lIMUConfig;
			IMUDATAINPUT_TypeDef lIMUInput;
			IMUDATAOUTPUT_TypeDef *lIMUOutput;
			bool isCameraStereo;

			taraCamera(rclcpp::Node::SharedPtr comm_nh);
			void onInit();
			void sendInfoLeft(sensor_msgs::msg::Image::SharedPtr &image, rclcpp::Time time);
			void sendInfoRight(sensor_msgs::msg::Image::SharedPtr &image, rclcpp::Time time);
			void feedImages();
			~taraCamera();
			void timeCb(builtin_interfaces::msg::Time time);
			BOOL LoadCameraMatrix();
			//IMU
			void getInclination(double w_x, double w_y, double w_z, double a_x, double a_y, double a_z);
			// getOrientation return the orientation in quaternion format
			void getOrientation(double w_x, double w_y, double w_z, double a_x, double a_y, double a_z);
			int returnValue;

		private:
			rclcpp::Node::SharedPtr node;
			image_transport::ImageTransport it;
			bool ok;

			int width, height, fps, skip_frames, frames_to_skip;
			std::string device, frame;
			std::string frameLeft;
			std::string frameRight;
			int  exposure_value;
			int  brightness_value;
			bool rotate;

			camera_info_manager::CameraInfoManager info_mgr_left;
			camera_info_manager::CameraInfoManager info_mgr_right;

			image_transport::Publisher pub, pub_left, pub_right, pub_concat;
			rclcpp::Publisher<camera_info_manager::CameraInfo>::SharedPtr info_pub;
			rclcpp::Publisher<camera_info_manager::CameraInfo>::SharedPtr info_pub_left;
			rclcpp::Publisher<camera_info_manager::CameraInfo>::SharedPtr info_pub_right;    
			rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr exposure_pub;
			rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr brightness_pub;
			rclcpp::Publisher<geometry_msgs::msg::Point>::SharedPtr IMU_inclination_pub;
			rclcpp::Publisher<sensor_msgs::msg::Imu>::SharedPtr IMU_pub;

			rclcpp::Subscription<builtin_interfaces::msg::Time>::SharedPtr time_sub;
			rclcpp::Subscription<std_msgs::msg::Float64>::SharedPtr exposure_sub;
			rclcpp::Subscription<std_msgs::msg::Float64>::SharedPtr brightness_sub;

			rclcpp::Clock clock;
			rclcpp::Time last_time;
			std::mutex time_mutex_;

			uvc_cam::Cam *cam;
			std::thread image_thread;
			std::thread IMU_thread;
			volatile float beta;	// 2 * proportional gain (Kp)
			volatile float q0, q1, q2, q3;	// quaternion of sensor frame relative to auxiliary frame

			double angleX, angleY, angleZ; // Rotational angle for cube [NEW]
			double RwEst[3];

			double squared(double x);
			double glIMU_Interval;

			void callBackExposure(std_msgs::msg::Float64 call_exposure_value);
			void callBackBrightness(std_msgs::msg::Float64 call_brightness_value);
			void SetIMUConfigDefaultEnable();
			void IMU_enable();    
			int econ_strcmp (const char * str1, const char *str2);
			/*  Returns the interval time for sampling the values of the IMU. */
			double GetIMUIntervalTime(IMUCONFIG_TypeDef	lIMUConfig);
			BOOL DisableIMU();
			BOOL checkFirmware (UINT8 MajorVersion, UINT8 MinorVersion1, UINT16 MinorVersion2, UINT16 MinorVersion3);		//Returns 1 if firmware supports auto exposure, else 0;
			float invSqrt(float x);

	};

};

