//
//  main.cpp
//  OpenGL1
//
//  Created by 刘峰 on 2020/11/3.
//

#define GL_SILENCE_DEPRECATION

#include "GLTools.h"
#include "GLMatrixStack.h"
#include "GLFrame.h"
#include "GLFrustum.h"
#include "GLGeometryTransform.h"
#include "StopWatch.h"

#include <math.h>
#ifdef __APPLE__
#include <GLUT/GLUT.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif

/// 相机视角
GLFrame viewFrame;
/// 三角形工具类
GLTriangleBatch torusBatch;
GLTriangleBatch sphereBatch;
GLTriangleBatch cylinderBatch; // 圆柱
GLTriangleBatch coneBatch; // 圆锥
GLTriangleBatch diskBatch;


/// 投影管理类
GLFrustum viewFrustum;
/// 模型变换矩阵
GLMatrixStack modelViewMatixStack;
/// 透视变换矩阵
GLMatrixStack projectionMatrixStack;
/// 几何变换管线,用来管理矩阵
GLGeometryTransform transformPipeline;

GLShaderManager shaderManager;

int nStep = 0;

// 设置平移和旋转
void SetTranslateAndRotate(M3DMatrix44f modelViewMatrix, M3DMatrix44f mtranslateMatrix, M3DMatrix44f mRotateMatrix) {
    // 平移
    m3dTranslationMatrix44(mtranslateMatrix, 0, 0.1f, 0);
    // 旋转
    m3dRotationMatrix44(mRotateMatrix, 0.1, 0.f, 0.3f, 0.f);

    // 模型变换,
    // 值得一提的是参数 2 和参数 3 的位置改变,得到的矩阵结果是不同的,在线性代数中叫矩阵叉乘,矩阵叉乘不可逆.
    m3dMatrixMultiply44(modelViewMatrix, mtranslateMatrix, mRotateMatrix);
}

// 自动沿着 z 轴旋转
void AutoRotate(M3DMatrix44f modelViewMatrix, M3DMatrix44f mRotateMatrix) {
    // 计时器
    static CStopWatch rotatedTimer;
    GLfloat angle = rotatedTimer.GetElapsedSeconds();
    m3dRotationMatrix44(mRotateMatrix, angle, 0, 0, 1);
}

void SetRC() {
    // 设置窗口的背景色
    glClearColor(0.996, 0.773, 0.455, 1.0);
    
//    glEnable(GL_DEPTH_TEST);
    
    shaderManager.InitializeStockShaders();
    viewFrame.MoveForward(15.f);
    
    /* 创建一个球体
     * void gltMakeSphere(GLTriangleBatch& sphereBatch, GLfloat fRadius, GLint iSlices, GLint iStacks);
     * @param sphereBatch 用来处理球体的三角形批次类,帮助我们完成球体的绘制
     * @param fRadius 半径长度
     * @param iSlices 球体中切片的个数
     * @param iStacks 球体中三角形的个数
    */
    gltMakeSphere(sphereBatch, 2, 20, 40);
    
    /* 创建一个圆环
     * void gltMakeTorus(GLTriangleBatch& torusBatch, GLfloat majorRadius, GLfloat minorRadius, GLint numMajor, GLint numMinor);
     * @param torusBatch 用来画圆环的三角形批次类
     * @param majorRadius 外环半径
     * @param minorRadius 内环半径
     * @param numMajor 外环面三角形个数
     * @param numMinor 内环面三角形个数
     */
    gltMakeTorus(torusBatch, 1.0, 0.5, 20, 40);
    
    /* 创建一个圆盘,形状类似光盘,是个平面
     * void gltMakeDisk(GLTriangleBatch& diskBatch, GLfloat innerRadius, GLfloat outerRadius, GLint nSlices, GLint nStacks);
     * @param diskBatch 辅助绘制光盘的三角形批次类
     * @param innerRadius 内环半径
     * @param outerRadius 外环半径
     * @param nSlices 圆环边的个数
     * @param nStacks 圆环每一块的三角形个数的 2 倍
     */
    gltMakeDisk(diskBatch, 1.5, 3.0, 13, 6);
    
    /* 创建一个圆柱体
     * void gltMakeCylinder(GLTriangleBatch& cylinderBatch, GLfloat baseRadius, GLfloat topRadius, GLfloat fLength, GLint numSlices, GLint numStacks);
     * @param cylinderBatch 圆柱体的三角形批次类
     * @param baseRadius 圆柱体下面的圆的半径
     * @param topRadius 圆柱体上面的圆的半径
     * @param fLength 圆柱体长度
     * @param numSlices 圆柱体的圆有几个边
     * @param numStacks 圆柱体每列的边有多少个正方形(也是三角形拼的,所以是三角形个数的二分之一)
     */
    gltMakeCylinder(cylinderBatch, 2.0, 2.0, 5, 20, 10);
    
    // 画圆锥,就把上边的圆半径变成 0 即可
    gltMakeCylinder(coneBatch, 2.0, 0, 5, 20, 10);
}

