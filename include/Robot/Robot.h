//
// Created by 缪克达 on 2022/12/26.
//

#ifndef CG_FINAL_ROBOT_H
#define CG_FINAL_ROBOT_H

#include "Robot/TriMesh.h"

struct openGLObject {
    // 顶点数组对象
    GLuint vao;
    // 顶点缓存对象
    GLuint vbo;

    // 着色器程序
    GLuint program;
    // 着色器文件
    std::string vshader;
    std::string fshader;
    // 着色器变量
    GLuint pLocation;
    GLuint cLocation;
    GLuint nLocation;

    // 投影变换变量
    GLuint modelLocation;
    GLuint viewLocation;
    GLuint projectionLocation;

    // 阴影变量
    GLuint shadowLocation;
};

class MatrixStack {
    int _index;
    int _size;
    glm::mat4 *_matrices;

public:
    MatrixStack(int numMatrices = 100) : _index(0), _size(numMatrices) { _matrices = new glm::mat4[numMatrices]; }

    ~MatrixStack() { delete[]_matrices; }

    void push(const glm::mat4 &m) {
        assert(_index + 1 < _size);
        _matrices[_index++] = m;
    }

    glm::mat4 &pop() {
        assert(_index - 1 >= 0);
        _index--;
        return _matrices[_index];
    }
};

struct Robot {
    // 关节大小
    float TORSO_HEIGHT = 4.0;
    float TORSO_WIDTH = 2.5;
    float UPPER_ARM_HEIGHT = 2.5;
    float LOWER_ARM_HEIGHT = 1.8;
    float UPPER_ARM_WIDTH = 0.8;
    float LOWER_ARM_WIDTH = 0.5;
    float UPPER_LEG_HEIGHT = 2.8;
    float LOWER_LEG_HEIGHT = 2.2;
    float UPPER_LEG_WIDTH = 1.0;
    float LOWER_LEG_WIDTH = 0.5;
    float HEAD_HEIGHT = 1.8;
    float HEAD_WIDTH = 1.5;

    // 关节角和菜单选项值
    enum {
        Torso,            // 躯干
        Head,            // 头部
        RightUpperArm,    // 右大臂
        RightLowerArm,    // 右小臂
        LeftUpperArm,    // 左大臂
        LeftLowerArm,    // 左小臂
        RightUpperLeg,    // 右大腿
        RightLowerLeg,    // 右小腿
        LeftUpperLeg,    // 左大腿
        LeftLowerLeg,    // 左小腿
    };

    // 关节角大小
    GLfloat theta[10] = {
            0.0,    // Torso
            0.0,    // Head
            0.0,    // RightUpperArm
            0.0,    // RightLowerArm
            0.0,    // LeftUpperArm
            0.0,    // LeftLowerArm
            0.0,    // RightUpperLeg
            0.0,    // RightLowerLeg
            0.0,    // LeftUpperLeg
            0.0     // LeftLowerLeg
    };
} robot;

TriMesh *Torso = new TriMesh();
TriMesh *Head = new TriMesh();
TriMesh *RightUpperArm = new TriMesh();
TriMesh *RightLowerArm = new TriMesh();
TriMesh *LeftUpperArm = new TriMesh();
TriMesh *LeftLowerArm = new TriMesh();
TriMesh *RightUpperLeg = new TriMesh();
TriMesh *RightLowerLeg = new TriMesh();
TriMesh *LeftUpperLeg = new TriMesh();
TriMesh *LeftLowerLeg = new TriMesh();

openGLObject TorsoObject;
openGLObject HeadObject;
openGLObject RightUpperArmObject;
openGLObject RightLowerArmObject;
openGLObject LeftUpperArmObject;
openGLObject LeftLowerArmObject;
openGLObject RightUpperLegObject;
openGLObject RightLowerLegObject;
openGLObject LeftUpperLegObject;
openGLObject LeftLowerLegObject;

void drawMesh(glm::mat4 modelMatrix, TriMesh *mesh, openGLObject object, Camera camera, glm::mat4 projection) {

    glBindVertexArray(object.vao);

    glUseProgram(object.program);

    // 父节点矩阵 * 本节点局部变换矩阵
    glUniformMatrix4fv(object.modelLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));
    glUniformMatrix4fv(object.viewLocation, 1, GL_FALSE, glm::value_ptr(camera.getView()));
    glUniformMatrix4fv(object.projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));
    // 绘制
    glDrawArrays(GL_TRIANGLES, 0, mesh->getPoints().size());
}

