#include <boost/circular_buffer.hpp>
#include <iostream>
#include <opencv2/opencv.hpp>   // Include OpenCV API
#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include <time.h>

#define RS_IR_L   1
#define RS_IR_R   2
#define RS_RGB    3

void save_img(rs2::video_frame &frame, cv::Size &frame_size, int frame_type, int _type, int num)
{
	cv::Mat mat;
	if (frame_type == RS_RGB)
		mat = cv::Mat(frame_size, _type, (void*)frame.get_data(), cv::Mat::AUTO_STEP);
	else
		mat = cv::Mat(frame_size, _type, (void*)frame.get_data());

	switch (frame_type)
	{
	case RS_IR_L:
		cv::imwrite("RS_IR_L/" + std::to_string(num) + ".png", mat);
		break;
	case RS_IR_R:
		cv::imwrite("RS_IR_R/" + std::to_string(num) + ".png", mat);
		break;
	case RS_RGB:
		cv::imwrite("RS_RGB/" + std::to_string(num) + ".png", mat);
		break;
	}
}

void save_frameset(boost::circular_buffer<rs2::frameset> &cb, cv::Size &ir_size, cv::Size &rgb_size)
{
	int i = 0;
	int maxFramesSaved = 200;
	std::thread rs_thread[3];
	rs2::frameset frameset;

	rs2::video_frame ir_frame_left = frameset.get_infrared_frame(1);
	rs2::video_frame ir_frame_right = frameset.get_infrared_frame(2);
	rs2::video_frame color_frame = frameset.get_color_frame();

	std::cout << cb.size() << " : " << std::endl;
	while (!cb.empty() && ++i < maxFramesSaved)
	{
		std::cout << cb.size() << " : " << std::endl;
		rs2::frameset frameset = cb[0];
		
		ir_frame_left = frameset.get_infrared_frame(1);
		ir_frame_right = frameset.get_infrared_frame(2);
		color_frame = frameset.get_color_frame();

		rs_thread[0] = std::thread(save_img, std::ref(ir_frame_left), std::ref(ir_size), RS_IR_L, CV_16UC1, i);
		rs_thread[1] = std::thread(save_img, std::ref(ir_frame_right), std::ref(ir_size), RS_IR_R, CV_16UC1, i);
		rs_thread[2] = std::thread(save_img, std::ref(color_frame), std::ref(rgb_size), RS_RGB, CV_8UC3, i);

		rs_thread[0].join();
		rs_thread[1].join();
		rs_thread[2].join();

		cb.pop_front();
	}
}

int main()
{
	unsigned long t0 = clock();
	unsigned long t;
	int width1 = 1280;
	int height1 = 800;
	int fps1 = 15;
	cv::Size ir_size = cv::Size(width1, height1);

	int width2 = 1920;//1280;
	int height2 = 1080;// 720;
	int fps2 = 15;
	cv::Size rgb_size = cv::Size(width2, height2);

	int nSave = 100;
	// Create a circular buffer
	const int bufsize = 2 * nSave;
	boost::circular_buffer<rs2::frameset> cb(bufsize);


	const auto window_name_l = "img_l";
	namedWindow(window_name_l, cv::WINDOW_AUTOSIZE);

	const auto window_name_r = "img_r";
	namedWindow(window_name_r, cv::WINDOW_AUTOSIZE);


	const auto window_name_c = "color";
	namedWindow(window_name_c, cv::WINDOW_AUTOSIZE);

	//Create a configuration for configuring the pipeline with a non default profile
	rs2::config cfg;

	//Add desired streams to configuration
	cfg.enable_stream(RS2_STREAM_INFRARED, 1, width1, height1, RS2_FORMAT_Y16, fps1);
	cfg.enable_stream(RS2_STREAM_INFRARED, 2, width1, height1, RS2_FORMAT_Y16, fps1);
	cfg.enable_stream(RS2_STREAM_COLOR, width2, height2, RS2_FORMAT_BGR8, fps2);

	// Declare RealSense pipeline, encapsulating the actual device and sensors
	rs2::pipeline pipe;

	//Instruct pipeline to start streaming with the requested configuration
	pipe.start(cfg);

	t = clock();
	std::cout << "config:" << (t - t0) << std::endl;
	t0 = t;
	// Capture 30 frames to give autoexposure, etc. a chance to settle
	for (auto i = 0; i < 30; ++i) pipe.wait_for_frames();
	t = clock();
	std::cout << "30 frames: " << (t - t0) << std::endl;
	t0 = t;

	while (1)
	{
		// wait for frames and get frameset
		rs2::frameset frameset = pipe.wait_for_frames();

		// get left and right infrared frames from frameset
		rs2::video_frame ir_frame_left = frameset.get_infrared_frame(1);
		rs2::video_frame ir_frame_right = frameset.get_infrared_frame(2);
		rs2::video_frame color_frame = frameset.get_color_frame();

		cv::Mat dMat_left = cv::Mat(ir_size, CV_16UC1, (void*)ir_frame_left.get_data());
		cv::Mat dMat_right = cv::Mat(ir_size, CV_16UC1, (void*)ir_frame_right.get_data());
		cv::Mat dMat_color = cv::Mat(rgb_size, CV_8UC3, (void*)color_frame.get_data(), cv::Mat::AUTO_STEP);

		cv::imshow(window_name_l, dMat_left);
		cv::imshow(window_name_r, dMat_right);
		cv::imshow(window_name_c, dMat_color);
		char c = cv::waitKey(1);
		if (c == 's')
		{
			t = clock();
			std::cout << "init to save: " << (t - t0) << std::endl;
			cb.push_back(pipe.wait_for_frames());
			std::thread cb_thread = std::thread(save_frameset, std::ref(cb), std::ref(ir_size), std::ref(rgb_size));
			int t1 = t;
			for (int i = 0; i < nSave; i++) {
				//std::this_thread::sleep_for(std::chrono::milliseconds(70));
				cb.push_back(pipe.wait_for_frames());

				t = clock();
				std::cout << "cb " << i << ": " << (t - t1) << std::endl;
				t1 = t;
			}
			cb_thread.join();
			t = clock();
			std::cout << "finish " << ": " << (t - t1) << std::endl;
			break;
		}
		else if (c == 'q')
			break;
	}
	system("pause");
	return 0;
}