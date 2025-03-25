#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <iostream>
#include <vector>
#include <cmath>

#include "stb_image.h"
#include "stb_image_write.h"

#define USE_LINEAR_RGB true

struct float3 {
    float x, y, z;
    float3() : x(0), y(0), z(0) {}
    float3(float x, float y, float z) : x(x), y(y), z(z) {}

    // Overload the + operator to add two float3 objects element-wise
    float3 operator+(const float3& other) const {
        return float3(x + other.x, y + other.y, z + other.z);
    }

    float3 operator*(const float scalar) const {
        return float3(x * scalar, y * scalar, z * scalar);
    }

    // Optionally, you can also define a scalar + float3 operator
    float3 operator+(float scalar) const {
        return float3(x + scalar, y + scalar, z + scalar);
    }

    float3 operator-(const float scalar) const {
        return float3(x - scalar, y - scalar, z - scalar);
    }

    float3 operator*(const float3& other) const {
        return float3(x * other.x, y * other.y, z * other.z);
    }

    float3 operator-(const float3& other) const {
        return float3(x - other.x, y - other.y, z - other.z);
    }
};

struct float4 {
    float x, y, z, w;
    float4() : x(0), y(0), z(0), w(0) {}
    float4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
    float3 xyz() { return float3(x, y, z); }

    // Allow setting xyz from float3
    void set_xyz(const float3& vec) {
        x = vec.x;
        y = vec.y;
        z = vec.z;
    }
};

double radiansToDegrees(double radians) {
    const double PI = 3.14159265358979323846;
    return radians * (180.0 / PI);
}

float3 sin(const float3& vec) {
    return float3(std::sin(vec.x), std::sin(vec.y), std::sin(vec.z));
}

// Function to simulate the ? operator on float3 components
float3 select(const float3& a, const float3& b, const float3& condition) {
    return float3(
        condition.x ? a.x : b.x,
        condition.y ? a.y : b.y,
        condition.z ? a.z : b.z
    );
}

float srgbToLinear(int v255) {
    float v = v255 / 255.0f;  // Normalize v255 to a value between 0 and 1
    if (v <= 0.04045f) {
        return v / 12.92f;  // Apply the linear transformation for small values of v
    }
    else {
        return std::pow((v + 0.055f) / 1.055f, 2.4f);  // Apply the non-linear transformation
    }
}

float linearToSrgb(float linear) {
    float srgb;
    if (linear < 0.0031308f) {
        srgb = linear * 12.92f;  // Apply linear scaling for small values
    }
    else {
        srgb = 1.055f * std::pow(linear, 1.0f / 2.4f) - 0.055f;  // Apply the non-linear transformation
    }
    return 255.0f * srgb;  // Scale the result to the range [0, 255]
}

void cl_alien_map(const std::vector<float4>& in,
    std::vector<float4>& out,
    const float3& freq,
    const float3& phaseshift,
    const float3& keep,
    size_t num_elements) {

    // Ensure output vector has the right size
    out.resize(num_elements);

    float3 unit(1.0f, 1.0f, 1.0f);

    // Process each element sequentially (instead of in parallel)
    for (size_t gid = 0; gid < num_elements; gid++) {
        float4 in_v = in[gid];

        // original
        // float3 tmp  = 0.5f * (unit +sin((2.0f * in_v.xyz - unit) * freq.xyz + phaseshift.xyz));

        // Calculate the transformed value
        // (2.0f * in_v.xyz - unit) * freq.xyz + phaseshift.xyz
        float3 inner = (in_v.xyz() * 2.0f) - unit;

        float3 withFreq = inner * freq;
        float3 withPhase = withFreq + phaseshift;
        float3 sinResult = sin(withPhase);
        float3 tmp = (unit + sinResult) * 0.5f;

        // Create output value
        float4 out_v;

        // Use select function to implement the conditional logic
        out_v.set_xyz(select(in_v.xyz(), tmp, keep));
        out_v.w = in_v.w;

        // Save the result
        out[gid] = out_v;
    }
}