// 给 3D 图形的三角形线绘制成黑色,并配置 四边形偏移,颜色混合,线段抗锯齿
void DrawPolygonScene(GLTriangleBatch &batch, M3DMatrix44f mvpMatrix, GLfloat vColor[]) {
    shaderManager.UseStockShader(GLT_SHADER_FLAT, mvpMatrix, vColor);
    batch.Draw();
    
    // 开启多边形偏移,防止出现 Z-Fighting 现象
    glEnable(GL_POLYGON_OFFSET_LINE);
    // 把一个个三角形画上线
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glPolygonOffset(-1.0, -1.0);
    glLineWidth(2.5f);
    
    // 开启颜色混合
    glEnable(GL_BLEND);
    // 开启线段抗锯齿
    glEnable(GL_LINE_SMOOTH);
    // 设置混合因子
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    GLfloat vBlack[] = {0.f, 0.f, 0.f, 1.f};
    shaderManager.UseStockShader(GLT_SHADER_FLAT, mvpMatrix, vBlack);
    
    batch.Draw();
    
    // 将上面开启的都关闭
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_POLYGON_OFFSET_LINE);
    glDisable(GL_BLEND);
    glDisable(GL_LINE_SMOOTH);
    glLineWidth(1.0f);
}

// 不使用几何变换管道也能实现矩阵变换
void RenderFunc() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    
    // 画球体
    M3DMatrix44f modelViewMatrix, mvpMatrix, mtranslateMatrix, mRotateMatrix;
    
#if 0
    // 下面是平移和旋转操作
    SetTranslateAndRotate(modelViewMatrix, mtranslateMatrix, mRotateMatrix);
#endif
    m3dLoadIdentity44(modelViewMatrix);
    
    AutoRotate(modelViewMatrix, mRotateMatrix);
    
    m3dMatrixMultiply44(modelViewMatrix, modelViewMatrix, mRotateMatrix);
    
    M3DMatrix44f viewMatrix;
    viewFrame.GetMatrix(viewMatrix);
    m3dMatrixMultiply44(modelViewMatrix, modelViewMatrix, viewMatrix);
    
    m3dMatrixMultiply44(mvpMatrix, viewFrustum.GetProjectionMatrix(), modelViewMatrix);
    
    GLfloat vBlack[] = {0.f, 0.f, 0.f, 1.f};
    GLfloat vGreen[] = {0.329, 0.867, 0.569, 1.f};
    shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, modelViewMatrix, viewFrustum.GetProjectionMatrix(), vGreen);
    sphereBatch.Draw();
    
    //5.判断你目前是绘制第几个图形
    switch(nStep) {
        case 0:
            sphereBatch.Draw();
//            DrawPolygonScene(sphereBatch, mvpMatrix, vGreen);
            break;
        case 1:
            torusBatch.Draw();
//            DrawPolygonScene(torusBatch, mvpMatrix, vGreen);
            break;
        case 2:
            DrawPolygonScene(cylinderBatch, mvpMatrix, vGreen);
            break;
        case 3:
            DrawPolygonScene(coneBatch, mvpMatrix, vGreen);
            break;
        case 4:
            DrawPolygonScene(diskBatch, mvpMatrix, vGreen);
            break;
    }
    
    glutSwapBuffers();
    glutPostRedisplay();
}

// 监听空格
void KeyboardPress(unsigned char key, int x, int y) {
    if(key == 32)
    {
        nStep++;
        
        if(nStep > 4)
            nStep = 0;
    }
    
    switch(nStep)
    {
        case 0:
            glutSetWindowTitle("Sphere");
            break;
        case 1:
            glutSetWindowTitle("Torus");
            break;
        case 2:
            glutSetWindowTitle("Cylinder");
            break;
        case 3:
            glutSetWindowTitle("Cone");
            break;
        case 4:
            glutSetWindowTitle("Disk");
            break;
    }
    
    glutPostRedisplay();
}

/// 监听键盘上下左右按键
void SpecialKey(int key, int x, int y) {
    // 这里世界坐标系和物体坐标系是重叠在一起的
    if (key == GLUT_KEY_UP) {
//        viewFrame.RotateLocal(m3dDegToRad(-5.f), 1.f, 0.f, 0.f);
        viewFrame.RotateWorld(m3dDegToRad(-5.f), 1.f, 0.f, 0.f);
    } else if (key == GLUT_KEY_DOWN) {
//        viewFrame.RotateLocal(m3dDegToRad(5.f), 1.f, 0.f, 0.f);
        viewFrame.RotateWorld(m3dDegToRad(5.f), 1.f, 0.f, 0.f);
    } else if (key == GLUT_KEY_LEFT) {
//        viewFrame.RotateLocal(m3dDegToRad(-5.f), 0.f, 1.f, 0.f);
        viewFrame.RotateWorld(m3dDegToRad(-5.f), 0.f, 1.f, 0.f);
    } else if (key == GLUT_KEY_RIGHT) {
//        viewFrame.RotateLocal(m3dDegToRad(5.f), 0.f, 1.f, 0.f);
        viewFrame.RotateWorld(m3dDegToRad(5.f), 0.f, 1.f, 0.f);
    }
    
    glutPostRedisplay();
}

void ChangeSize(int w, int h) {
    if (h == 0) {
        h = 1;
    }
    
    glViewport(0, 0, w, h);
    
    viewFrustum.SetPerspective(35.f, float(w)/float(h), 1.0f, 1000.f);
}

int main (int argc, char *argv[]) {
    gltSetWorkingDirectory(argv[0]);
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA | GLUT_STENCIL);
    glutCreateWindow("Sphere");
    glutInitWindowSize(800, 600);
    glutKeyboardFunc(KeyboardPress);
    glutSpecialFunc(SpecialKey);
    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderFunc);
    
    glewInit();
    
    SetRC();
    
    glutMainLoop();
    return 0;
}
