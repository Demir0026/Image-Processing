#include "Image.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>

const double PI = 3.14159265358979323846;

#pragma pack(push, 1)
struct BMPFileHeader {
    uint16_t file_type{0x4D42};
    uint32_t file_size{0};
    uint16_t reserved1{0};
    uint16_t reserved2{0};
    uint32_t offset_data{0};
};

struct BMPInfoHeader {
    uint32_t size{0};
    int32_t width{0};
    int32_t height{0};
    uint16_t planes{1};
    uint16_t bit_count{0};
    uint32_t compression{0};
    uint32_t size_image{0};
    int32_t x_pixels_per_meter{0};
    int32_t y_pixels_per_meter{0};
    uint32_t colors_used{0};
    uint32_t colors_important{0};
};
#pragma pack(pop)

bool Image::read(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Dosya acilamadi: " << filename << std::endl;
        return false;
    }

    BMPFileHeader file_header;
    BMPInfoHeader info_header;

    file.read(reinterpret_cast<char*>(&file_header), sizeof(file_header));
    file.read(reinterpret_cast<char*>(&info_header), sizeof(info_header));

    width = info_header.width;
    height = info_header.height;

    file.seekg(file_header.offset_data, std::ios::beg);

    int padding = (4 - (width * 3) % 4) % 4;
    data.resize(width * height);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int index = (height - 1 - y) * width + x;
            file.read(reinterpret_cast<char*>(&data[index]), 3);
        }
        file.seekg(padding, std::ios::cur);
    }

    file.close();
    return true;
}

bool Image::write(const std::string& filename) const {
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to create the file " << filename << std::endl;
        return false;
    }

    BMPFileHeader file_header;
    BMPInfoHeader info_header;

    int padding = (4 - (width * 3) % 4) % 4;
    int row_stride = width * 3 + padding;

    file_header.offset_data = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader);
    file_header.file_size = file_header.offset_data + (row_stride * height);

    info_header.size = sizeof(BMPInfoHeader);
    info_header.width = width;
    info_header.height = height;
    info_header.planes = 1;
    info_header.bit_count = 24;
    info_header.compression = 0;
    info_header.size_image = row_stride * height;

    file.write(reinterpret_cast<const char*>(&file_header), sizeof(file_header));
    file.write(reinterpret_cast<const char*>(&info_header), sizeof(info_header));

    uint8_t pad_bytes[3] = {0, 0, 0};
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int index = (height - 1 - y) * width + x;
            file.write(reinterpret_cast<const char*>(&data[index]), 3);
        }
        file.write(reinterpret_cast<const char*>(pad_bytes), padding);
    }

    file.close();
    return true;
}

std::vector<YCbCrPixel> Image::convertToYCbCr() const {
    std::vector<YCbCrPixel> ycbcrData(data.size());
    for (size_t i = 0; i < data.size(); ++i) {
        double r = data[i].r;
        double g = data[i].g;
        double b = data[i].b;

        ycbcrData[i].y  = 0.299 * r + 0.587 * g + 0.114 * b;
        ycbcrData[i].cb = 128.0 - 0.168736 * r - 0.331264 * g + 0.5 * b;
        ycbcrData[i].cr = 128.0 + 0.5 * r - 0.418688 * g - 0.081312 * b;
    }
    return ycbcrData;
}

std::vector<HSIPixel> Image::convertToHSI() const {
    std::vector<HSIPixel> hsiData(data.size());
    for (size_t i = 0; i < data.size(); ++i) {
        double r = data[i].r / 255.0;
        double g = data[i].g / 255.0;
        double b = data[i].b / 255.0;

        double i_val = (r + g + b) / 3.0;
        double min_val = std::min({r, g, b});
        double s_val = (i_val > 0.0) ? (1.0 - (min_val / i_val)) : 0.0;

        double h_val = 0.0;
        double num = 0.5 * ((r - g) + (r - b));
        double den = std::sqrt((r - g) * (r - g) + (r - b) * (g - b));

        if (den != 0.0) {
            double theta = std::acos(num / den);
            h_val = (theta * 180.0) / PI;
            if (b > g) h_val = 360.0 - h_val;
        }

        hsiData[i].h = h_val;
        hsiData[i].s = s_val;
        hsiData[i].i = i_val;
    }
    return hsiData;
}
void Image::equalizeChannel(std::vector<double>& channel, double max_val) {
    int total_pixels = channel.size();
    std::vector<int> histogram(256, 0);

    for (double val : channel) {
        int bin = static_cast<int>(std::round((val / max_val) * 255.0));
        bin = std::clamp(bin, 0, 255);
        histogram[bin]++;
    }

    std::vector<int> cdf(256, 0);
    cdf[0] = histogram[0];
    int cdf_min = cdf[0];

    for (int i = 1; i < 256; ++i) {
        cdf[i] = cdf[i - 1] + histogram[i];
        if (cdf[i] > 0 && cdf_min == 0) cdf_min = cdf[i];
    }
    if (cdf_min == 0) cdf_min = 1;

    for (double& val : channel) {
        int bin = static_cast<int>(std::round((val / max_val) * 255.0));
        bin = std::clamp(bin, 0, 255);
        double equalized_val = std::round(static_cast<double>(cdf[bin] - cdf_min) / (total_pixels - cdf_min) * 255.0);
        val = (equalized_val / 255.0) * max_val;
    }
}

