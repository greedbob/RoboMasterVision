//
// Created by greedbob on 18-12-19.
//
#include "opencv2/opencv.hpp"
#include <iostream>
#include <queue>

#ifndef OPENCV_EXAMPLE_PROJECT_FLOWERDETECTOR_HPP
#define OPENCV_EXAMPLE_PROJECT_FLOWERDETECTOR_HPP

using namespace cv;
using namespace std;

class FlowerDetector {
public:
    void outPut();

private:
    void setImage();
    void showRects(vector<RotatedRect> & rects);
    void findAll();


private:
    Mat src = cv::imread("/home/greedbob/rm/flower_1.jpg");
//    Mat src = imread("/home/greedbob/rm/r1.jpg");
    Mat src_gray;
    Mat src_red;
    Mat src_red_gray;
    Mat src_channel_r;
    Mat src_channel_g;
    Mat src_channel_b;
    Mat max_color;
    Mat output;
    vector<RotatedRect> rects;

};


#endif //OPENCV_EXAMPLE_PROJECT_FLOWERDETECTOR_HPP
