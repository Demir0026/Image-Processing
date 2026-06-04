#pragma once
#include <vector>
#include <cstdint>
#include <string>

struct Pixel {
    uint8_t b, g, r;
};

struct YCbCrPixel {
    double y, cb, cr;
};

struct HSIPixel {
    double h, s, i;
};

class Image {
public:
    int width{0};
    int height{0};
    std::vector<Pixel> data;

    bool read(const std::string& filename);
    bool write(const std::string& filename) const;
    std::vector<YCbCrPixel> convertToYCbCr() const;
    std::vector<HSIPixel> convertToHSI() const;
    static void equalizeChannel(std::vector<double>& channel, double max_val);
    void convertYCbCrToRGB(const std::vector<YCbCrPixel>& ycbcrData);
    void convertHSIToRGB(const std::vector<HSIPixel>& hsiData);

    void equalizeRGB();
    static void calculateMetrics(const Image& original, const Image& processed, const std::string& method_name);
};