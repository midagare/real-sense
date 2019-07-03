#include <iostream>
#include <opencv2/opencv.hpp>   // Include OpenCV API
#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API

#define RS_IR_L   1
#define RS_IR_R   2
#define RS_RGB    3

void save_img(rs2::video_frame &frame, cv::Size &frame_size, int frame_type, int _type, std::string datetime, std::string sensor, int i)
{
	cv::Mat mat;
	if (frame_type == RS_RGB)
		mat = cv::Mat(frame_size, _type, (void*)frame.get_data(), cv::Mat::AUTO_STEP);
	else
		mat = cv::Mat(frame_size, _type, (void*)frame.get_data());

	switch (frame_type)
	{
	case RS_IR_L:
		cv::imwrite("Ima/" + datetime + sensor + "_IR_L_" + std::to_string(i) + ".png", mat);
		break;
	case RS_IR_R:
		cv::imwrite("Ima/" + datetime + sensor + "_IR_R_" + std::to_string(i) + ".png", mat);
		break;
	case RS_RGB:
		cv::imwrite("Ima/" + datetime + sensor + "_RGB_" + std::to_string(i) + ".png", mat);
		break;
	}
}

int main(int argc, char** argv)
{
	int nSave = 5;
	int set = 1;
	if (argc > 1)
		set = std::atoi(argv[1]);
	std::cout << "Programa de Excavación, conjunto: " << set << std::endl;

	std::time_t t = std::time(nullptr);
	char mbstr[100];
	std::strftime(mbstr, sizeof(mbstr), "%Y%m%d_%H%M%S", std::localtime(&t));
	std::cout << mbstr << std::endl;

	//DEPTH 0
	int width00 = 480;
	int height00 = 270;
	int fps00 = 6;
	cv::Size ir_size0 = cv::Size(width00, height00);
	//RGB 0
	int width01 = 640;
	int height01 = 480;
	int fps01 = 6;
	cv::Size rgb_size0 = cv::Size(width01, height01);

	int width1 = 1280;
	int height1 = 800;
	int fps1 = 15;
	cv::Size ir_size = cv::Size(width1, height1);

	int width2 = 1280;//1920;//1280;
	int height2 = 720;//1080;// 720;
	int fps2 = 15;
	cv::Size rgb_size = cv::Size(width2, height2);

	//Add desired streams to configuration
	rs2::config cfg0;
	rs2::config cfg1;
	rs2::config cfg2;
	
	cfg0.enable_stream(RS2_STREAM_INFRARED, 1, width00, height00, RS2_FORMAT_Y8, fps00);
	cfg0.enable_stream(RS2_STREAM_INFRARED, 2, width00, height00, RS2_FORMAT_Y8, fps00);
	cfg0.enable_stream(RS2_STREAM_DEPTH, width00, height00, RS2_FORMAT_Z16, fps00);
	cfg0.enable_stream(RS2_STREAM_COLOR, width01, height01, RS2_FORMAT_BGR8, fps01);

	cfg1.enable_stream(RS2_STREAM_INFRARED, 1, width1, height1, RS2_FORMAT_Y16, fps1);
	cfg1.enable_stream(RS2_STREAM_INFRARED, 2, width1, height1, RS2_FORMAT_Y16, fps1);
	cfg1.enable_stream(RS2_STREAM_COLOR, width2, height2, RS2_FORMAT_BGR8, fps2);

	cfg2.enable_stream(RS2_STREAM_INFRARED, 1, width2, height2, RS2_FORMAT_Y8, fps2);
	cfg2.enable_stream(RS2_STREAM_INFRARED, 2, width2, height2, RS2_FORMAT_Y8, fps2);
	cfg2.enable_stream(RS2_STREAM_DEPTH, width2, height2, RS2_FORMAT_Z16, fps2);

	const auto window_name_d = "depth";
	namedWindow(window_name_d, cv::WINDOW_NORMAL);

	const auto window_name_c = "color";
	namedWindow(window_name_c, cv::WINDOW_NORMAL);

	// Declare RealSense pipeline, encapsulating the actual device and sensors
	rs2::pipeline pipe;

	//Instruct pipeline to start streaming with the requested configuration
	rs2::pipeline_profile pipe_profile = pipe.start(cfg0);
	auto d_sensor = pipe_profile.get_device().first<rs2::depth_sensor>();
	d_sensor.set_option(RS2_OPTION_EMITTER_ENABLED, 0.f);
	d_sensor.set_option(RS2_OPTION_DEPTH_UNITS, 0.0001f); //0.0001f
	rs2::colorizer color_map;
	color_map.set_option(RS2_OPTION_COLOR_SCHEME, 2.f);

	cv::Mat dMat_color0;
	cv::Mat dMat_depth0;
	int cont = 0;

	cv::Mat dMat_depth;

	while (1)
	{
		cont = (cont + 1) % 10;
		// wait for frames and get frameset
		rs2::frameset frameset = pipe.wait_for_frames();
		rs2::video_frame color_frame = frameset.get_color_frame();
		rs2::frame depth = frameset.get_depth_frame().apply_filter(color_map);

		dMat_color0 = cv::Mat(rgb_size0, CV_8UC3, (void*)color_frame.get_data(), cv::Mat::AUTO_STEP);
		dMat_depth0 = cv::Mat(ir_size0, CV_8UC3, (void*)depth.get_data(), cv::Mat::AUTO_STEP);

		cv::imshow(window_name_c, dMat_color0);
		cv::imshow(window_name_d, dMat_depth0);

		if (cont == 0)
			std::cout << "Mean: " << cv::mean(dMat_depth0).val[0] << std::endl;


		char c = cv::waitKey(1);
		if (c == 's')
		{
			pipe.stop();
			pipe_profile = pipe.start(cfg1);
			d_sensor = pipe_profile.get_device().first<rs2::depth_sensor>();
			d_sensor.set_option(RS2_OPTION_EMITTER_ENABLED, 0.f);
			d_sensor.set_option(RS2_OPTION_DEPTH_UNITS, 0.0001f); //0.0001f
			std::cout << "emitter off: " << std::endl;
			std::cout << "Saving..." << std::endl;
			for (int k = 0; k < 2; k++)
			{
				std::string sensor = "_NS";
				if (k==1)
				{
					sensor = "_YS";
				}
				for (auto i = 0; i < 30; ++i) pipe.wait_for_frames();

				for (int i = 0; i < nSave; i++) {
					t = std::time(nullptr);
					std::strftime(mbstr, sizeof(mbstr), "%Y%m%d_%H%M%S", std::localtime(&t));
					std::string datetime = mbstr;
					frameset = pipe.wait_for_frames();
					rs2::video_frame ir_frame_left = frameset.get_infrared_frame(1);
					rs2::video_frame ir_frame_right = frameset.get_infrared_frame(2);
					rs2::video_frame color_frame = frameset.get_color_frame();

					save_img(std::ref(ir_frame_left), std::ref(ir_size), RS_IR_L, CV_16UC1, datetime, sensor, i);
					save_img(std::ref(ir_frame_right), std::ref(ir_size), RS_IR_R, CV_16UC1, datetime, sensor, i);
					save_img(std::ref(color_frame), std::ref(rgb_size), RS_RGB, CV_8UC3, datetime, sensor, i);
				}

				//Second capture
				pipe.stop();
				pipe_profile = pipe.start(cfg2);

				for (auto i = 0; i < 30; ++i) pipe.wait_for_frames();

				for (int i = 0; i < nSave; i++) {
					t = std::time(nullptr);
					std::strftime(mbstr, sizeof(mbstr), "%Y%m%d_%H%M%S", std::localtime(&t));
					std::string datetime = mbstr;
					depth = pipe.wait_for_frames().get_depth_frame();

					dMat_depth = cv::Mat(cv::Size(width2, height2), CV_16UC1, (void*)depth.get_data());
					cv::imwrite("Ima/" + datetime + sensor + "_D_" + std::to_string(i) + ".png", dMat_depth);
				}

				pipe.stop();
				pipe_profile = pipe.start(cfg1);

				if (k == 0) {
					d_sensor = pipe_profile.get_device().first<rs2::depth_sensor>();
					d_sensor.set_option(RS2_OPTION_EMITTER_ENABLED, 1.f);
					d_sensor.set_option(RS2_OPTION_DEPTH_UNITS, 0.0001f); //0.0001f
					std::cout << "units: " << d_sensor.get_depth_scale() << std::endl;
					std::cout << "emitter on: " << std::endl;
				}
				else
				{
					d_sensor = pipe_profile.get_device().first<rs2::depth_sensor>();
					d_sensor.set_option(RS2_OPTION_EMITTER_ENABLED, 0.f);
					d_sensor.set_option(RS2_OPTION_DEPTH_UNITS, 0.0001f); //0.0001f
					std::cout << "emitter off: " << std::endl;
				}
			}
			pipe.stop();
			pipe_profile = pipe.start(cfg0);
		}
		else if (c == 'q') 
		{
			pipe.stop();
			break;
		}
	}

	system("pause");
	return 0;
}
