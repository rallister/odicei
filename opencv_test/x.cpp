#include <stdio.h>
#include "opencv2/opencv.hpp"
using namespace std;
using namespace cv;
/*
If opencv2/opencv.hpp is not being found

Check output pkg-config --libs opencv4
	e.g., -I/usr/include/opencv4/opencv -I/usr/include/opencv4
And ln one of them:sudo ln -s /usr/include/opencv4/opencv2 /usr/local/include/opencv4

*/

// Compile as:  g++ x.cpp -o HelloOpenCV `pkg-config --libs opencv4`

int main(int argc, char** argv )
{

    cv::Mat image, outImg; 
    image = cv::imread("/home/kami/misc/treasure_dog/TreasurePicks/Treasure_to_site/20221006_204641.jpg", IMREAD_COLOR);
	
	//cv::resize(image, outImg, cv::Size(), 0.09, 0.9, cv::INTER_LANCZOS4 );
	cv::resize(image, outImg,  cv::Size(302, 403), 0,0, cv::INTER_LANCZOS4 );
    cv::Size size = outImg.size();
	printf("%dx%d\n", size.width, size.height);
	
	vector<int> compression_params;
    compression_params.push_back(IMWRITE_JPEG_QUALITY);
    compression_params.push_back(60);
	
	imwrite("nooo.jpg", outImg, compression_params);
    return 0;
}