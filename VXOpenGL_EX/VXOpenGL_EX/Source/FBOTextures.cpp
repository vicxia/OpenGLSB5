#include <stdio.h>
#include <iostream>

#include <GLTools.h>
#include <GLShaderManager.h>
#include <GLFrustum.h>
#include <GLBatch.h>
#include <GLMatrixStack.h>
#include <GLGeometryTransform.h>
#include <StopWatch.h>

#include <GLUT/GLUT.h>

static GLfloat vGreen[] = { 0.0f, 1.0f, 0.0f, 1.0f };
static GLfloat vBlue[] = { 0.0f, 0.0f, 1.0f, 1.0f };
static GLfloat vWhite[] = { 1.0f, 1.0f, 1.0f, 1.0f };
static GLfloat vBlack[] = { 0.0f, 0.0f, 0.0f, 1.0f };
static GLfloat vGrey[] =  { 0.5f, 0.5f, 0.5f, 1.0f };
static GLfloat vLightPos[] = { -2.0f, 3.0f, -2.0f, 1.0f };
static const GLenum windowBuff[] = { GL_BACK_LEFT };
static const GLenum fboBuffs[] = { GL_COLOR_ATTACHMENT0};
static GLint mirrorTexWidth  = 800;
static GLint mirrorTexHeight = 800;

GLsizei     screenWidth;            // Desired window or desktop width
GLsizei  screenHeight;            // Desired window or desktop height

GLboolean bFullScreen;            // Request to run full screen
GLboolean bAnimated;            // Request for continual updates


GLShaderManager        shaderManager;            // Shader Manager
GLMatrixStack        modelViewMatrix;        // Modelview Matrix
GLMatrixStack        projectionMatrix;        // Projection Matrix
GLFrustum            viewFrustum;            // View Frustum
GLGeometryTransform    transformPipeline;        // Geometry Transform Pipeline
GLFrame                cameraFrame;            // Camera frame
GLFrame                mirrorFrame;            // Mirror frame

GLTriangleBatch        torusBatch;
GLTriangleBatch        sphereBatch;
GLTriangleBatch        cylinderBatch;
GLBatch                floorBatch;
GLBatch                mirrorBatch;
GLBatch                mirrorBorderBatch;

GLuint              fboName;
GLuint                textures[1];
GLuint                mirrorTexture;
GLuint              depthBufferName;

void DrawWorld(GLfloat yRot);
bool LoadBMPTexture(const char *szFileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode);


///////////////////////////////////////////////////////////////////////////////////////////////////////
// Load in a BMP file as a texture. Allows specification of the filters and the wrap mode
bool LoadBMPTexture(const char *szFileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode)
{
    GLbyte *pBits;
    GLint iWidth, iHeight;
    
    pBits = gltReadBMPBits(szFileName, &iWidth, &iHeight);
    if(pBits == NULL)
        return false;
    
    // Set Wrap modes
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, iWidth, iHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, pBits);
    
    // Do I need to generate mipmaps?
    if(minFilter == GL_LINEAR_MIPMAP_LINEAR || minFilter == GL_LINEAR_MIPMAP_NEAREST || minFilter == GL_NEAREST_MIPMAP_LINEAR || minFilter == GL_NEAREST_MIPMAP_NEAREST)
        glGenerateMipmap(GL_TEXTURE_2D);
    
    return true;
}

