//
// Created by greedbob on 18-12-2.
//

#include "ArmorDetector.hpp"
#include <iostream>
#include <queue>

using namespace cv;
using namespace std;

void ArmorDetector::setImage(const ArmorParam & para){
    Mat splitChannels[3];
    split(src, splitChannels);
    src_channel_r = splitChannels[2];
    src_channel_g = splitChannels[1];
    src_channel_b = splitChannels[0];
    if (para.enemy_color == RED){
        aim = src_channel_r;
        threshold(aim, max_color, para.min_light_gray, para.max_light_gray, THRESH_BINARY);
    }
    else{
        aim = src_channel_b;
        threshold(aim, max_color, para.min_light_gray, para.max_light_gray, THRESH_BINARY);
    }
}


void ArmorDetector::setImage_origin(const ArmorParam & para){
    int total_pixel = src.cols * src.rows;
    const uchar * ptr_src = src.data; ///首地址
    const uchar * ptr_src_end = src.data + total_pixel * 3;

    src_channel_g.create(src.size(), CV_8UC1); ///Unsigned 8bits 1 通道
    aim.create(src.size(), CV_8UC1);
    max_color = cv::Mat(src.size(), CV_8UC1, cv::Scalar(0)); ///全黑
    uchar *ptr_g = src_channel_g.data, *ptr_aim = aim.data, *ptr_max_color = max_color.data;
    if (para.enemy_color == RED){
        for (; ptr_src != ptr_src_end; ++ptr_src, ++ptr_g, ++ptr_max_color, ++ptr_aim)	{
            uchar b = *ptr_src;
            uchar g = *(++ptr_src);
            uchar r = *(++ptr_src);
            *ptr_g = g; /// 提取绿通道
            *ptr_aim = r; /// 提取目标色
            //*ptr_g = b;
            if (r > para.min_light_gray) /// 全黑中画出红色区域
                *ptr_max_color = 255;
//            if (r - b > _para.br_threshold && r >= g)
//                *ptr_max_color = 255;
        }
    }
    else {
        for (; ptr_src != ptr_src_end; ++ptr_src, ++ptr_g, ++ptr_max_color, ++ptr_aim)	{
            uchar b = *ptr_src;
            uchar g = *(++ptr_src);
            uchar r = *(++ptr_src);
            *ptr_g = g;
            *ptr_aim = b;
            //*ptr_g = r;
            if (b > para.min_light_gray) /// 全黑中画出蓝色区域
                *ptr_max_color = 255;
//            if (b - r > _para.br_threshold && b >= g)
//                *ptr_max_color = 255;
        }
    }
}


void ArmorDetector::initTemplate(){
    Mat bgr[3];
    split(template_src, bgr);

    Mat temp_green;
    resize(bgr[1], temp_green, Size(100,25));
    cv::threshold(temp_green, template_src, 128, 255, THRESH_OTSU);

    split(template_small, bgr);
    resize(bgr[1], temp_green, Size(100,25));
    cv::threshold(temp_green, template_small, 128, 255, THRESH_OTSU);
}


