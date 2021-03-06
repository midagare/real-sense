// RealSenseGetStarted.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <opencv2/opencv.hpp>   // Include OpenCV API
#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include <iostream>

int main()
{
	// Declare depth colorizer for pretty visualization of depth data
	rs2::colorizer color_map;
	color_map.set_option(RS2_OPTION_COLOR_SCHEME, 2.f);
	
	rs2::config cfg;
	cfg.enable_stream(RS2_STREAM_DEPTH, 1280, 720, RS2_FORMAT_Z16, 15);
	// Declare RealSense pipeline, encapsulating the actual device and sensors
	rs2::pipeline pipe;
	// Start streaming with default recommended configuration
	pipe.start(cfg);
	using namespace cv;
	const auto window_name = "Display Image";
	namedWindow(window_name, WINDOW_NORMAL);

	rs2::frameset data;
	rs2::frame depth;
	int w;
	int h;

	while (waitKey(1) < 0 && getWindowProperty(window_name, WND_PROP_AUTOSIZE) >= 0)
	{
		data = pipe.wait_for_frames(); // Wait for next set of frames from the camera
		depth = data.get_depth_frame();//.apply_filter(color_map);

		// Query frame size (width and height)
		w = depth.as<rs2::video_frame>().get_width();
		h = depth.as<rs2::video_frame>().get_height();

		// Create OpenCV matrix of size (w,h) from the colorized depth data
		Mat image(Size(w, h), CV_16UC1, (void*)depth.get_data(), cv::Mat::AUTO_STEP);

		// Update the window with new data
		imshow(window_name, image);
		char c = cv::waitKey(1);
		if (c == 's') {
			cv::imwrite("test.png", image);
			break;
		}
	}
	cv::waitKey(0);
	return 0;
}