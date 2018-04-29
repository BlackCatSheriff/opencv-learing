#include <opencv2\opencv.hpp>
#include <fstream>
using namespace std;
using namespace cv;

int main()
{
	
	int image_count = 1;                    /****    ͼ������     ****/
	vector<Point2f> corners;                  /****    ����ÿ��ͼ���ϼ�⵽�Ľǵ�       ****/
	vector<Mat>  image_Seq;

	int count = 0;
	for (int i = 0; i != image_count; i++)
	{
		string imageFileName;
		std::stringstream StrStm;
		StrStm << i + 1;
		StrStm >> imageFileName;
		imageFileName += ".jpg";
		cv::Mat image = imread("img" + imageFileName);
		image_Seq.push_back(image);
	}
	cout << "�ǵ���ȡ��ɣ�\n";
	///* ��ʼ���� */
	Size image_size = image_Seq[0].size();
	
	cv::Matx33d intrinsic_matrix;    /*****    ������ڲ�������    ****/
	cv::Vec4d distortion_coeffs;     /* �������4������ϵ����k1,k2,k3,k4*/
	intrinsic_matrix << 360.7406034419511, 0, 311.4822234334455, 0, 362.1742734247002, 219.5196371955846, 0, 0, 1;
	distortion_coeffs << -0.487037, 1.12335, -1.18219, -0.00255861;

	
	Mat mapx = Mat(image_size, CV_32FC1);
	Mat mapy = Mat(image_size, CV_32FC1);
	Mat R = Mat::eye(3, 3, CV_32F);

	cout << "�������ͼ��" << endl;
	for (int i = 0; i != image_count; i++)
	{
		cout << "Frame #" << i + 1 << "..." << endl;
		Mat newCameraMatrix = Mat(3, 3, CV_32FC1, Scalar::all(0));
		fisheye::initUndistortRectifyMap(intrinsic_matrix, distortion_coeffs, R, intrinsic_matrix, image_size, CV_32FC1, mapx, mapy);
		Mat t = image_Seq[i].clone();
		cv::remap(image_Seq[i], t, mapx, mapy, INTER_LINEAR);
		string imageFileName;
		std::stringstream StrStm;
		StrStm << i + 1;
		StrStm >> imageFileName;
		imageFileName += "_d.jpg";
		imwrite(imageFileName, t);
	}
	cout << "�������" << endl;
	return 0;
}