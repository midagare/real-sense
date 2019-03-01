#include <iostream>
#include <thread>
#include <sstream>
#include <opencv2/opencv.hpp>   // Include OpenCV API
#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include <time.h>
#include "m_rs.hpp"

//Hello world code. Needs #include <iostream>
void helloWorld() {
	std::cout << "Hello, World!" << std::endl;
	system("pause");
}

//Open CV code. Needs #include <opencv2/opencv.hpp>
void opencv_example() {
	cv::Mat img = cv::imread("D:/Miguel/Documentos/01-Git/github/intelVisualStudio/ToolProject/ToolProject/0-1.jpg");
	cv::namedWindow("image", cv::WINDOW_NORMAL);
	cv::imshow("image", img);
	cv::waitKey(0);
}

//Intel RealSense example with OpenCV. Needs the folowing libraries
// #include <opencv2/opencv.hpp>   // Include OpenCV API
// #include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
void opencv_realsense() {
	// Declare depth colorizer for pretty visualization of depth data
	rs2::colorizer color_map;
	// Declare RealSense pipeline, encapsulating the actual device and sensors
	rs2::pipeline pipe;
	// Start streaming with default recommended configuration
	pipe.start();
	using namespace cv;
	const auto window_name = "Display Image";
	namedWindow(window_name, WINDOW_AUTOSIZE);

	while (waitKey(1) < 0 && getWindowProperty(window_name, WND_PROP_AUTOSIZE) >= 0)
	{
		rs2::frameset data = pipe.wait_for_frames(); // Wait for next set of frames from the camera
		rs2::frame depth = data.get_depth_frame().apply_filter(color_map);

		// Query frame size (width and height)
		const int w = depth.as<rs2::video_frame>().get_width();
		const int h = depth.as<rs2::video_frame>().get_height();

		// Create OpenCV matrix of size (w,h) from the colorized depth data
		Mat image(Size(w, h), CV_8UC3, (void*)depth.get_data(), Mat::AUTO_STEP);

		// Update the window with new data
		imshow(window_name, image);
	}
}

int opencv_multicamera() {
	unsigned long t0 = clock();
	unsigned long t;
	int width1 = 1280;
	int height1 = 800;
	int fps1 = 15;
	
	int width2 = 1280;
	int height2 = 720;
	int fps2 = 15;


	const auto window_name_l = "img_l";
	namedWindow(window_name_l, cv::WINDOW_AUTOSIZE);

	const auto window_name_r = "img_r";
	namedWindow(window_name_r, cv::WINDOW_AUTOSIZE);

	const auto window_name_c = "color";
	namedWindow(window_name_c, cv::WINDOW_AUTOSIZE);

	//Create a configuration for configuring the pipeline with a non default profile
	rs2::config cfg;

	//Add desired streams to configuration
	cfg.enable_stream(RS2_STREAM_INFRARED, 1, width1, height1, RS2_FORMAT_Y8, fps1);
	cfg.enable_stream(RS2_STREAM_INFRARED, 2, width1, height1, RS2_FORMAT_Y8, fps1);
	cfg.enable_stream(RS2_STREAM_COLOR, width2, height2, RS2_FORMAT_BGR8, fps2);
	//cfg.enable_stream(RS2_STREAM_DEPTH, width, height, RS2_FORMAT_Z16, fps);

	// Declare RealSense pipeline, encapsulating the actual device and sensors
	rs2::pipeline pipe;

	//Instruct pipeline to start streaming with the requested configuration
	pipe.start(cfg);

	t = clock();
	std::cout << (t - t0) << std::endl;
	t0 = t;
	// Capture 30 frames to give autoexposure, etc. a chance to settle
	for (auto i = 0; i < 30; ++i) pipe.wait_for_frames();
	t = clock();
	std::cout << "30 frames: " << (t - t0) << std::endl;
	t0 = t;

	while (1) // Application still alive?
	{
		// wait for frames and get frameset
		rs2::frameset frameset = pipe.wait_for_frames();
		// get single infrared frame from frameset
		//rs2::video_frame ir_frame = frameset.get_infrared_frame();

		// get left and right infrared frames from frameset
		rs2::video_frame ir_frame_left = frameset.get_infrared_frame(1);
		rs2::video_frame ir_frame_right = frameset.get_infrared_frame(2);

		//rs2::video_frame depth_frame = frameset.get_color_frame().apply_filter(color_map);
		//rs2::frame depth = data.get_depth_frame().apply_filter(color_map);
		//rs2::video_frame depth_frame = color_map(frameset.get_depth_frame());
		//rs2::video_frame depth_frame = frameset.get_depth_frame().apply_filter(color_map); //work
		rs2::video_frame color_frame = frameset.get_color_frame();

		cv::Mat dMat_left = cv::Mat(cv::Size(width1, height1), CV_8UC1, (void*)ir_frame_left.get_data());
		cv::Mat dMat_right = cv::Mat(cv::Size(width1, height1), CV_8UC1, (void*)ir_frame_right.get_data());
		//cv::Mat dMat_depth = cv::Mat(cv::Size(width, height), CV_8UC3, (void*)depth_frame.get_data());
		cv::Mat dMat_color = cv::Mat(cv::Size(width2, height2), CV_8UC3, (void*)color_frame.get_data(), cv::Mat::AUTO_STEP);

		cv::imshow(window_name_l, dMat_left);
		cv::imshow(window_name_r, dMat_right);
		cv::imshow(window_name_c, dMat_color);
		char c = cv::waitKey(1);
		if (c == 's')
		{
			t = clock();
			std::cout << "init to save: " << (t - t0) << std::endl;
			t0 = t;
			for (int i = 0; i < 10; i++) {
				frameset = pipe.wait_for_frames();
				ir_frame_left = frameset.get_infrared_frame(1);
				ir_frame_right = frameset.get_infrared_frame(2);
				color_frame = frameset.get_color_frame();

				dMat_left = cv::Mat(cv::Size(width1, height1), CV_8UC1, (void*)ir_frame_left.get_data());
				dMat_right = cv::Mat(cv::Size(width1, height1), CV_8UC1, (void*)ir_frame_right.get_data());
				dMat_color = cv::Mat(cv::Size(width2, height2), CV_8UC3, (void*)color_frame.get_data(), cv::Mat::AUTO_STEP);
				cv::imwrite(std::to_string(i) + "-1.jpg", dMat_left);
				cv::imwrite(std::to_string(i) + "-2.jpg", dMat_right);
				////cv::imwrite(std::to_string(i) + "-3.jpg", dMat_depth);
				t = clock();
				std::cout << "save " << i << ": " << (t - t0) << std::endl;
				t0 = t;
			}
			break;
		}
		else if (c == 'q')
			break;
	}
	return EXIT_SUCCESS;
}

int main()
{
	//helloWorld();
	//opencv_example();
	//opencv_realsense();
	opencv_multicamera();
	//std::cout << m_rs::sum(4, 5) << std::endl;
	system("pause");
	return 0;
}