std::vector<float4> load_image_to_float4(const std::string& image_path, int& width, int& height, int& channels) {
    // Load the image using stb_image
    unsigned char* image_data = stbi_load(image_path.c_str(), &width, &height, &channels, STBI_rgb_alpha);

    if (!image_data) {
        std::cerr << "Failed to load image: " << image_path << std::endl;
        exit(1);
    }

    // Create a vector of float4 to store image data
    std::vector<float4> image_float4;
    size_t num_pixels = width * height;

    for (size_t i = 0; i < num_pixels; ++i) {
        unsigned char* pixel = image_data + i * 4;
        // Convert to float4, normalize RGB values to [0, 1]
        float r, g, b, a;
        if (USE_LINEAR_RGB)
        {
            r = srgbToLinear(pixel[0]);
            g = srgbToLinear(pixel[1]);
            b = srgbToLinear(pixel[2]);
            a = srgbToLinear(pixel[3]);
        }
        else
        {
            r = pixel[0] / 255.0f;
            g = pixel[1] / 255.0f;
            b = pixel[2] / 255.0f;
            a = pixel[3] / 255.0f;
        }
        image_float4.push_back(float4(r, g, b, a));
    }

    // Free the image data (since it's no longer needed)
    stbi_image_free(image_data);

    return image_float4;
}

void save_image_from_float4(const std::vector<float4>& image_data, const std::string& output_path, int width, int height) {
    // Convert image data back to RGBA format (0-255)
    std::vector<unsigned char> output_image(width * height * 4);

    for (size_t i = 0; i < width * height; ++i) {
        const float4& pixel = image_data[i];

        if (USE_LINEAR_RGB)
        {
            output_image[i * 4] = linearToSrgb(pixel.x);
            output_image[i * 4 + 1] = linearToSrgb(pixel.y);
            output_image[i * 4 + 2] = linearToSrgb(pixel.z);
            output_image[i * 4 + 3] = linearToSrgb(pixel.w);
        }
        else
        {
            output_image[i * 4] = static_cast<unsigned char>(std::min(std::max(pixel.x * 255.0f, 0.0f), 255.0f));
            output_image[i * 4 + 1] = static_cast<unsigned char>(std::min(std::max(pixel.y * 255.0f, 0.0f), 255.0f));
            output_image[i * 4 + 2] = static_cast<unsigned char>(std::min(std::max(pixel.z * 255.0f, 0.0f), 255.0f));
            output_image[i * 4 + 3] = static_cast<unsigned char>(std::min(std::max(pixel.w * 255.0f, 0.0f), 255.0f));
        }
    }

    // Save the image using stb_image_write
    if (!stbi_write_png(output_path.c_str(), width, height, 4, output_image.data(), width * 4)) {
    //if (!stbi_write_png(output_path.c_str(), width, height, 4, output_image.data(), width * 4)) {
        std::cerr << "Failed to save image to " << output_path << std::endl;
        exit(1);
    }
}

int mainee() {
    int width, height, channels;
    std::string input_image_path = "input_image.png";  // Your image path here
    std::string output_image_path = "output_image.png"; // Your output path here

    // Load image into float4 format
    std::vector<float4> image_data = load_image_to_float4(input_image_path, width, height, channels);

    // Frequency and phase shift values
    float3 freq(1.0f, 0.0f, 0.0f);
    float3 phaseshift(0.0f, 0.0f, 0.0f);

    // Keep flag for each component
    float3 keep(0.0f, 0.0f, 0.0f);

    // Prepare the output image vector
    std::vector<float4> output_image_data(image_data.size());

    // Call the function to process the image
    cl_alien_map(image_data, output_image_data, freq, phaseshift, keep, image_data.size());
    
    // Save the processed image to output path
    save_image_from_float4(output_image_data, output_image_path, width, height);

    std::cout << "Image processing complete! Output saved to: " << output_image_path << std::endl;

    return 0;
}
