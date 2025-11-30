#include <jni.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <android/log.h>

#define LOG_TAG "serial_port_native"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static speed_t getBaudrate(int baud) {
    switch (baud) {
        case 0: return B0;
        case 50: return B50;
        case 75: return B75;
        case 110: return B110;
        case 134: return B134;
        case 150: return B150;
        case 200: return B200;
        case 300: return B300;
        case 600: return B600;
        case 1200: return B1200;
        case 1800: return B1800;
        case 2400: return B2400;
        case 4800: return B4800;
        case 9600: return B9600;
        case 19200: return B19200;
        case 38400: return B38400;
        case 57600: return B57600;
        case 115200: return B115200;
        case 230400: return B230400;
        default: return B9600;
    }
}

JNIEXPORT jobject JNICALL
Java_com_example_serialportscannerjunsun_SerialPort_open(JNIEnv *env, jclass thiz, jstring path, jint baudrate, jint flags) {
    const char *path_utf = (*env)->GetStringUTFChars(env, path, NULL);
    int fd = open(path_utf, O_RDWR | O_NOCTTY | O_NONBLOCK);
    (*env)->ReleaseStringUTFChars(env, path, path_utf);
    if (fd == -1) {
        LOGE("Cannot open port: %s", strerror(errno));
        return NULL;
    }

    // configure
    struct termios cfg;
    if (tcgetattr(fd, &cfg)) {
        LOGE("tcgetattr() failed");
        close(fd);
        return NULL;
    }

    cfmakeraw(&cfg);
    speed_t speed = getBaudrate(baudrate);
    cfsetispeed(&cfg, speed);
    cfsetospeed(&cfg, speed);

    cfg.c_cflag |= CLOCAL | CREAD;
    cfg.c_cflag &= ~CSTOPB;
    cfg.c_cflag &= ~CSIZE;
    cfg.c_cflag |= CS8;

    if (tcsetattr(fd, TCSANOW, &cfg) != 0) {
        LOGE("tcsetattr failed");
        close(fd);
        return NULL;
    }

    // make FileDescriptor
    jclass fdClass = (*env)->FindClass(env, "java/io/FileDescriptor");
    jmethodID fdInit = (*env)->GetMethodID(env, fdClass, "<init>", "()V");
    jobject fdObj = (*env)->NewObject(env, fdClass, fdInit);

    jfieldID descriptorField = (*env)->GetFieldID(env, fdClass, "descriptor", "I");
    (*env)->SetIntField(env, fdObj, descriptorField, fd);

    // FileInputStream and FileOutputStream
    jclass fisClass = (*env)->FindClass(env, "java/io/FileInputStream");
    jmethodID fisInit = (*env)->GetMethodID(env, fisClass, "<init>", "(Ljava/io/FileDescriptor;)V");
    jobject fisObj = (*env)->NewObject(env, fisClass, fisInit, fdObj);

    jclass fosClass = (*env)->FindClass(env, "java/io/FileOutputStream");
    jmethodID fosInit = (*env)->GetMethodID(env, fosClass, "<init>", "(Ljava/io/FileDescriptor;)V");
    jobject fosObj = (*env)->NewObject(env, fosClass, fosInit, fdObj);

    // Construct SerialPort Java object
    jclass serialPortClass = (*env)->FindClass(env, "com/example/serialportscannerjunsun/SerialPort");
    jmethodID constructor = (*env)->GetMethodID(env, serialPortClass, "<init>", "(Ljava/io/FileDescriptor;Ljava/io/FileInputStream;Ljava/io/FileOutputStream;)V");
    if (!constructor) {
        // fallback: try to find constructor with different signature (rare)
        LOGE("Constructor not found");
        close(fd);
        return NULL;
    }

    jobject serialPortObj = (*env)->NewObject(env, serialPortClass, constructor, fdObj, fisObj, fosObj);
    return serialPortObj;
}

JNIEXPORT void JNICALL Java_com_example_serialportscannerjunsun_SerialPort_close(JNIEnv *env, jclass clazz, jobject fileDescriptor) {
    jclass fdClass = (*env)->GetObjectClass(env, fileDescriptor);
    jfieldID descriptorField = (*env)->GetFieldID(env, fdClass, "descriptor", "I");
    jint fd = (*env)->GetIntField(env, fileDescriptor, descriptorField);
    close(fd);
}
