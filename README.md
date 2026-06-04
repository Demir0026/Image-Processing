# Color Image Enhancement and Quality Evaluation

## Overview
This project implements color image enhancement techniques using histogram equalization across various color models (RGB, HSI, and YCbCr). The algorithms are developed entirely in C++ from scratch at the pixel level, operating without the use of external computer vision libraries such as OpenCV. 

## Features
* **Custom BMP I/O:** Manual parsing and writing of 24-bit RGB `.bmp` headers and pixel data, including memory alignment (padding) handling.
* **Color Space Transformations:** Mathematical conversions between RGB, YCbCr, and HSI color models.
* **Histogram Equalization:** Extraction of Probability Density Function (PDF) and Cumulative Distribution Function (CDF) to maximize contrast.
* **Objective Quality Assessment:** Calculation of Mean Squared Error (MSE), Peak Signal-to-Noise Ratio (PSNR), and Signal-to-Noise Ratio (SNR) to mathematically evaluate image reconstruction quality.

## Methodology
The histogram equalization process is executed through two distinct approaches to analyze the impact on structural and visual quality:
1. **Direct RGB Equalization:** The R, G, and B channels are separated and equalized independently. 
2. **Luminance-Based Equalization:** The image is converted to YCbCr and HSI spaces. Equalization is applied exclusively to the luminance/intensity components (Y channel in YCbCr, I channel in HSI) while preserving the original chrominance and saturation data. The image is then reconstructed back into the RGB space.

## Performance Evaluation
The objective quality of the enhanced images is measured against the original image using the following formula for PSNR:

`PSNR = 10 * log10(MAX^2 / MSE)`

*(Where MAX is the maximum possible pixel value, e.g., 255)*

**Results for `mandrill.bmp`:**

| Applied Method | MSE (Mean Squared Error) | PSNR (dB) | SNR (dB) |
| :--- | :--- | :--- | :--- |
| **RGB Channels (Direct)** | 578.831 | 20.5053 | 15.1958 |
| **YCbCr (Y Channel)** | 995.840 | 18.1489 | 12.8394 |
| **HSI (I Channel)** | 934.502 | 18.4250 | 13.1155 |

## Discussion
While direct RGB equalization yields the highest PSNR and lowest MSE, it disrupts the original color balance, resulting in false colors and severe artifacts. Conversely, equalizing only the luminance channels in YCbCr and HSI spaces yields lower PSNR values but preserves color integrity. This demonstrates that structural similarity metrics do not always correlate directly with perceived visual quality. The YCbCr and HSI methods provide universally superior visual results for color image enhancement.

## Build and Execution
This project utilizes **CMake** for build configuration.

### Prerequisites
* A standard C++17 (or higher) compiler
* CMake (Version 3.10+)

### Compilation Steps
1. Clone the repository:
```bash
   git clone [https://github.com/YOUR_USERNAME/Color-Image-Enhancement.git](https://github.com/YOUR_USERNAME/Color-Image-Enhancement.git)
   cd Color-Image-Enhancement