void SetupRC()
{
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        fprintf(stderr, "Error: %s", glewGetErrorString(err));
    }
    
    shaderManager.InitializeStockShaders();
    glEnable(GL_DEPTH_TEST);
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    gltMakeSphere(sphereBatch, 0.1f, 26, 13);
    gltMakeTorus(torusBatch, 0.4f, 0.15f, 35, 35);
    gltMakeCylinder(cylinderBatch, 0.3f, 0.2f, 1.0f, 10, 10);
    
    GLfloat alpha = 0.25f;
    
    floorBatch.Begin(GL_TRIANGLE_FAN, 4, 1);
    floorBatch.Color4f(0.0f, 1.0f, 0.0f, alpha);
    floorBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
    floorBatch.Normal3f(0.0, 1.0f, 0.0f);
    floorBatch.Vertex3f(-20.0f, -0.41f, 20.0f);
    
    floorBatch.Color4f(0.0f, 1.0f, 0.0f, alpha);
    floorBatch.MultiTexCoord2f(0, 10.0f, 0.0f);
    floorBatch.Normal3f(0.0, 1.0f, 0.0f);
    floorBatch.Vertex3f(20.0f, -0.41f, 20.0f);
    
    floorBatch.Color4f(0.0f, 1.0f, 0.0f, alpha);
    floorBatch.MultiTexCoord2f(0, 10.0f, 10.0f);
    floorBatch.Normal3f(0.0, 1.0f, 0.0f);
    floorBatch.Vertex3f(20.0f, -0.41f, -20.0f);
    
    floorBatch.Color4f(0.0f, 1.0f, 0.0f, alpha);
    floorBatch.MultiTexCoord2f(0, 0.0f, 10.0f);
    floorBatch.Normal3f(0.0, 1.0f, 0.0f);
    floorBatch.Vertex3f(-20.0f, -0.41f, -20.0f);
    floorBatch.End();
    
    mirrorBatch.Begin(GL_TRIANGLE_FAN, 4, 1);
    mirrorBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    mirrorBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
    mirrorBatch.Normal3f(0.0f, 1.0f, 0.0f);
    mirrorBatch.Vertex3f(-1.0f, 0.0f, 0.0f);
    
    mirrorBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    mirrorBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
    mirrorBatch.Normal3f(0.0f, 1.0f, 0.0f);
    mirrorBatch.Vertex3f(1.0f, 0.0f, 0.0f);
    
    mirrorBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    mirrorBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
    mirrorBatch.Normal3f(0.0f, 1.0f, 1.0f);
    mirrorBatch.Vertex3f(1.0f, 2.0f, 0.0f);
    
    mirrorBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
    mirrorBatch.MultiTexCoord2f(0, 0.0f, 1.0f);
    mirrorBatch.Normal3f(0.0f, 1.0f, 0.0f);
    mirrorBatch.Vertex3f(-1.0f, 2.0f, 0.0f);
    mirrorBatch.End();
    
    mirrorBorderBatch.Begin(GL_TRIANGLE_STRIP, 13);
    mirrorBorderBatch.Normal3f(0.0f, 0.0f, 1.0f);
    mirrorBorderBatch.Vertex3f(-1.0f, 0.1f, 0.01f);
    
    mirrorBorderBatch.Normal3f(0.0f, 0.0f, 1.0f);
    mirrorBorderBatch.Vertex3f(-1.0f, 0.0f, 0.01);
    
    mirrorBorderBatch.Normal3f(0.0f, 0.0f, 1.0f);
    mirrorBorderBatch.Vertex3f(1.0f, 0.1f, 0.01f);
    
    mirrorBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
    mirrorBorderBatch.Vertex3f(1.0f, 0.0f, 0.01f);
    
    mirrorBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
    mirrorBorderBatch.Vertex3f(0.9f, 0.0f, 0.01f);
    
    mirrorBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
    mirrorBorderBatch.Vertex3f(1.0f, 2.0f, 0.01f);
    
    mirrorBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
    mirrorBorderBatch.Vertex3f(0.9f, 2.0f, 0.01f);
    
    mirrorBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
    mirrorBorderBatch.Vertex3f(1.0f, 1.9f, 0.01f);
    
    mirrorBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
    mirrorBorderBatch.Vertex3f(-1.0f, 2.f, 0.01f);
    
    mirrorBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
    mirrorBorderBatch.Vertex3f(-1.0f, 1.9f, 0.01f);
    
    mirrorBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
    mirrorBorderBatch.Vertex3f(-0.9f, 2.f, 0.01f);
    
    mirrorBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
    mirrorBorderBatch.Vertex3f(-1.0f, 0.0f, 0.01f);
    
    mirrorBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
    mirrorBorderBatch.Vertex3f(-0.9f, 0.0f, 0.01f);
    mirrorBorderBatch.End();
    
    glGenTextures(1, textures);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    
    LoadBMPTexture("Marble.bmp", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT);
    
    glGenFramebuffers(1, &fboName);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboName);
    
    glGenRenderbuffers(1, &depthBufferName);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBufferName);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, mirrorTexWidth, mirrorTexHeight);

    glGenTextures(1, &mirrorTexture);
    glBindTexture(GL_TEXTURE_2D, mirrorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, mirrorTexWidth, mirrorTexHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mirrorTexture, 0);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBufferName);
    
    gltCheckErrors();
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void ShutdownRC(void)
{
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    glDeleteTextures(1, &depthBufferName);
    glDeleteTextures(1, textures);
    
    glDeleteRenderbuffers(1, &depthBufferName);
    glDeleteFramebuffers(1, &fboName);
}


