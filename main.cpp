#include "opencv2/opencv.hpp"
#include <iostream>
#include "ArmorDetector.hpp"
#include "FlowerDetector.hpp"


using namespace cv;
using namespace std;

//void Detector(const ArmorParam & para = ArmorParam());
//void Detector2(const ArmorParam & para = ArmorParam());

int main(){
//    Detector();
//    Detector2();
//    ArmorDetector Armor;
//    Armor.outPut();
    FlowerDetector Flower;
    Flower.outPut();
    waitKey(0);
    return 0;
}