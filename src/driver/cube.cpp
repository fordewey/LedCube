#include "./cube.h"
#include "../utility/image_lib.h"
#include <thread>
#include <chrono>
#include <iostream>
#include <wiringPi.h>

X74hc154 LedCube::x74hc154[4];
int LedCube::vcc[8];
int LedCube::leds[8][8][8];  // Z X Y
bool LedCube::isRunning = true;
bool LedCube::isBackgroundThreadQuit = true;
std::mutex LedCube::mutex_;


LedCube::~LedCube() {
    isRunning = false;
    int count = 5;
    while (!isBackgroundThreadQuit && count-- > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    lightOffAll();
}


void LedCube::lock() {
    mutex_.lock();
}

void LedCube::unlock() {
    mutex_.unlock();
}


bool LedCube::setup() {
    x74hc154[0].setup(17, 27, 22, 5, 6);
    x74hc154[1].setup(17, 27, 22, 5, 13);
    x74hc154[2].setup(17, 27, 22, 5, 19);
    x74hc154[3].setup(17, 27, 22, 5, 26);

    vcc[0] = 18;
    vcc[1] = 23;
    vcc[2] = 24;
    vcc[3] = 25;
    vcc[4] = 12;
    vcc[5] = 16;
    vcc[6] = 20;
    vcc[7] = 21;

    for (int i = 0; i < 8; ++i) {
        pinMode(vcc[i], OUTPUT);
    }

    lightOffAll();

    std::thread t(backgroundThread);
    t.detach();

    return true;
}


void LedCube::lightOffAll() {
    // 74hc154
    // set G1 or G2 to HIGH
    // so all the outputs are HIGH
    for (int i = 0; i < 4; ++i) {
        x74hc154[i].enable(false);
    }

    // VCC
    // set all VCC to LOW
    for (int i = 0; i < 8; ++i) {
        digitalWrite(vcc[i], LOW);
    }

    for (int z = 0; z < 8; ++z) {
        for (int x = 0; x < 8; ++x) {
            for (int y = 0; y < 8; ++y) {
                leds[z][x][y] = LED_OFF;
            }
        }
    }
}


/****************************************
 *
 *   Background Thread
 *      scan all of the cube
 *      light on or light off
 *
** *************************************/
void LedCube::backgroundThread() {
    isBackgroundThreadQuit = false;
    while (isRunning) {
        mutex_.lock();
        for (int z = 0; z < 8; ++z) {
            for (int x = 0; x < 8; ++x) {
                lightRowX(z, x);
            }
        }
        mutex_.unlock();
        std::this_thread::sleep_for(std::chrono::nanoseconds(300));
    }

    isBackgroundThreadQuit = true;
    printf("Background thread quit!\n");
}

void LedCube::lightRowX(int z, int x) {
    int idx = x / 2;
    digitalWrite(vcc[z], HIGH);
    for (int y = 0; y < 8; ++y) {
        //printf("%d %d %d : ", x, y, z);
        if (leds[z][x][y] == LED_ON) {
            x74hc154[idx].setOutput(y + 8 * (x % 2));
            //std::this_thread::sleep_for(std::chrono::nanoseconds(500));
            x74hc154[idx].enable(true);
            std::this_thread::sleep_for(std::chrono::nanoseconds(300));
            x74hc154[idx].enable(false);
        }
    }
    digitalWrite(vcc[z], LOW);
}


/************************************
 *
 *   Set all leds to LED_OFF
 *
** **********************************/
void LedCube::clear() {
    for (int z = 0; z < 8; ++z) {
        for (int x = 0; x < 8; ++x) {
            for (int y = 0; y < 8; ++y) {
                leds[z][x][y] = LED_OFF;
            }
        }
    }
}


/***************************************************
 *
 *  Light on or off a whole layer
 *
***************************************************/
void LedCube::lightLayerX(int x, int state) {
    for (int z = 0; z < 8; ++z) {
        for (int y = 0; y < 8; ++y) {
            leds[z][x][y] = state;
        }
    }
}


void LedCube::lightLayerY(int y, int state) {
    for (int z = 0; z < 8; ++z) {
        for (int x = 0; x < 8; ++x) {
            leds[z][x][y] = state;
        }
    }
}


void LedCube::lightLayerZ(int z, int state) {
    for (int x = 0; x < 8; ++x) {
        for (int y = 0; y < 8; ++y) {
            leds[z][x][y] = state;
        }
    }
}


/***************************************************
 *
 *  Light on or off a row
 *
***************************************************/
void LedCube::lightRowXY(int x, int y, int state) {
    for (int z = 0; z < 8; ++z) {
        leds[z][x][y] = state;
    }
}

void LedCube::lightRowYZ(int y, int z, int state)  {
    for (int x = 0; x < 8; ++x) {
        leds[z][x][y] = state;
    }
}

void LedCube::lightRowXZ(int x, int z, int state) {
    for (int y = 0; y < 8; ++y) {
        leds[z][x][y] = state;
    }
}


/***************************************************
 *
 *  Show Text in a layer
 *
***************************************************/
void LedCube::setImageInLayerZ(unsigned char ch, int z, Direction direction, Angle rotate) {
    Array2D_8_8 image;
    getImageInLayerZ(image, ch, direction, rotate);
    setImageInLayerZ(z, image);
}

void LedCube::setImageInLayerY(unsigned char ch, int y, Direction direction, Angle rotate) {
    Array2D_8_8 image;
    getImageInLayerY(image, ch, direction, rotate);
    setImageInLayerY(y, image);
}

void LedCube::setImageInLayerX(unsigned char ch, int x, Direction direction, Angle rotate) {
    Array2D_8_8 image;
    getImageInLayerX(image, ch, direction, rotate);
    setImageInLayerX(x, image);
}

void LedCube::setImageInLayerZ(int z, const Array2D_8_8& image) {
    for (int x = 0; x < 8; ++x) {
        for (int y = 0; y < 8; ++y) {
            leds[z][x][y] = image[x][y];
        }
    }
}

void LedCube::setImageInLayerY(int y, const Array2D_8_8& image) {
    for (int z = 0; z < 8; ++z) {
        for (int x = 0; x < 8; ++x) {
            leds[z][x][y] = image[z][x];
        }
    }
}

void LedCube::setImageInLayerX(int x, const Array2D_8_8& image) {
    for (int z = 0; z < 8; ++z) {
        for (int y = 0; y < 8; ++y) {
            leds[z][x][y] = image[z][y];
        }
    }
}

void LedCube::getImageInLayerZ(Array2D_8_8& image, unsigned char ch, Direction direction, Angle rotate) {
    auto table = ImageLib::get(ch);

    if (direction == Z_ASCEND) {
        switch (rotate) {
        case ANGLE_0:
            for (int x = 0; x < 8; ++x) {
                for (int y = 0; y < 8; ++y) {
                    image[x][y] = table[x][y]; 
                }
            }
            break;
        case ANGLE_90:
            for (int y = 7; y > -1; --y) {
                for (int x = 0; x < 8; ++x) {
                    image[x][y] = table[7-y][x];
                }
            }
            break;
        case ANGLE_180:
            for (int x = 7; x > -1; --x) {
                for (int y = 7; y > -1; --y) {
                    image[x][y] = table[7-x][7-y];
                }
            }
            break;
        case ANGLE_270:
            for (int y = 0; y < 8; ++y) {
                for (int x = 7; x > -1; --x) {
                    image[x][y] = table[y][7-x];
                }
            }
            break;
        default:
            break;
        }
    }
    else if (direction == Z_DESCEND) {
        switch (rotate) {
        case ANGLE_0:
            for (int x = 0; x < 8; ++x) {
                for (int y = 7; y > -1; --y) {
                    image[x][y] = table[x][7-y]; 
                }
            }
            break;
        case ANGLE_90:
            for (int y = 0; y < 8; ++y) {
                for (int x = 0; x < 8; ++x) {
                    image[x][y] = table[y][x];
                }
            }
            break;
        case ANGLE_180:
            for (int x = 7; x > -1; --x) {
                for (int y = 0; y < 8; ++y) {
                    image[x][y] = table[7-x][y];
                }
            }
            break;
        case ANGLE_270:
            for (int y = 7; y > -1; --y) {
                for (int x = 7; x > -1; --x) {
                    image[x][y] = table[7-y][7-x];
                }
            }
            break;
        default:
            break;
        }
    }
}

void LedCube::getImageInLayerY(Array2D_8_8& image, unsigned char ch, Direction direction, Angle rotate) {
    auto table = ImageLib::get(ch);

    if (direction == Y_ASCEND) {
        switch (rotate) {
        case ANGLE_0:
            for (int z = 7; z > -1; --z) {
                for (int x = 7; x > -1; --x) {
                    image[z][x] = table[7-z][7-x];
                }
            }
            break;
        case ANGLE_90:
            for (int x = 0; x < 8; ++x) {
                for (int z = 7; z > -1; --z) {
                    image[z][x] = table[x][7-z];
                }
            }
            break;
        case ANGLE_180:
            for (int z = 0; z < 8; ++z) {
                for (int x = 0; x < 8; ++x) {
                    image[z][x] = table[z][x];
                }
            }
            break;
        case ANGLE_270:
            for (int x = 7; x > -1; --x) {
                for (int z = 0; z < 8; ++z) {
                    image[z][x] = table[7-x][z];
                }
            }
            break;
        default:
            break;
        }
    }
    else if (direction == Y_DESCEND) {
        switch (rotate) {
        case ANGLE_0:
            for (int z = 7; z > -1; --z) {
                for (int x = 0; x < 8; ++x) {
                    image[z][x] = table[7-z][x];
                }
            }
            break;
        case ANGLE_90:
            for (int x = 7; x > -1; --x) {
                for (int z = 7; z > -1; --z) {
                    image[z][x] = table[7-x][7-z];
                }
            }
            break;
        case ANGLE_180:
            for (int z = 0; z < 8; ++z) {
                for (int x = 7; x > -1; --x) {
                    image[z][x] = table[z][7-x];
                }
            }
            break;
        case ANGLE_270:
            for (int x = 0; x < 8; ++x) {
                for (int z = 0; z < 8; ++z) {
                    image[z][x] = table[x][z];
                }
            }
            break;
        }
    }
}

void LedCube::getImageInLayerX(Array2D_8_8& image, unsigned char ch, Direction direction, Angle rotate) {
    auto table = ImageLib::get(ch);

    if (direction == X_ASCEND) {
        switch (rotate) {
        case ANGLE_0:
            for (int z = 7; z > -1; --z) {
                for (int y = 0; y < 8; ++y) {
                    image[z][y] = table[7-z][y];
                }
            }
            break;
        case ANGLE_90:
            for (int y = 7; y > -1; --y) {
                for (int z = 7; z > -1; --z) {
                    image[z][y] = table[7-y][7-z];
                }
            }
            break;
        case ANGLE_180:
            for (int z = 0; z < 8; ++z) {
                for (int y = 7; y > -1; --y) {
                    image[z][y] = table[z][7-y];
                }
            }
            break;
        case ANGLE_270:
            for (int y = 0; y < 8; ++y) {
                for (int z = 0; z < 8; ++z) {
                    image[z][y] = table[y][z];
                }
            }
            break;
        default:
            break;
        }
    }
    else if (direction == X_DESCEND) {
        switch (rotate) {
        case ANGLE_0:
            for (int z = 7; z > -1; --z) {
                for (int y = 7; y > -1; --y) {
                    image[z][y] = table[7-z][7-y];
                }
            }
            break;
        case ANGLE_90:
            for (int y = 0; y < 8; ++y) {
                for (int z = 7; z > -1; --z) {
                    image[z][y] = table[y][7-z];
                }
            }
            break;
        case ANGLE_180:
            for (int z = 0; z < 8; ++z) {
                for (int y = 0; y < 8; ++y) {
                    image[z][y] = table[z][y];
                }
            }
            break;
        case ANGLE_270:
            for (int y = 7; y > -1; --y) {
                for (int z = 0; z < 8; ++z) {
                    image[z][y] = table[7-y][z];
                }
            }
            break;
        }
    }
}

void LedCube::showStringInLayerZ(std::string str, int interval, int z, Direction direction, Angle rotate) {
    ImageLib::validate(str);
    for (auto ch : str) {
        mutex_.lock();
        setImageInLayerZ(ch, z, direction, rotate);
        mutex_.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(interval));
    }
}

void LedCube::showStringInLayerY(std::string str, int interval, int y, Direction direction, Angle rotate) {
    ImageLib::validate(str);
    for (auto ch : str) {
        mutex_.lock();
        setImageInLayerY(ch, y, direction, rotate);
        mutex_.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(interval));
    }
}

void LedCube::showStringInLayerX(std::string str, int interval, int x, Direction direction, Angle rotate) {
    ImageLib::validate(str);
    for (auto ch : str) {
        mutex_.lock();
        setImageInLayerX(ch, x, direction, rotate);
        mutex_.unlock();
        if (ch == ' ')
            std::this_thread::sleep_for(std::chrono::milliseconds(interval / 2));
        else
            std::this_thread::sleep_for(std::chrono::milliseconds(interval));
    }
}


