#include <iostream>
#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;

void make_map(Size src_size, Size dst_size, Point2i circle_center, int circle_radius, float theta, float phi, float FOV, Mat &map_x, Mat &map_y) {
	/*
		src_size : source image size()
		dst_size : destination image size()
		circle_center : center of circle
		circle_radius : circle radius
		float theta, phi : angle of view (in sphere space) (theta for z rotation, phi for down angle)
			0 <= theta <= 1
			0 <= phi <= 1
		FOV : (horizontal)angle of view
			0 <= FOV <= 1
	*/
	map_x.create(dst_size, CV_32FC1);
	map_y.create(dst_size, CV_32FC1);

	// output image size
	int remap_width = dst_size.width; //dst.cols;
	int remap_height = dst_size.height; //dst.rows;

	float view_width = CV_PI * FOV;
	float view_height = view_width * ((float)remap_height / (float)remap_width);

	float view_theta = 2 * CV_PI * theta;	// 0 <= theta <= 1
	float view_phi = ((CV_PI / 2) - (view_height / 2))* phi; // 0 <= phi <= 1

	float aXr = view_width / remap_width;
	float bXr = view_width / 2;
	float aYr = view_height / remap_height;
	float bYr = view_height / 2;

	float sin_view_phi = sin(-view_phi);
	float cos_view_phi = cos(-view_phi);
	float sin_view_theta = sin(-view_theta);
	float cos_view_theta = cos(-view_theta);

	for (int x = 0; x < remap_width; x++) {
		for (int y = 0; y < remap_height; y++) {
			// flat 이미지에서 좌표값에 따른 회전각(왼쪽위 = +Xr - Yr 오른쪽아래 = -Xr + Yr)
			float Xr = x * aXr - bXr;
			float Yr = y * aYr - bYr;

			// 회전각에 따른 view에서의 벡터값(중심축은 view 벡터, 그로부터 벌어진 각도가 Xr Xy)
			float Vx = -tan(Yr);
			float Vy = tan(Xr);

			// view로부터의 백터값 V 를 view 벡터에 맞도록 변환(y_rotate->z_rotate)->구면에서의 벡터값
			float Vx_ = cos_view_phi * Vx - sin_view_phi;
			float Vy_ = Vy;
			float Vz_ = -cos_view_phi - sin_view_phi * Vx;

			float Vx__ = cos_view_theta * Vx_ - sin_view_theta * Vy_;
			float Vy__ = sin_view_theta * Vx_ + cos_view_theta * Vy_;
			float Vz__ = Vz_;

			// 벡터의 세타 파이값 추출
			float V_theta = atan2(Vy__, Vx__);
			float V_phi = atan2(sqrt(Vx__*Vx__ + Vy__ * Vy__), (-Vz__));

			// x, y 좌표
			float r = (V_phi / (CV_PI / 2)) * (circle_radius);
			float Cx = r * cos(V_theta);
			float Cy = r * sin(V_theta);

			// 구면 이미지에서의 양수 x, y 좌표값
			map_x.at<float>(y, x) = Cx + circle_center.x;
			map_y.at<float>(y, x) = Cy + circle_center.y;
		}
	}
}

int main(int argc, char** argv) { // args for noting!!
	VideoCapture cap("C:/Users/UfXpri/Desktop/dewarp/dewarp/43.mp4");
	if (!cap.isOpened())
		return -1;

	// get frame size
	Mat frame;
	cap >> frame;

	int image_width = frame.cols;
	int image_height = frame.rows;
	int remap_width = 1280;				// TODO define you own output resolution
	int remap_height = 720;				// TODO define you own output resolution
	Point2i circle_center(1301, 935);	// TODO define your own circle center (px)
	int circle_radius = 845;			// TODO define your own circle radius (px)

	Mat quad(remap_height*2, remap_width*2, CV_8UC3);	// quad view
	Mat remap0(remap_height, remap_width, CV_8UC3);		// top left
	Mat remap1(remap_height, remap_width, CV_8UC3);		// top right
	Mat remap2(remap_height, remap_width, CV_8UC3);		// bottom right
	Mat remap3(remap_height, remap_width, CV_8UC3);		// bottom left

	Mat map_x0, map_y0;
	Mat map_x1, map_y1;
	Mat map_x2, map_y2;
	Mat map_x3, map_y3;

	// make mapping info
	make_map(frame.size(), remap0.size(), circle_center, circle_radius, 1 / 8.0, 19 / 20.0, 2 / 5.0, map_x0, map_y0);
	make_map(frame.size(), remap1.size(), circle_center, circle_radius, 3 / 8.0, 19 / 20.0, 2 / 5.0, map_x1, map_y1);
	make_map(frame.size(), remap2.size(), circle_center, circle_radius, 5 / 8.0, 19 / 20.0, 2 / 5.0, map_x2, map_y2);
	make_map(frame.size(), remap3.size(), circle_center, circle_radius, 7 / 8.0, 19 / 20.0, 2 / 5.0, map_x3, map_y3);

	namedWindow("quad", 1);
	for (;;) {
		cap >> frame;
		// remap frame
		remap(frame, remap0, map_x0, map_y0, CV_INTER_LINEAR);
		remap(frame, remap1, map_x1, map_y1, CV_INTER_LINEAR);
		remap(frame, remap2, map_x2, map_y2, CV_INTER_LINEAR);
		remap(frame, remap3, map_x3, map_y3, CV_INTER_LINEAR);

		// concatnate 4 image
		Mat H0, H1;
		hconcat(remap1, remap0, H0);
		hconcat(remap3, remap2, H1);
		vconcat(H0, H1, quad);

		// resize
		Mat Sframe, Squad;
		resize(frame, Sframe, Size(), 1 / 4.0, 1 / 4.0, 1);
		resize(quad , Squad , Size(), 1 / 4.0, 1 / 4.0, 1);
		imshow("frame", Sframe);
		imshow("quad" , Squad );

		waitKey(1);
	}
}