void ArmorDetector::findContourInEnemyColor(
        cv::Mat & left, cv::Mat & right,
        vector<vector<Point2i> > &contours_left,
        vector<vector<Point2i> > &contours_right) {
    // find contour in sub image of blue and red
    vector<vector<Point2i> > contours_br; ///store points of contour
    vector<Vec4i> hierarchy; ///各个轮廓的继承关系，长度和contours相等，每个元素和contours的元素对应。
    findContours(max_color, contours_br, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
    /// CV_RETR_EXTERNAL：只检测外轮廓。忽略轮廓内部的洞
    /// CV_CHAIN_APPROX_SIMPLE：只存储水平，垂直，对角直线的起始点
    vector<vector<Point2i> >::const_iterator it = contours_br.begin(); ///轮廓点集的开头

    // left lamp template
    // o o x x x x x x
    // o o x x x x x x
    // o o x x x x x x

    // right lamp template
    // x x x x x x o o
    // x x x x x x o o
    // x x x x x x o o

    // margin_l -> col of 'o'
    // margin_r -> col of 'x'
    // margin_h -> row of 'o' or 'x'

    // using average gray value of 'x' minus average gray value of 'o'
    // if the value lager than threshold, the point is consider as a lamp point

    left = Mat::zeros(max_color.size(), CV_8UC1);
    right = Mat::zeros(max_color.size(), CV_8UC1);
    const int margin_l = 1, margin_r = 10;
    const int margin_h = 3;

    while (it != contours_br.end()) {
        Rect rect = cv::boundingRect(*it);
        if (rect.height < para.min_light_height) {
            ++it; ///?
            continue;
        }

        int max_i = rect.x + rect.width;
        max_i = std::min(max_color.cols, max_i + margin_r);
        int half_j = (margin_h >> 1), max_j = rect.y + rect.height, min_j = rect.y; ///?
        /// >> 相当于除二且向下取整
        max_j = std::min(max_color.rows - 1, max_j + half_j);
        min_j = std::max(min_j, half_j);
        int count_left = 0, count_right = 0;

        const uchar *ptr_gray_base = src_channel_g.data;
        /// _g是与src同大的 Unsigned 8bits 1 通道的黑色图片
        for (size_t j = min_j; j < max_j; ++j) {
            const uchar *ptr_gray = ptr_gray_base + j * src_channel_g.cols;
            for (size_t i = rect.x; i < max_i; ++i) {
                if (*(ptr_gray + i) < para.min_light_gray)
                    continue;

                float block0 = 0, block1 = 0, block_1 = 0;
                // do margin protection
                if (i >= margin_r) {
                    for (int m = -half_j; m <= half_j; ++m) {
                        const uchar *ptr = ptr_gray + m * src_channel_g.cols + i;
                        // for common 'o' of template
                        for (int k = 0; k < margin_l; k++)
                            block0 += *(ptr + k);
                        // for 'x' of left template
                        for (int k = margin_l; k < margin_r; k++)
                            block1 += *(ptr + k);
                        // for 'x' of right template
                        for (int k = -margin_l; k > -margin_r; --k)
                            block_1 += *(ptr + k);
                    }
                    block0 /= margin_h * margin_l;
                    block1 /= margin_h * (margin_r - margin_l);
                    block_1 /= margin_h * (margin_r - margin_l);
                    int avgdist = block0 - block1;
                    left.at<uchar>(j, i) = avgdist > para.avg_contrast_threshold ? (++count_left, 255) : 0;
                    avgdist = block0 - block_1;
                    right.at<uchar>(j, i) = avgdist > para.avg_contrast_threshold ? (++count_right, 255) : 0;
                } else {
                    for (int m = -half_j; m <= half_j; ++m) {
                        const uchar *ptr = ptr_gray + m * src_channel_g.cols + i;
                        // for 'o' of left template
                        for (int k = 0; k < margin_l; k++)
                            block0 += *(ptr + k);
                        // for 'x' of left template
                        for (int k = margin_l; k < margin_r; k++)
                            block1 += *(ptr + k);;
                    }
                    block0 /= margin_h * margin_l;
                    block1 /= margin_h * (margin_r - margin_l);
                    int avgdist = block0 - block1;
                    left.at<uchar>(j, i) = avgdist > para.avg_contrast_threshold ? (++count_left, 255) : 0;
                }
            }
        }

        // find the lamp contours
        if (count_left > 10) {
            vector<vector<Point2i> > contour;
            vector<Vec4i> hierarchy;
            findContours(left(rect), contour, hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE,
                         cv::Point2i(rect.x, rect.y));
            contours_left.insert(contours_left.end(), contour.begin(), contour.end());
        }

        if (count_right > 10) {
            vector<vector<Point2i> > contour;
            vector<Vec4i> hierarchy;
            findContours(right(rect), contour, hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE,
                         cv::Point2i(rect.x, rect.y));
            contours_right.insert(contours_right.end(), contour.begin(), contour.end());
        }
        ++it;
    }
}


RotatedRect ArmorDetector::boundingRRect(const cv::RotatedRect & left, const cv::RotatedRect & right){
    const Point & pl = left.center, & pr = right.center;
    Point2f center = (pl + pr) / 2.0;
    cv::Size2f wh_l = left.size;
    cv::Size2f wh_r = right.size;
    float width = POINT_DIST(pl, pr) - (wh_l.width + wh_r.width) / 2.0;
    float height = std::max(wh_l.height, wh_r.height);
    //float height = (wh_l.height + wh_r.height) / 2.0;
    float angle = std::atan2(right.center.y - left.center.y, right.center.x - left.center.x);
    return RotatedRect(center, Size2f(width, height), angle * 180 / CV_PI);
}

RotatedRect ArmorDetector::adjustRRect(const RotatedRect & rect){
    const Size2f & s = rect.size;
    if (s.width < s.height)
        return rect;
    return RotatedRect(rect.center, Size2f(s.height, s.width), rect.angle + 90.0); /// 为什么旋转?
}

void ArmorDetector::findTargetInContours(const vector<vector<Point> > & contours_left,
        const vector<vector<Point> > & contours_right, vector<RotatedRect> & rects, std::vector<double> & score) {
    // 用直线拟合轮廓，找出符合斜率范围的轮廓
    vector<RotatedRect> final_contour_rect_left, final_contour_rect_right;
    vector<double> score_left, score_right;

    for (size_t i = 0; i < contours_left.size(); ++i){
        // fit the lamp contour as a eclipse 输入的点集中拟合一个面积最小的旋转矩形
        RotatedRect rrect = minAreaRect(contours_left[i]);
        rrect = adjustRRect(rrect);
        double angle = rrect.angle;
        angle = 90 - angle;
        angle = angle < 0 ? angle + 180 : angle;

        // the contour must be near-vertical 接近垂直
        float delta_angle = abs(angle - 90);
        if (delta_angle < para.light_slope_offset){
            final_contour_rect_left.push_back(rrect);
            score_left.push_back(delta_angle);
        }
    }

    for (size_t i = 0; i < contours_right.size(); ++i){
        // fit the lamp contour as a eclipse
        RotatedRect rrect = minAreaRect(contours_right[i]);
        rrect = adjustRRect(rrect);
        double angle = rrect.angle;
        angle = 90 - angle;
        angle = angle < 0 ? angle + 180 : angle;

        // the contour must be near-vertical
        float delta_angle = abs(angle - 90);
        if (delta_angle < para.light_slope_offset){
            final_contour_rect_right.push_back(rrect);
            score_right.push_back(delta_angle);
        }
    }

    // using all the left edge and right edge to make up rectangles 组合左右的矩形
    for (size_t i = 0; i < final_contour_rect_left.size(); ++i) {
        const RotatedRect & rect_i = final_contour_rect_left[i];
        const Point & center_i = rect_i.center;
        float xi = center_i.x;
        float yi = center_i.y;

        for (size_t j = 0; j < final_contour_rect_right.size(); j++) {
            const RotatedRect & rect_j = final_contour_rect_right[j];
            const Point & center_j = rect_j.center;
            float xj = center_j.x;
            float yj = center_j.y;
            float delta_h = xj - xi;
            float delta_angle = abs(rect_j.angle - rect_i.angle);

            // if rectangle is match condition, put it in candidate vector
            if (delta_h > para.min_light_delta_h && delta_h < para.max_light_delta_h &&
                abs(yi - yj) < para.max_light_delta_v &&
                delta_angle < para.max_light_delta_angle) {
                RotatedRect rect = boundingRRect(rect_i, rect_j);
                rects.push_back(rect);
                score.push_back((score_right[j] + score_left[i]) / 6.0 + delta_angle);
            }
        }
    }
}


 void ArmorDetector::outPut(){
    imshow("src", src);
    setImage();
//    imshow("aim", aim);
//    imshow("src_channel_g", src_channel_g);
    imshow("max_color", max_color);

    cv::Mat contrast_left, contrast_right;
    vector<vector<Point2i> > contours_left, contours_right;
    findContourInEnemyColor(contrast_left, contrast_right, contours_left, contours_right);
    imshow("left", contrast_left);
    imshow("right", contrast_right);

    vector<RotatedRect> rects;
    vector<double> score;
    findTargetInContours(contours_left, contours_right, rects, score);
//    RotatedRect final_rect = chooseTarget(rects, score);
    showrects(rects);
}


void ArmorDetector::showrects(vector<RotatedRect> & rects){
//    Mat img_rects = Mat(aim.size(), CV_8UC1, cv::Scalar(0));
    Mat img_rects = src;
//    const uchar * ptr_img_rects = img_rects.data;
    for (int i = 0; i < aim.cols * aim.rows; ++i){
        for (int j = 0; j < rects.size(); ++j){
            Point2f vertices[4];
            rects[j].points(vertices);
            for (int k = 0; k < 4; k++) {
                line(img_rects, vertices[k], vertices[(k + 1) % 4], Scalar(255, 255, 255));
//                Rect brect = rects[j].boundingRect();
//                rectangle(img_rects, brect, Scalar(255, 0, 0));
            }
        }
    }
    imshow("img_rects", img_rects);
}