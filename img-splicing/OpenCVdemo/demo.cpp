#include<opencv2/opencv.hpp>
#include<opencv2/xfeatures2d.hpp>
#include<iostream>
#include<algorithm>
#include<math.h>
using namespace std;
using namespace cv;
using namespace cv::xfeatures2d;

typedef struct
{
	Point2f left_top;
	Point2f left_bottom;
	Point2f right_top;
	Point2f right_bottom;
}four_corners_t;

four_corners_t corners;


void OptimizeSeam(Mat& img1, Mat& trans, Mat& dst);
void CalcCorners(const Mat& H, const Mat& src);

int main(int argc, char** argv)
{
	Mat image1 = imread("q1.jpg", IMREAD_COLOR);
	Mat image2 = imread("q2.jpg", IMREAD_COLOR);
	//Mat image1 = imread("q1.jpg", IMREAD_COLOR);
	imshow("p1", image1);
	imshow("p2", image2);
	//-- Step 1: Detect the keypoints using SURF Detector
	int minHessian = 400;
	Ptr<SURF> detector = SURF::create(minHessian);
	std::vector<KeyPoint> obj_keypoint, scene_keypoint;
	detector->detect(image2, obj_keypoint);
	detector->detect(image1, scene_keypoint);
	//computer the descriptors
	Mat obj_descriptors, scene_descriptors;
	detector->compute(image2, obj_keypoint, obj_descriptors);
	detector->compute(image1, scene_keypoint, scene_descriptors);
	//use BruteForce to match,and get good_matches
	BFMatcher matcher;
	vector<DMatch> matches;
	matcher.match(obj_descriptors, scene_descriptors, matches);
	sort(matches.begin(), matches.end());  //ɸѡƥ���
	vector<DMatch> good_matches;
	for (int i = 0; i < min(50, (int)(matches.size()*0.15)); i++) {
		good_matches.push_back(matches[i]);
	}
	//draw matches
	Mat imgMatches;
	drawMatches(image2, obj_keypoint, image1, scene_keypoint, good_matches, imgMatches);
	//get obj bounding
	vector<Point2f> obj_good_keypoint;
	vector<Point2f> scene_good_keypoint;
	for (int i = 0; i < good_matches.size(); i++) {
		obj_good_keypoint.push_back(obj_keypoint[good_matches[i].queryIdx].pt);
		scene_good_keypoint.push_back(scene_keypoint[good_matches[i].trainIdx].pt);
	}
	Mat homo = findHomography(obj_good_keypoint, scene_good_keypoint, RANSAC); //find the perspective transformation between the source and the destination
	CalcCorners(homo, image2);
	Mat imageTransform;
	warpPerspective(image2, imageTransform, homo, Size(MAX(corners.right_top.x, corners.right_bottom.x), image1.rows));
	imshow("transform", imageTransform);
	//����ƴ�Ӻ��ͼ,����ǰ����ͼ�Ĵ�С
	int dst_width = imageTransform.cols;  //ȡ���ҵ�ĳ���Ϊƴ��ͼ�ĳ���
	int dst_height = image1.rows;
	Mat dst(dst_height, dst_width, CV_8UC3);
	dst.setTo(0);
	imageTransform.copyTo(dst(Rect(0, 0, imageTransform.cols, imageTransform.rows)));
	image1.copyTo(dst(Rect(0, 0, image1.cols, image1.rows)));
	imshow("b_dst", dst);
	OptimizeSeam(image1, imageTransform, dst);
	imshow("dst", dst);
	imwrite("dst_img.jpg", dst);
	waitKey(0);
	return 0;
}
void OptimizeSeam(Mat& img1, Mat& trans, Mat& dst)
{
	int start = MIN(corners.left_top.x, corners.left_bottom.x);//��ʼλ�ã����ص��������߽�  

	double processWidth = img1.cols - start;//�ص�����Ŀ��  
	int rows = dst.rows;
	int cols = img1.cols; //ע�⣬������*ͨ����
	double alpha = 1;//img1�����ص�Ȩ��  
	for (int i = 0; i < rows; i++)
	{
		uchar* p = img1.ptr<uchar>(i);  //��ȡ��i�е��׵�ַ
		uchar* t = trans.ptr<uchar>(i);
		uchar* d = dst.ptr<uchar>(i);
		for (int j = start; j < cols; j++)
		{
			//�������ͼ��trans�������صĺڵ㣬����ȫ����img1�е�����
			if (t[j * 3] == 0 && t[j * 3 + 1] == 0 && t[j * 3 + 2] == 0)
			{
				alpha = 1;
			}
			else
			{
				//img1�����ص�Ȩ�أ��뵱ǰ�������ص�������߽�ľ�������ȣ�ʵ��֤�������ַ���ȷʵ��  
				alpha = (processWidth - (j - start)) / processWidth;
			}

			d[j * 3] = p[j * 3] * alpha + t[j * 3] * (1 - alpha);
			d[j * 3 + 1] = p[j * 3 + 1] * alpha + t[j * 3 + 1] * (1 - alpha);
			d[j * 3 + 2] = p[j * 3 + 2] * alpha + t[j * 3 + 2] * (1 - alpha);

		}
	}
}

void CalcCorners(const Mat& H, const Mat& src)
{
	double v2[] = { 0, 0, 1 };//���Ͻ�
	double v1[3];//�任�������ֵ
	Mat V2 = Mat(3, 1, CV_64FC1, v2);  //������
	Mat V1 = Mat(3, 1, CV_64FC1, v1);  //������

	V1 = H * V2;
	//���Ͻ�(0,0,1)
	cout << "V2: " << V2 << endl;
	cout << "V1: " << V1 << endl;
	corners.left_top.x = v1[0] / v1[2];
	corners.left_top.y = v1[1] / v1[2];

	//���½�(0,src.rows,1)
	v2[0] = 0;
	v2[1] = src.rows;
	v2[2] = 1;
	V2 = Mat(3, 1, CV_64FC1, v2);  //������
	V1 = Mat(3, 1, CV_64FC1, v1);  //������
	V1 = H * V2;
	corners.left_bottom.x = v1[0] / v1[2];
	corners.left_bottom.y = v1[1] / v1[2];

	//���Ͻ�(src.cols,0,1)
	v2[0] = src.cols;
	v2[1] = 0;
	v2[2] = 1;
	V2 = Mat(3, 1, CV_64FC1, v2);  //������
	V1 = Mat(3, 1, CV_64FC1, v1);  //������
	V1 = H * V2;
	corners.right_top.x = v1[0] / v1[2];
	corners.right_top.y = v1[1] / v1[2];

	//���½�(src.cols,src.rows,1)
	v2[0] = src.cols;
	v2[1] = src.rows;
	v2[2] = 1;
	V2 = Mat(3, 1, CV_64FC1, v2);  //������
	V1 = Mat(3, 1, CV_64FC1, v1);  //������
	V1 = H * V2;
	corners.right_bottom.x = v1[0] / v1[2];
	corners.right_bottom.y = v1[1] / v1[2];

}
