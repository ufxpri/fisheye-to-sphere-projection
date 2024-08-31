# Video Dewarping Tool

이 프로젝트는 어안렌즈로 촬영된 영상을 평활화 하는 툴 입니다. 이미지를 입력으로 받아서 주어진 해상도로 변환하여 저장합니다.

## Features

- 어안 영상 평활화
- 어안렌즈 중심, 지름, 사이즈 옵션 조절 가능

## Requirements

- C++17 이상
- OpenCV 4.x 이상

## Installation

1. **레포지토리 복사:**

    ```sh
    git clone https://github.com/ufxpri/fisheye-to-sphere-projection.git
    cd fisheye-to-sphere-projection
    ```

2. **의존성 설치:**

   OpenCV가 설치되어 있는지 확인해주세요. 직접 빌드한 파일또한 사용 가능합니다.

   For Ubuntu:
   
   ```sh
   sudo apt-get update
   sudo apt-get install libopencv-dev
   ```

   For Windows
   사전 빌드된 파일을 설치하여 사용 가능합니다. [OpenCV 공식 사이트](https://opencv.org/).

3. **프로젝트 빌드:**

   CMake 사용시:

   ```sh
   mkdir build
   cd build
   cmake ..
   make
   ```

## Usage

빌드 후에 다음과 같이 사용할 수 있습니다:

```sh
./dewarp_image.exe -i fisheye.jpg -o flat.jpg
```

## License

This project is licensed under the MIT License - see the [LICENSE](./LICENSE) file for details.