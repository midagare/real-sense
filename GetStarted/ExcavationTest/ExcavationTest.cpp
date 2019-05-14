#include <boost/circular_buffer.hpp>
#include <iostream>
#include <opencv2/opencv.hpp>   // Include OpenCV API
#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include <time.h>

#define RS_IR_L   1
#define RS_IR_R   2
#define RS_RGB    3

void save_img(rs2::video_frame &frame, cv::Size &frame_size, int frame_type, int _type, unsigned long num)
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
	unsigned long tn;

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
		tn = clock();
		rs_thread[0] = std::thread(save_img, std::ref(ir_frame_left), std::ref(ir_size), RS_IR_L, CV_16UC1, tn);
		rs_thread[1] = std::thread(save_img, std::ref(ir_frame_right), std::ref(ir_size), RS_IR_R, CV_16UC1, tn);
		rs_thread[2] = std::thread(save_img, std::ref(color_frame), std::ref(rgb_size), RS_RGB, CV_8UC3, tn);

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

	int width2 = 1280;//1920;//1280;
	int height2 = 720;//1080;// 720;
	int fps2 = 15;
	cv::Size rgb_size = cv::Size(width2, height2);

	int nSave = 5;
	// Create a circular buffer
	const int bufsize = 2 * nSave;
	boost::circular_buffer<rs2::frameset> cb(bufsize);
	boost::circular_buffer<rs2::frameset> cb2(bufsize);


	const auto window_name_l = "img_l";
	namedWindow(window_name_l, cv::WINDOW_AUTOSIZE);

	const auto window_name_r = "img_r";
	namedWindow(window_name_r, cv::WINDOW_AUTOSIZE);


	const auto window_name_c = "color";
	namedWindow(window_name_c, cv::WINDOW_AUTOSIZE);

	//Create a configuration for configuring the pipeline with a non default profile
	rs2::config cfg;
	rs2::config cfg2;

	// Declare depth colorizer for pretty visualization of depth data
	rs2::colorizer color_map;
	color_map.set_option(RS2_OPTION_COLOR_SCHEME, 2.f);

	//Add desired streams to configuration
	cfg.enable_stream(RS2_STREAM_INFRARED, 1, width1, height1, RS2_FORMAT_Y16, fps1);
	cfg.enable_stream(RS2_STREAM_INFRARED, 2, width1, height1, RS2_FORMAT_Y16, fps1);
	cfg.enable_stream(RS2_STREAM_COLOR, width2, height2, RS2_FORMAT_BGR8, fps2);

	cfg2.enable_stream(RS2_STREAM_INFRARED, 1, width2, height2, RS2_FORMAT_Y8, fps2);
	cfg2.enable_stream(RS2_STREAM_INFRARED, 2, width2, height2, RS2_FORMAT_Y8, fps2);
	cfg2.enable_stream(RS2_STREAM_DEPTH, width2, height2, RS2_FORMAT_Z16, fps2);

	// Declare RealSense pipeline, encapsulating the actual device and sensors
	rs2::pipeline pipe;

	//Instruct pipeline to start streaming with the requested configuration
	rs2::pipeline_profile pipe_profile = pipe.start(cfg);
	auto d_sensor = pipe_profile.get_device().first<rs2::depth_sensor>();
	d_sensor.set_option(RS2_OPTION_EMITTER_ENABLED, 0.f);
	d_sensor.set_option(RS2_OPTION_DEPTH_UNITS, 0.0001f);

	t = clock();
	std::cout << "config:" << (t - t0) << std::endl;
	t0 = t;
	// Capture 30 frames to give autoexposure, etc. a chance to settle
	//for (auto i = 0; i < 30; ++i) pipe.wait_for_frames();
	//t = clock();
	//std::cout << "30 frames: " << (t - t0) << std::endl;
	//t0 = t;

	while (1)
	{
		// wait for frames and get frameset
		rs2::frameset frameset = pipe.wait_for_frames();

		// get left and right infrared frames from frameset
		rs2::video_frame ir_frame_left = frameset.get_infrared_frame(1);
		rs2::video_frame ir_frame_right = frameset.get_infrared_frame(2);
		rs2::video_frame color_frame = frameset.get_color_frame();

		rs2::frame depth;

		cv::Mat dMat_left = cv::Mat(ir_size, CV_16UC1, (void*)ir_frame_left.get_data());
		cv::Mat dMat_right = cv::Mat(ir_size, CV_16UC1, (void*)ir_frame_right.get_data());
		cv::Mat dMat_color = cv::Mat(rgb_size, CV_8UC3, (void*)color_frame.get_data(), cv::Mat::AUTO_STEP);
		cv::Mat dMat_depth;

		cv::imshow(window_name_l, dMat_left);
		cv::imshow(window_name_r, dMat_right);
		cv::imshow(window_name_c, dMat_color);
		char c = cv::waitKey(1);
		if (c == 's')
		{
			for (int k = 0; k < 2; k++)
			{
				for (auto i = 0; i < 30; ++i) pipe.wait_for_frames();
				t = clock();
				std::cout << "init to save: " << (t - t0) << std::endl;
				cb.push_back(pipe.wait_for_frames());
				std::thread cb_thread = std::thread(save_frameset, std::ref(cb), std::ref(ir_size), std::ref(rgb_size));
				int t1 = t;
				for (int i = 0; i < nSave-1; i++) {
					//std::this_thread::sleep_for(std::chrono::milliseconds(70));
					cb.push_back(pipe.wait_for_frames());

					t = clock();
					std::cout << "cb " << i << ": " << (t - t1) << std::endl;
					t1 = t;
				}
				cb_thread.join();
				t = clock();
				std::cout << "finish1 " << ": " << (t - t1) << std::endl;

				//Second capture
				pipe.stop();
				
				pipe_profile = pipe.start(cfg2);

				for (auto i = 0; i < 30; ++i) pipe.wait_for_frames();
				t1 = t;
				t = clock();
				std::cout << "init to save2: " << (t - t1) << std::endl;

				for (int i = 0; i < nSave; i++) {
					depth = pipe.wait_for_frames().get_depth_frame();//; .apply_filter(color_map);
					//std::cout << "depth...";
					// Create OpenCV matrix of size (w,h) from the colorized depth data
					dMat_depth = cv::Mat(cv::Size(width2, height2), CV_16UC1, (void*)depth.get_data());

					//std::cout << "mat...";
					cv::imwrite("RS_D/" + std::to_string(clock()) + ".png", dMat_depth);
					//std::cout << "saving...";
				}
				t = clock();
				std::cout << "finish2 " << ": " << (t - t1) << std::endl;

				pipe.stop();
				pipe_profile = pipe.start(cfg);

				if (k == 0) {
					d_sensor = pipe_profile.get_device().first<rs2::depth_sensor>();
					d_sensor.set_option(RS2_OPTION_EMITTER_ENABLED, 1.f);
					std::cout << "units: " << d_sensor.get_depth_scale() << std::endl;
					std::cout << "emitter on: " << std::endl;
				}
				else
				{
					d_sensor = pipe_profile.get_device().first<rs2::depth_sensor>();
					d_sensor.set_option(RS2_OPTION_EMITTER_ENABLED, 0.f);
					std::cout << "emitter off: " << std::endl;
				}
			}
		}
		else if (c == 'q') {
			pipe.stop();
			break;
		}
	}
	//system("pause");
	return 0;
}