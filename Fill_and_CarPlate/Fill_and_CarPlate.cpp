#include <iostream>
#include "../../Common/Common.h"
using namespace std;

int main()
{
    /* Load Image */
    
    //image path
    string image_Dir = "../../thirdparty/KCCImageNet/images/passat/";
    string fileName = image_Dir + "passat_30.png";

    //load image
    Mat img = imread(fileName, cv::ImreadModes::IMREAD_ANYCOLOR);
    
    /*-----------------------------------------------------------------------*/

    
    /* 1. fill in the blank */

    //create mask image 
    Mat dst;
    double thres_min = 0;
    double value = 255;
    threshold(img, dst, thres_min, value, ThresholdTypes::THRESH_BINARY);
    dst = ~dst;
    
    //mask closing
    Mat se = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
    cv::erode(dst, dst, se);   //침식(dilatation)
    cv::dilate(dst, dst, se);   //팽창(dilatation)

    Mat mask_img = dst.clone();
    //imwrite("mask_img.png", mask_img);
    
    //fill blank
    Mat fill_img;
    inpaint(img, mask_img, fill_img, 5, INPAINT_NS);
    imwrite("fill_img.png", fill_img);

    /*-----------------------------------------------------------------------*/

    
  
        
    /* 2. find car_plate */
   
    //threshold                                      
    Mat dst2;
    thres_min = 130;
    threshold(fill_img, dst2, thres_min, value, ThresholdTypes::THRESH_BINARY);
    dst2 = ~dst2;
   
    //find countours
    vector<vector<Point>>contours;
    vector<Vec4i> hierarchy;

    findContours(dst2, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

    int txt_index = 0;
    RNG rng(12345);
    vector<Rect> vCharROI;
    Point2f plate_tl, plate_br;
    Rect rect;  

    //번호판만 rectangle
    for (size_t i = 0; i < contours.size(); i++) 
    {
        double area = cv::contourArea(contours[i]);
        double length = cv::arcLength(contours[i], false);
        if((area < 1500) || (area > 2000)) continue;
        
        Rect rt_t = cv::boundingRect(contours[i]);  //값 가져오기
        rect = rt_t;
        //drawContours(fill_img, contours, (int)i, Scalar(0, 0, 255), 1, LINE_8, hierarchy, 0);  //외각선그리기
        cv::rectangle(fill_img, rt_t, Scalar(255, 255, 255), 1);  //상자만들기

        string msg = std::format("[{}]", i);
        //putText(fill_img, msg, rt_t.tl(), HersheyFonts::FONT_HERSHEY_SIMPLEX, 0.4, Scalar(255, 255, 255), 1);

        msg = std::format("[{}]-A {}-L {}", i, (int)area, (int)length);
        //putText(fill_img, msg, Point(10, 50 + txt_index * 20), HersheyFonts::FONT_HERSHEY_SIMPLEX, 0.4, Scalar(255, 255, 255), 1);
        txt_index++;

        vCharROI.push_back(rt_t);

        //좌표값 확인
        //cout << rt_t.tl()  <<rt_t.br() <<endl;
        plate_tl =(rt_t.tl());
        plate_br =(rt_t.br());
        //cout << plate_tl.x << plate_br.y << endl;
    }
    

    //번호판(rectangle) 부분만 자르기
    Mat plate_image = fill_img(Range(plate_tl.y, plate_br.y), Range(plate_tl.x, plate_br.x));
    imwrite("plate_image.png", plate_image);
    
    //뜨레띠홀드값 재설정
    thres_min = 79;
    Mat thresh_plate_imge;
    threshold(plate_image, thresh_plate_imge, thres_min, value, ThresholdTypes::THRESH_BINARY);
    thresh_plate_imge = ~thresh_plate_imge;
    Mat thresh_plate_imge2 = thresh_plate_imge.clone();


    //침식팽창으로 전처리
    cv::erode(thresh_plate_imge2, thresh_plate_imge2, se);
    cv::erode(thresh_plate_imge2, thresh_plate_imge2, se);
    cv::dilate(thresh_plate_imge2, thresh_plate_imge2, se);
    cv::dilate(thresh_plate_imge2, thresh_plate_imge2, se);
    cv::dilate(thresh_plate_imge2, thresh_plate_imge2, se);
    cv::dilate(thresh_plate_imge2, thresh_plate_imge2, se);
    cv::dilate(thresh_plate_imge2, thresh_plate_imge2, se);
    cv::erode(thresh_plate_imge2, thresh_plate_imge2, se);
    cv::erode(thresh_plate_imge2, thresh_plate_imge2, se);
    thresh_plate_imge = thresh_plate_imge - thresh_plate_imge2;

    
    //findContours
    
    findContours(thresh_plate_imge, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
    
    for (size_t i = 1; i < contours.size(); i++)
    {
        double area = cv::contourArea(contours[i]);
        double length = cv::arcLength(contours[i], false);
        
        if ((area < 40) || (area > 100)) continue;
      
        Rect rt_t = cv::boundingRect(contours[i]);  //값 가져오기
        //drawContours(fill_img, contours, (int)i, Scalar(0, 0, 255), 1, LINE_8, hierarchy, 0);  //외곽선그리기
        cv::rectangle(plate_image, rt_t, Scalar(255, 255, 255), 1);  //네모그리기


        //contours정보표시
        string msg = std::format("[{}]", i);
        putText(fill_img, msg, Point(rt_t.x+ rect.x, rt_t.y+ rect.y), HersheyFonts::FONT_HERSHEY_SIMPLEX, 0.4, Scalar(255, 255, 255), 1);

        //크기
        int size = rt_t.width * rt_t.height;
        //길이
        int r_length = 2 * (rt_t.width + rt_t.height);

        //크기, 길이 콘솔로 출력
        cout << "크기: " << rt_t.width << " x " << rt_t.height << " = " << rt_t.width * rt_t.height << std::endl;
        cout << "길이: " << r_length << std::endl;

        //크기, 길이 result_img에 표시
        msg = std::format("[{}] - size : {} length : {}", i, size, r_length);
        putText(fill_img, msg, Point(10, 50 + txt_index * 20), HersheyFonts::FONT_HERSHEY_SIMPLEX, 0.4, Scalar(255, 255, 255), 1);
        txt_index++;

        vCharROI.push_back(rt_t);
    }
    
    //결과이미지 파일로 저장
    imwrite("result.png", fill_img);
    //imwrite("plate_image.png", plate_image);
    //imwrite("thresh_plate_image.png", thresh_plate_imge);
    int p = 0;
	return 0;
}