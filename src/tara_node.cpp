#include <rclcpp/rclcpp.hpp>

#include "uvc_camera/tara_ros.h"

int main (int argc, char **argv)
{
	rclcpp::init(argc, argv);
	
	auto node = rclcpp::Node::make_shared("tara_ros");

	uvc_camera::taraCamera camera(node);
	if ( camera.isCameraStereo == false )
	{
		rclcpp::shutdown();
	}
	else
	{  
		rclcpp::spin(node);
	}
	return 0;
}