void bindObjectAndData(TriMesh *mesh, openGLObject &object, Shader shader) {

    // 创建顶点数组对象
    glGenVertexArrays(1, &object.vao);    // 分配1个顶点数组对象
    glBindVertexArray(object.vao);    // 绑定顶点数组对象

    // 创建并初始化顶点缓存对象
    glGenBuffers(1, &object.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, object.vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 (mesh->getPoints().size() + mesh->getColors().size() + mesh->getNormals().size()) * sizeof(glm::vec3),
                 NULL,
                 GL_STATIC_DRAW);

    glBufferSubData(GL_ARRAY_BUFFER, 0, mesh->getPoints().size() * sizeof(glm::vec3), &mesh->getPoints()[0]);
    glBufferSubData(GL_ARRAY_BUFFER, mesh->getPoints().size() * sizeof(glm::vec3),
                    mesh->getColors().size() * sizeof(glm::vec3), &mesh->getColors()[0]);
    glBufferSubData(GL_ARRAY_BUFFER, (mesh->getPoints().size() + mesh->getColors().size()) * sizeof(glm::vec3),
                    mesh->getNormals().size() * sizeof(glm::vec3), &mesh->getNormals()[0]);

    object.program = shader.id;

    // 从顶点着色器中初始化顶点的坐标
    object.pLocation = glGetAttribLocation(object.program, "vPosition");
    glEnableVertexAttribArray(object.pLocation);
    glVertexAttribPointer(object.pLocation, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    // 从顶点着色器中初始化顶点的颜色
    object.cLocation = glGetAttribLocation(object.program, "vColor");
    glEnableVertexAttribArray(object.cLocation);
    glVertexAttribPointer(object.cLocation, 3, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(mesh->getPoints().size() * sizeof(glm::vec3)));

    // 从顶点着色器中初始化顶点的法向量
    object.nLocation = glGetAttribLocation(object.program, "vNormal");
    glEnableVertexAttribArray(object.nLocation);
    glVertexAttribPointer(object.nLocation, 3,
                          GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET((mesh->getPoints().size() + mesh->getColors().size()) * sizeof(glm::vec3)));


    // 获得矩阵位置
    object.modelLocation = glGetUniformLocation(object.program, "model");
    object.viewLocation = glGetUniformLocation(object.program, "view");
    object.projectionLocation = glGetUniformLocation(object.program, "projection");
}

// 躯体
void torso(glm::mat4 modelMatrix, Camera camera, glm::mat4 projection) {
    // 本节点局部变换矩阵
    glm::mat4 instance = glm::mat4(1.0);
    instance = glm::translate(instance, glm::vec3(0.0, 0.5 * robot.TORSO_HEIGHT, 0.0));
    instance = glm::scale(instance, glm::vec3(robot.TORSO_WIDTH, robot.TORSO_HEIGHT, robot.TORSO_WIDTH));

    // 乘以来自父物体的模型变换矩阵，绘制当前物体
    drawMesh(modelMatrix * instance, Torso, TorsoObject, camera, projection);
}

// 头部
void head(glm::mat4 modelMatrix, Camera camera, glm::mat4 projection) {
    // 本节点局部变换矩阵
    glm::mat4 instance = glm::mat4(1.0);
    instance = glm::translate(instance, glm::vec3(0.0, 0.5 * robot.HEAD_HEIGHT, 0.0));
    instance = glm::scale(instance, glm::vec3(robot.HEAD_WIDTH, robot.HEAD_HEIGHT, robot.HEAD_WIDTH));

    // 乘以来自父物体的模型变换矩阵，绘制当前物体
    drawMesh(modelMatrix * instance, Head, HeadObject, camera, projection);
}


// 左大臂
void left_upper_arm(glm::mat4 modelMatrix, Camera camera, glm::mat4 projection) {
    // 本节点局部变换矩阵
    glm::mat4 instance = glm::mat4(1.0);
    instance = glm::translate(instance, glm::vec3(0.0, -0.5 * robot.UPPER_ARM_HEIGHT, 0.0));
    instance = glm::scale(instance, glm::vec3(robot.UPPER_ARM_WIDTH, robot.UPPER_ARM_HEIGHT, robot.UPPER_ARM_WIDTH));

    // 乘以来自父物体的模型变换矩阵，绘制当前物体
    drawMesh(modelMatrix * instance, LeftUpperArm, LeftUpperArmObject, camera, projection);
}


void left_lower_arm(glm::mat4 modelMatrix, Camera camera, glm::mat4 projection) {
    glm::mat4 instance = glm::mat4(1.0);
    instance = glm::translate(instance, glm::vec3(0.0, -0.5 * robot.LOWER_ARM_HEIGHT, 0.0));
    instance = glm::scale(instance, glm::vec3(robot.LOWER_ARM_WIDTH, robot.LOWER_ARM_HEIGHT, robot.LOWER_ARM_WIDTH));

    // 乘以来自父物体的模型变换矩阵，绘制当前物体
    drawMesh(modelMatrix * instance, LeftLowerArm, LeftLowerArmObject, camera, projection);
}

void right_upper_arm(glm::mat4 modelMatrix, Camera camera, glm::mat4 projection) {
    glm::mat4 instance = glm::mat4(1.0);
    instance = glm::translate(instance, glm::vec3(0.0, -0.5 * robot.UPPER_ARM_HEIGHT, 0.0));
    instance = glm::scale(instance, glm::vec3(robot.UPPER_ARM_WIDTH, robot.UPPER_ARM_HEIGHT, robot.UPPER_ARM_WIDTH));

    // 乘以来自父物体的模型变换矩阵，绘制当前物体
    drawMesh(modelMatrix * instance, RightUpperArm, RightUpperArmObject, camera, projection);
}

void right_lower_arm(glm::mat4 modelMatrix, Camera camera, glm::mat4 projection) {
    glm::mat4 instance = glm::mat4(1.0);
    instance = glm::translate(instance, glm::vec3(0.0, -0.5 * robot.LOWER_ARM_HEIGHT, 0.0));
    instance = glm::scale(instance, glm::vec3(robot.LOWER_ARM_WIDTH, robot.LOWER_ARM_HEIGHT, robot.LOWER_ARM_WIDTH));

    // 乘以来自父物体的模型变换矩阵，绘制当前物体
    drawMesh(modelMatrix * instance, RightLowerArm, RightLowerArmObject, camera, projection);
}

void left_upper_leg(glm::mat4 modelMatrix, Camera camera, glm::mat4 projection) {
    glm::mat4 instance = glm::mat4(1.0);
    instance = glm::translate(instance, glm::vec3(0.0, -0.5 * robot.UPPER_LEG_HEIGHT, 0.0));
    instance = glm::scale(instance, glm::vec3(robot.UPPER_LEG_WIDTH, robot.UPPER_LEG_HEIGHT, robot.UPPER_LEG_WIDTH));

    // 乘以来自父物体的模型变换矩阵，绘制当前物体
    drawMesh(modelMatrix * instance, LeftUpperLeg, LeftUpperLegObject, camera, projection);
}

void left_lower_leg(glm::mat4 modelMatrix, Camera camera, glm::mat4 projection) {
    glm::mat4 instance = glm::mat4(1.0);
    instance = glm::translate(instance, glm::vec3(0.0, -0.5 * robot.LOWER_LEG_HEIGHT, 0.0));
    instance = glm::scale(instance, glm::vec3(robot.LOWER_LEG_WIDTH, robot.LOWER_LEG_HEIGHT, robot.LOWER_LEG_WIDTH));

    // 乘以来自父物体的模型变换矩阵，绘制当前物体
    drawMesh(modelMatrix * instance, LeftLowerLeg, LeftLowerLegObject, camera, projection);
}

void right_upper_leg(glm::mat4 modelMatrix, Camera camera, glm::mat4 projection) {
    glm::mat4 instance = glm::mat4(1.0);
    instance = glm::translate(instance, glm::vec3(0.0, -0.5 * robot.UPPER_LEG_HEIGHT, 0.0));
    instance = glm::scale(instance, glm::vec3(robot.UPPER_LEG_WIDTH, robot.UPPER_LEG_HEIGHT, robot.UPPER_LEG_WIDTH));

    // 乘以来自父物体的模型变换矩阵，绘制当前物体
    drawMesh(modelMatrix * instance, RightUpperLeg, RightUpperLegObject, camera, projection);
}

void right_lower_leg(glm::mat4 modelMatrix, Camera camera, glm::mat4 projection) {
    glm::mat4 instance = glm::mat4(1.0);
    instance = glm::translate(instance, glm::vec3(0.0, -0.5 * robot.LOWER_LEG_HEIGHT, 0.0));
    instance = glm::scale(instance, glm::vec3(robot.LOWER_LEG_WIDTH, robot.LOWER_LEG_HEIGHT, robot.LOWER_LEG_WIDTH));

    // 乘以来自父物体的模型变换矩阵，绘制当前物体
    drawMesh(modelMatrix * instance, RightLowerLeg, RightLowerLegObject, camera, projection);
}

#endif //CG_FINAL_ROBOT_H