void ChangeSize(int nWidth, int nHeight)
{
    glViewport(0, 0, nWidth, nHeight);
    transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);
    viewFrustum.SetPerspective(35.0f, float(nWidth) / float(nHeight), 1.0f, 100.0f);
    projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
    modelViewMatrix.LoadIdentity();
    screenWidth = nWidth;
    screenHeight = nHeight;
}

void SpecialKeys(int key, int x, int y)
{
    float linear = 0.40f;
    float angular = float(m3dDegToRad(2.5f));
    
    if(key == GLUT_KEY_UP)
        cameraFrame.MoveForward(linear);
    
    if(key == GLUT_KEY_DOWN)
        cameraFrame.MoveForward(-linear);
    
    if(key == GLUT_KEY_LEFT)
        cameraFrame.RotateWorld(angular, 0.0f, 1.0f, 0.0f);
    
    if(key == GLUT_KEY_RIGHT)
        cameraFrame.RotateWorld(-angular, 0.0f, 1.0f, 0.0f);
}

void DrawWorld(GLfloat yRot)
{
    M3DMatrix44f mCamera;
    modelViewMatrix.GetMatrix(mCamera);
    
    M3DVector4f vLightTrnasformed;
    m3dTransformVector4(vLightTrnasformed, vLightPos, mCamera);
    
    modelViewMatrix.PushMatrix();
    modelViewMatrix.Translatev(vLightPos);
    shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vWhite);
    sphereBatch.Draw();
    modelViewMatrix.PopMatrix();
    
    modelViewMatrix.PushMatrix();
    modelViewMatrix.Translate(0.2f, 0.2f, -2.5f);
//    modelViewMatrix.Rotate(yRot, 0.0f, 1.0f, 0.0f);
    shaderManager.UseStockShader(GLT_SHADER_POINT_LIGHT_DIFF, modelViewMatrix.GetMatrix(), transformPipeline.GetProjectionMatrix(), vLightTrnasformed, vGreen, 0);
    torusBatch.Draw();
    modelViewMatrix.PopMatrix();
}

