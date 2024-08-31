#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

/**
 * @brief Creates mapping matrices for dewarping.
 * 
 * @param src_size Source image size.
 * @param dst_size Destination image size.
 * @param circle_center Center of the fisheye circle in the source image.
 * @param circle_radius Radius of the fisheye circle in the source image.
 * @param theta Rotation angle around the Z-axis (normalized between 0 and 1).
 * @param phi Downward angle (normalized between 0 and 1).
 * @param FOV Horizontal field of view (normalized between 0 and 1).
 * @param map_x Output mapping matrix for X coordinates.
 * @param map_y Output mapping matrix for Y coordinates.
 */
void make_map(Size src_size, Size dst_size, Point2i circle_center, int circle_radius, 
              float theta, float phi, float FOV, Mat &map_x, Mat &map_y) {
    map_x.create(dst_size, CV_32FC1);
    map_y.create(dst_size, CV_32FC1);

    int remap_width = dst_size.width;
    int remap_height = dst_size.height;

    float view_width = CV_PI * FOV;
    float view_height = view_width * (static_cast<float>(remap_height) / remap_width);

    float view_theta = 2 * CV_PI * theta;    // 0 <= theta <= 1
    float view_phi = ((CV_PI / 2) - (view_height / 2)) * phi; // 0 <= phi <= 1

    float aXr = view_width / remap_width;
    float bXr = view_width / 2;
    float aYr = view_height / remap_height;
    float bYr = view_height / 2;

    float sin_view_phi = sin(-view_phi);
    float cos_view_phi = cos(-view_phi);
    float sin_view_theta = sin(-view_theta);
    float cos_view_theta = cos(-view_theta);

    for (int x = 0; x < remap_width; x++) {
        for (int y = 0; y < remap_height; y++) {
            float Xr = x * aXr - bXr;
            float Yr = y * aYr - bYr;

            float Vx = -tan(Yr);
            float Vy = tan(Xr);

            float Vx_ = cos_view_phi * Vx - sin_view_phi;
            float Vy_ = Vy;
            float Vz_ = -cos_view_phi - sin_view_phi * Vx;

            float Vx__ = cos_view_theta * Vx_ - sin_view_theta * Vy_;
            float Vy__ = sin_view_theta * Vx_ + cos_view_theta * Vy_;
            float Vz__ = Vz_;

            float V_theta = atan2(Vy__, Vx__);
            float V_phi = atan2(sqrt(Vx__ * Vx__ + Vy__ * Vy__), (-Vz__));

            float r = (V_phi / (CV_PI / 2)) * circle_radius;
            float Cx = r * cos(V_theta);
            float Cy = r * sin(V_theta);

            map_x.at<float>(y, x) = Cx + circle_center.x;
            map_y.at<float>(y, x) = Cy + circle_center.y;
        }
    }
}

/**
 * @brief Displays usage instructions.
 */
void print_usage(const char* program_name) {
    cout << "Usage: " << program_name << " [OPTIONS]\n"
         << "Options:\n"
         << "  -i, --input       Path to input JPEG image (required)\n"
         << "  -o, --output      Path to output dewarped JPEG image (required)\n"
         << "  -cw, --circle_x    X-coordinate of the circle center (default: 1301)\n"
         << "  -ch, --circle_y    Y-coordinate of the circle center (default: 935)\n"
         << "  -cr, --radius      Circle radius (default: 845)\n"
         << "  -rw, --remap_w     Remap width (default: 1280)\n"
         << "  -rh, --remap_h     Remap height (default: 720)\n"
         << "  -t, --theta        Theta angle (0 to 1, default: 0.125)\n"
         << "  -p, --phi          Phi angle (0 to 1, default: 0.95)\n"
         << "  -f, --fov          Field of view (0 to 1, default: 0.4)\n"
         << "  -h, --help         Display this help message\n";
}

int main(int argc, char** argv) {
    // Default parameters
    string input_path;
    string output_path;
    int circle_x = 1301;
    int circle_y = 935;
    int circle_radius = 845;
    int remap_width = 1280;
    int remap_height = 720;
    float theta = 1.0f / 8.0f;
    float phi = 19.0f / 20.0f;
    float FOV = 2.0f / 5.0f;

    // Parse command-line arguments
    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if ((arg == "-i" || arg == "--input") && i + 1 < argc) {
            input_path = argv[++i];
        }
        else if ((arg == "-o" || arg == "--output") && i + 1 < argc) {
            output_path = argv[++i];
        }
        else if ((arg == "-cw" || arg == "--circle_x") && i + 1 < argc) {
            circle_x = stoi(argv[++i]);
        }
        else if ((arg == "-ch" || arg == "--circle_y") && i + 1 < argc) {
            circle_y = stoi(argv[++i]);
        }
        else if ((arg == "-cr" || arg == "--radius") && i + 1 < argc) {
            circle_radius = stoi(argv[++i]);
        }
        else if ((arg == "-rw" || arg == "--remap_w") && i + 1 < argc) {
            remap_width = stoi(argv[++i]);
        }
        else if ((arg == "-rh" || arg == "--remap_h") && i + 1 < argc) {
            remap_height = stoi(argv[++i]);
        }
        else if ((arg == "-t" || arg == "--theta") && i + 1 < argc) {
            theta = stof(argv[++i]);
            if (theta < 0.0f || theta > 1.0f) {
                cerr << "Theta must be between 0 and 1.\n";
                return -1;
            }
        }
        else if ((arg == "-p" || arg == "--phi") && i + 1 < argc) {
            phi = stof(argv[++i]);
            if (phi < 0.0f || phi > 1.0f) {
                cerr << "Phi must be between 0 and 1.\n";
                return -1;
            }
        }
        else if ((arg == "-f" || arg == "--fov") && i + 1 < argc) {
            FOV = stof(argv[++i]);
            if (FOV < 0.0f || FOV > 1.0f) {
                cerr << "FOV must be between 0 and 1.\n";
                return -1;
            }
        }
        else if (arg == "-h" || arg == "--help") {
            print_usage(argv[0]);
            return 0;
        }
        else {
            cerr << "Unknown option: " << arg << "\n";
            print_usage(argv[0]);
            return -1;
        }
    }

    // Validate required arguments
    if (input_path.empty() || output_path.empty()) {
        cerr << "Input and output paths are required.\n";
        print_usage(argv[0]);
        return -1;
    }

    // Load the input image
    Mat frame = imread(input_path, IMREAD_COLOR);
    if (frame.empty()) {
        cerr << "Failed to load image: " << input_path << "\n";
        return -1;
    }

    Size src_size = frame.size();
    Size dst_size(remap_width, remap_height);
    Point2i circle_center(circle_x, circle_y);

    // Create mapping matrices
    Mat map_x, map_y;
    make_map(src_size, dst_size, circle_center, circle_radius, theta, phi, FOV, map_x, map_y);

    // Apply remapping
    Mat remapped;
    remap(frame, remapped, map_x, map_y, INTER_LINEAR, BORDER_CONSTANT, Scalar(0,0,0));

    // Save the dewarped image
    if (!imwrite(output_path, remapped)) {
        cerr << "Failed to save image: " << output_path << "\n";
        return -1;
    }

    cout << "Dewarped image saved to " << output_path << "\n";
    return 0;
}
