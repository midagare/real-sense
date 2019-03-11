#include <opencv2/opencv.hpp>

int main(int argc, char** argv)
{
	cv::String imageName("bones.jpg");
	if (argc > 1) {
		imageName = argv[1];
	}
	cv::Mat img = cv::imread(imageName);
	cv::namedWindow("image", cv::WINDOW_AUTOSIZE);
	cv::imshow("image", img);
	cv::waitKey(0);
	return 0;
}
