//
// Created by greedbob on 18-12-2.
//

#ifndef OPENCV_EXAMPLE_PROJECT_ARMORDETECTOR_HPP
#define OPENCV_EXAMPLE_PROJECT_ARMORDETECTOR_HPP

#endif //OPENCV_EXAMPLE_PROJECT_ARMORDETECTOR_HPP

#include "opencv2/opencv.hpp"
#define POINT_DIST(p1,p2) std::sqrt((p1.x-p2.x)*(p1.x-p2.x) + (p1.y-p2.y)*(p1.y-p2.y))
typedef unsigned char uchar;

enum EnemyColor { RED = 0, BLUE = 1};


struct ArmorParam {
    uchar min_light_gray;	        // 板灯最小灰度值
    uchar max_light_gray;
    uchar min_light_height;			// 板灯最小高度值
    uchar avg_contrast_threshold;	// 对比度检测中平均灰度差阈值，大于该阈值则为显著点
    uchar light_slope_offset;		// 允许灯柱偏离垂直线的最大偏移量，单位度
    int  max_light_delta_h;         // 左右灯柱在水平位置上的最大差值，像素单位
    uchar min_light_delta_h;		// 左右灯柱在水平位置上的最小差值，像素单位
    uchar max_light_delta_v;		// 左右灯柱在垂直位置上的最大差值，像素单位
    uchar max_light_delta_angle;	// 左右灯柱在斜率最大差值，单位度
    uchar avg_board_gray_threshold; // 矩形区域平均灰度阈值，小于该阈值则选择梯度最小的矩阵
    uchar avg_board_grad_threshold; // 矩形区域平均梯度阈值，小于该阈值则选择梯度最小的矩阵
    uchar grad_threshold;			// 矩形区域梯度阈值，在多个矩形区域中选择大于该阈值像素个数最少的区域  (not used)
    uchar br_threshold;				// 红蓝通道相减后的阈值
    uchar enemy_color;                 // 0 for red, otherwise blue

    ArmorParam(){
        min_light_gray = 210;
        max_light_gray = 255;
        min_light_height = 8;
        avg_contrast_threshold = 110;
        light_slope_offset = 30;
        max_light_delta_h = 450;
        min_light_delta_h = 12;
        max_light_delta_v = 50;
        max_light_delta_angle = 30;
        avg_board_gray_threshold = 80;
        avg_board_grad_threshold = 25;
        grad_threshold = 25;
        br_threshold = 30;
        enemy_color = 0;
    }
};

class ArmorDetector{
public:
    void outPut();

private:
    void setImage(const ArmorParam & para = ArmorParam());
    void setImage_origin(const ArmorParam & para = ArmorParam());
    void findContourInEnemyColor(cv::Mat & left, cv::Mat & right,
            std::vector<std::vector<cv::Point2i> > &contours_left, std::vector<std::vector<cv::Point2i> > &contours_right);
    cv::RotatedRect boundingRRect(const cv::RotatedRect & left, const cv::RotatedRect & right);
    cv::RotatedRect adjustRRect(const cv::RotatedRect & rect);
    void findTargetInContours(const std::vector<std::vector<cv::Point> > & contours_left,
            const std::vector<std::vector<cv::Point> > & contours_right,
            std::vector<cv::RotatedRect> & rects, std::vector<double> & score);
    void showrects(std::vector<cv::RotatedRect> & rects);
    void initTemplate();
//    void chooseTarget();

private:
    cv::Mat src = cv::imread("/home/greedbob/rm/r1.jpg");
    cv::Mat template_src = cv::imread("/home/greedbob/rm/template.bmp", 1);
    cv::Mat template_small = cv::imread("/home/greedbob/rm/small_template.bmp", 1);
    cv::Mat aim;
    cv::Mat max_color;
    cv::Mat src_channel_r;
    cv::Mat src_channel_g;
    cv::Mat src_channel_b;
//    cv::Mat left;
//    cv::Mat right;
//    cv::Mat left = cv::Mat::zeros(src_channel_r.size(), CV_8UC1);
//    cv::Mat right = cv::Mat::zeros(src_channel_r.size(), CV_8UC1);
    ArmorParam para;

};