void RenderScene(void)
{
    static CStopWatch animationTimer;
    float mirrorRot = animationTimer.GetElapsedSeconds();
    float yRot =  mirrorRot * 60.0f;
    
    M3DVector3f vCameraPos;
    M3DVector3f vCameraForward;
    M3DVector3f vMirrorPos;
    M3DVector3f vMirrorForward;
    
    cameraFrame.GetOrigin(vCameraPos);
    cameraFrame.GetForwardVector(vCameraForward);
    
    vMirrorPos[0] = 0.0f;
    vMirrorPos[1] = 0.1f;
    vMirrorPos[2] = -6.0f;
    mirrorFrame.SetOrigin(vMirrorPos);
    
//    vMirrorForward[0] = vCameraPos[0];
//    vMirrorForward[1] = vCameraPos[1];
//    vMirrorForward[2] = (vCameraPos[2] + 1);
//    m3dNormalizeVector3(vMirrorForward);
    M3DMatrix44f rotateMatrix;
    M3DVector4f vRotatedBefore;
    vRotatedBefore[0] = 0;
    vRotatedBefore[1] = 0;
    vRotatedBefore[2] = 1;

    m3dRotationMatrix44(rotateMatrix, mirrorRot, 0.0f, -1.0f, 0.f);
    m3dTranslationMatrix44(rotateMatrix, vCameraPos[0], vCameraPos[0], vCameraPos[2]);
    m3dTransformVector3(vMirrorForward, vRotatedBefore, rotateMatrix);
    m3dNormalizeVector3(vMirrorForward);
    mirrorFrame.SetForwardVector(vMirrorForward);
    
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboName);
    glDrawBuffers(1, fboBuffs);
    glViewport(0, 0, mirrorTexWidth, mirrorTexHeight);
    
    modelViewMatrix.PushMatrix();
    M3DMatrix44f mMirrorView;
    mirrorFrame.GetCameraMatrix(mMirrorView);
    modelViewMatrix.MultMatrix(mMirrorView);
    
    modelViewMatrix.Scale(-1.0f, 1.0f, 1.0f);
    
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shaderManager.UseStockShader(GLT_SHADER_TEXTURE_MODULATE, transformPipeline.GetModelViewProjectionMatrix(), vWhite, 0);
    floorBatch.Draw();
    DrawWorld(yRot);
    
    M3DVector4f vLightTransformed;
    modelViewMatrix.GetMatrix(mMirrorView);
    m3dTransformVector4(vLightTransformed, vLightPos, mMirrorView);
    modelViewMatrix.Translate(vCameraPos[0], vCameraPos[1] - 0.8f, vCameraPos[2] - 1.0f);
    modelViewMatrix.Rotate(-90.0f, 1.0f, 0.0f, 0.0f);
    shaderManager.UseStockShader(GLT_SHADER_POINT_LIGHT_DIFF, modelViewMatrix.GetMatrix(), transformPipeline.GetProjectionMatrix(), vLightTransformed, vBlue, 0);
    cylinderBatch.Draw();
    modelViewMatrix.PopMatrix();
    
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glDrawBuffers(1, windowBuff);
    glViewport(0, 0, screenWidth, screenHeight);
    
    modelViewMatrix.PushMatrix();
    M3DMatrix44f mCamera;
    cameraFrame.GetCameraMatrix(mCamera);
    modelViewMatrix.MultMatrix(mCamera);
    
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shaderManager.UseStockShader(GLT_SHADER_TEXTURE_MODULATE, transformPipeline.GetModelViewProjectionMatrix(), vWhite, 0);
    floorBatch.Draw();
    DrawWorld(yRot);
    
    modelViewMatrix.PushMatrix();
    modelViewMatrix.Translate(0.0f, -0.4f, -5.0f);
    modelViewMatrix.Rotate(yRot, 0, 1.0f, 0.0f);
//    if (vCameraPos[2] > -5.0f) {
//        glBindTexture(GL_TEXTURE_2D, mirrorTexture);
//        shaderManager.UseStockShader(GLT_SHADER_TEXTURE_REPLACE, transformPipeline.GetModelViewProjectionMatrix(), 0);
//    } else {
//        shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlack);
//    }
    float pi = 3.1415926;
    float angle = ((int)(mirrorRot * 10000000) % (int)(pi * 2* 10000000)) / 10000000.0f;
    if ((0 <= angle && angle <= pi / 2) ||(pi * 1.5 <= angle && angle < pi * 2)) {
        glBindTexture(GL_TEXTURE_2D, mirrorTexture);
        shaderManager.UseStockShader(GLT_SHADER_TEXTURE_REPLACE, transformPipeline.GetModelViewProjectionMatrix(), 0);
    } else {
        shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlack);
    }
    
    mirrorBatch.Draw();
    shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGrey);
    mirrorBorderBatch.Draw();
    modelViewMatrix.PopMatrix();
    modelViewMatrix.PopMatrix();
    
    glutSwapBuffers();
    glutPostRedisplay();
}

int main(int argc, char* argv[])
{
    screenWidth = 800;
    screenHeight = 600;
    bFullScreen = false;
    bAnimated = true;
    fboName = 0;
    depthBufferName = 0;
    
    glewExperimental = GL_TRUE;
    gltSetWorkingDirectory(argv[0]);
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(screenWidth,screenHeight);
    
    glutCreateWindow("FBO Textures");
    
    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderScene);
    glutSpecialFunc(SpecialKeys);
    
    SetupRC();
    glutMainLoop();
    ShutdownRC();
    return 0;
}
