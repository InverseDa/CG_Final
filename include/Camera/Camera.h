//
// Created by inver on 2022/9/16.
//

#ifndef COMPUTERGRAPHICS_CAMERA_H
#define COMPUTERGRAPHICS_CAMERA_H

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

class Camera{
public:
    //  摄像头的四个向量：位置向量，摄像头坐标系的三个方向向量
    glm::vec3 cameraPos, cameraFront, cameraUp, cameraRight;
    //  欧拉角
    float yaw, pitch;
    //  视角范围
    float fov;
    //  给定摄像头位置向量、摄像头的z轴方向向量、摄像头的y轴方向向量；构造一个摄像头，并生成x轴方向向量
    Camera(glm::vec3 cameraPos, glm::vec3 cameraFront, glm::vec3 cameraUp, float pitch, float yaw, float fov);
    //  鼠标控制欧拉角
    void processMouseMovement(double xOffset, double yOffset);
    //  滚轮控制视角范围
    void processScrollMovement(double yOffset);

    glm::mat4 getView();
    glm::vec3 getCameraPos();
};

#endif //COMPUTERGRAPHICS_CAMERA_H