void Image::convertYCbCrToRGB(const std::vector<YCbCrPixel>& ycbcrData) {
    data.resize(ycbcrData.size());
    for (size_t i = 0; i < ycbcrData.size(); ++i) {
        double y = ycbcrData[i].y;
        double cb = ycbcrData[i].cb;
        double cr = ycbcrData[i].cr;

        double r = y + 1.402 * (cr - 128.0);
        double g = y - 0.344136 * (cb - 128.0) - 0.714136 * (cr - 128.0);
        double b = y + 1.772 * (cb - 128.0);

        data[i].r = static_cast<uint8_t>(std::clamp(std::round(r), 0.0, 255.0));
        data[i].g = static_cast<uint8_t>(std::clamp(std::round(g), 0.0, 255.0));
        data[i].b = static_cast<uint8_t>(std::clamp(std::round(b), 0.0, 255.0));
    }
}

void Image::convertHSIToRGB(const std::vector<HSIPixel>& hsiData) {
    data.resize(hsiData.size());
    for (size_t i = 0; i < hsiData.size(); ++i) {
        double h = hsiData[i].h;
        double s = hsiData[i].s;
        double i_val = hsiData[i].i;

        double r = 0, g = 0, b = 0;

        if (s == 0) {
            r = g = b = i_val;
        } else {
            if (h >= 0 && h < 120.0) {
                double h_rad = h * PI / 180.0;
                b = i_val * (1.0 - s);
                r = i_val * (1.0 + (s * std::cos(h_rad)) / std::cos(60.0 * PI / 180.0 - h_rad));
                g = 3.0 * i_val - (r + b);
            } else if (h >= 120.0 && h < 240.0) {
                double h_rad = (h - 120.0) * PI / 180.0;
                r = i_val * (1.0 - s);
                g = i_val * (1.0 + (s * std::cos(h_rad)) / std::cos(60.0 * PI / 180.0 - h_rad));
                b = 3.0 * i_val - (r + g);
            } else if (h >= 240.0 && h <= 360.0) {
                double h_rad = (h - 240.0) * PI / 180.0;
                g = i_val * (1.0 - s);
                b = i_val * (1.0 + (s * std::cos(h_rad)) / std::cos(60.0 * PI / 180.0 - h_rad));
                r = 3.0 * i_val - (g + b);
            }
        }

        data[i].r = static_cast<uint8_t>(std::clamp(std::round(r * 255.0), 0.0, 255.0));
        data[i].g = static_cast<uint8_t>(std::clamp(std::round(g * 255.0), 0.0, 255.0));
        data[i].b = static_cast<uint8_t>(std::clamp(std::round(b * 255.0), 0.0, 255.0));
    }
}
void Image::equalizeRGB() {
    std::vector<double> r_channel(data.size());
    std::vector<double> g_channel(data.size());
    std::vector<double> b_channel(data.size());

    for (size_t i = 0; i < data.size(); ++i) {
        r_channel[i] = data[i].r;
        g_channel[i] = data[i].g;
        b_channel[i] = data[i].b;
    }

    equalizeChannel(r_channel, 255.0);
    equalizeChannel(g_channel, 255.0);
    equalizeChannel(b_channel, 255.0);

    for (size_t i = 0; i < data.size(); ++i) {
        data[i].r = static_cast<uint8_t>(std::clamp(r_channel[i], 0.0, 255.0));
        data[i].g = static_cast<uint8_t>(std::clamp(g_channel[i], 0.0, 255.0));
        data[i].b = static_cast<uint8_t>(std::clamp(b_channel[i], 0.0, 255.0));
    }
}

void Image::calculateMetrics(const Image& original, const Image& processed, const std::string& method_name) {
    double mse = 0.0;
    double signal_power = 0.0;
    double noise_power = 0.0;
    int total_pixels = original.width * original.height;

    for (size_t i = 0; i < original.data.size(); ++i) {
        double r_diff = static_cast<double>(original.data[i].r) - processed.data[i].r;
        double g_diff = static_cast<double>(original.data[i].g) - processed.data[i].g;
        double b_diff = static_cast<double>(original.data[i].b) - processed.data[i].b;

        double squared_error = r_diff * r_diff + g_diff * g_diff + b_diff * b_diff;
        mse += squared_error;
        noise_power += squared_error;

        signal_power += (original.data[i].r * original.data[i].r) +
                        (original.data[i].g * original.data[i].g) +
                        (original.data[i].b * original.data[i].b);
    }

    mse /= (total_pixels * 3.0);

    double psnr = (mse > 0.0) ? (10.0 * std::log10((255.0 * 255.0) / mse)) : 99.99;
    double snr = (noise_power > 0.0) ? (10.0 * std::log10(signal_power / noise_power)) : 99.99;

    std::cout << "[" << method_name << "] MSE: " << mse << " | PSNR: " << psnr << " dB | SNR: " << snr << " dB" << std::endl;
}