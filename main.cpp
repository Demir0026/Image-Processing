#include <iostream>
#include <vector>
#include <algorithm>
#include "Image.h"

int main() {
    Image originalImg;

    if (originalImg.read("mandrill.bmp")) {
        std::cout << "[Information] The original image has been read " << std::endl;


        Image equalizedRGBImg = originalImg;
        equalizedRGBImg.equalizeRGB();
        equalizedRGBImg.write("equalized_rgb.bmp");
        std::cout << "[SUCCESSFUL] RGB-based equalization recorded." << std::endl;


        std::vector<YCbCrPixel> ycbcrData = originalImg.convertToYCbCr();
        std::vector<double> y_channel(ycbcrData.size());
        for (size_t i = 0; i < ycbcrData.size(); ++i) y_channel[i] = ycbcrData[i].y;
        Image::equalizeChannel(y_channel, 255.0);
        for (size_t i = 0; i < ycbcrData.size(); ++i) ycbcrData[i].y = y_channel[i];

        Image equalizedYCbCrImg;
        equalizedYCbCrImg.width = originalImg.width;
        equalizedYCbCrImg.height = originalImg.height;
        equalizedYCbCrImg.convertYCbCrToRGB(ycbcrData);
        equalizedYCbCrImg.write("equalized_ycbcr.bmp");
        std::cout << "[SUCCESSFUL] Ycbcr-based equalization recorded." << std::endl;


        std::vector<HSIPixel> hsiData = originalImg.convertToHSI();
        std::vector<double> i_channel(hsiData.size());
        for (size_t i = 0; i < hsiData.size(); ++i) i_channel[i] = hsiData[i].i;
        Image::equalizeChannel(i_channel, 1.0);
        for (size_t i = 0; i < hsiData.size(); ++i) hsiData[i].i = i_channel[i];

        Image equalizedHSIImg;
        equalizedHSIImg.width = originalImg.width;
        equalizedHSIImg.height = originalImg.height;
        equalizedHSIImg.convertHSIToRGB(hsiData);
        equalizedHSIImg.write("equalized_hsi.bmp");
        std::cout << "[SUCCESSFUL] HSI-based equalization recorded." << std::endl;


        std::cout << "\n--- PERFORMANCE METRICS ---" << std::endl;
        Image::calculateMetrics(originalImg, equalizedRGBImg, "RGB_Direct");
        Image::calculateMetrics(originalImg, equalizedYCbCrImg, "YCbCr_Y_Chan");
        Image::calculateMetrics(originalImg, equalizedHSIImg, "HSI_I_Chan");
    }

    return 0;
}