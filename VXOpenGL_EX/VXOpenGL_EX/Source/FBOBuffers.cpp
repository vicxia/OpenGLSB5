//
//  FBOBuffers.cpp
//  VXOpenGL_EX
//
//  Created by vicxia on 09/11/2017.
//  Copyright © 2017 vicxia. All rights reserved.
//

#include <stdio.h>
#include <iostream>

#include <GLTools.h>
#include <GLShaderManager.h>
#include <GLFrustum.h>
#include <GLBatch.h>
#include <GLMatrixStack.h>
#include <GLGeometryTransform.h>
#include <StopWatch.h>
#include "sbm.h"
#include <GLUT/GLUT.h>

static GLfloat vGreen[] = {0.0f, 1.0f, 0.0f, 1.0f};
static GLfloat vWhite[] = {1.0f, 1.0f, 1.0f, 1.0f};
static GLfloat vLightPos[] = {0.0f, 3.0f, 0.0f, 1.0f};
static const GLenum windowBuff[] = {GL_BACK_LEFT};
static const GLenum fboBuffs[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};

GLsizei screenWidth;
GLsizei screenHeight;
GLboolean bFullScreen;
GLboolean bAnimated;

GLShaderManager shaderManager;
GLMatrixStack   modelViewMatrix;
GLMatrixStack   projectionMatrix;
GLFrustum       viewFrustum;
GLGeometryTransform transformPipeline;
GLFrame         cameraFrame;

GLTriangleBatch torusBatch;
GLTriangleBatch spereBatch;
GLBatch floorBatch;
GLBatch screenQuad;

GLuint  textures[3];
GLuint  processProg;
GLuint  texBO[3];
GLuint  texBOTexture;
bool    bUseFBO;
GLuint  fboName;
GLuint  depthBufferName;
GLuint  renderBufferNames[3];

SBObject  ninja;
GLuint  ninjaTex[1];

void MoveCamer(void);
void DrawWorld(GLfloat yRot);
bool LoadBMPTexture(const char *szFileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode);

static float * LoadFloatData(const char * szFile, int *count)
{
    GLint lineCount = 0;
    float *data = 0;
    FILE *fp;
    fp = fopen(szFile, "r");
    if (fp != NULL) {
        char szFloat[1024];
        while (fgets(szFloat, sizeof(szFloat), fp) != NULL) {
            lineCount++;
        }
        rewind(fp);
        
        data = (float *)malloc(lineCount * sizeof(float));
        if (data != NULL) {
            int index = 0;
            while (fgets(szFloat, sizeof(szFloat), fp) != NULL) {
                data[index] = (float)atof(szFloat);
                index ++;
            }
            count[0] = index;
        }
        fclose(fp);
        return data;
    }
    return 0;
}

bool LoadBMPTexture(const char *szFileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode)
{
    GLbyte *pBits;
    GLint iWidth, iHeight;
    
    pBits = gltReadBMPBits(szFileName, &iWidth, &iHeight);
    if (pBits == NULL) {
        return false;
    }
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, iWidth, iHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, pBits);
    
    if (minFilter == GL_LINEAR_MIPMAP_LINEAR || minFilter == GL_LINEAR_MIPMAP_NEAREST ||
        minFilter == GL_NEAREST_MIPMAP_LINEAR || minFilter == GL_NEAREST_MIPMAP_NEAREST) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    return true;
}

void SetupRC()
{
    GLenum  err = glewInit();
    if (GLEW_OK != err) {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
    }
    
    shaderManager.InitializeStockShaders();
    
    glEnable(GL_DEPTH_TEST);
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    ninja.LoadFromSBM("ninja.sbm", GLT_ATTRIBUTE_VERTEX, GLT_ATTRIBUTE_NORMAL, GLT_ATTRIBUTE_TEXTURE0);
    
    gltMakeTorus(torusBatch, 0.4f, 0.15f, 35, 35);
    gltMakeSphere(spereBatch, 0.1f, 26, 13);
    
    GLfloat alpha = 0.25f;
    
    floorBatch.Begin(GL_TRIANGLE_FAN, 4, 1);
    floorBatch.Color4f(0.0f, 1.0f, 0.0f, alpha);
    floorBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
    floorBatch.Normal3f(0.0f, 1.0f, 0.0f);
    floorBatch.Vertex3f(-20.f, -0.41f, 20.0f);
    
    floorBatch.Color4f(0.0f, 1.0f, 0.0f, alpha);
    floorBatch.MultiTexCoord2f(0, 10.0f, 0.0f);
    floorBatch.Normal3f(0.0f, 1.0f, 0.0f);
    floorBatch.Vertex3f(20.0f, -0.41f, 20.0f);
    
    floorBatch.Color4f(0.0f, 1.0f, 0.0f, alpha);
    floorBatch.MultiTexCoord2f(0, 10.0f, 10.f);
    floorBatch.Normal3f(0.0f, 1.0f, 0.0f);
    floorBatch.Vertex3f(20.0f, -0.41f, -20.0f);
    
    floorBatch.Color4f(0.0f, 1.0f, 0.0f, alpha);
    floorBatch.MultiTexCoord2f(0, 0.0f, 10.0f);
    floorBatch.Normal3f(0.0f, 1.0f, 0.0f);
    floorBatch.Vertex3f(-20.f, -0.41f, -20.f);
    floorBatch.End();
    
    glGenTextures(1, textures);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    LoadBMPTexture("Marble.bmp", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT);
    
    glGenTextures(1, ninjaTex);
    glBindTexture(GL_TEXTURE_2D, ninjaTex[0]);
    LoadBMPTexture("NinjaComp.BMP", GL_LINEAR, GL_LINEAR, GL_CLAMP);
    
    glGenFramebuffers(1, &fboName);
    
    glGenRenderbuffers(1, &depthBufferName);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBufferName);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, screenWidth, screenHeight);
    
    glGenRenderbuffers(3, renderBufferNames);
    glBindRenderbuffer(GL_RENDERBUFFER, renderBufferNames[0]);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, screenWidth, screenHeight);
    glBindRenderbuffer(GL_RENDERBUFFER, renderBufferNames[1]);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, screenWidth, screenHeight);
    glBindRenderbuffer(GL_RENDERBUFFER, renderBufferNames[2]);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, screenWidth, screenHeight);
    
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboName);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBufferName);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderBufferNames[0]);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_RENDERBUFFER, renderBufferNames[1]);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_RENDERBUFFER, renderBufferNames[2]);
    
    processProg = gltLoadShaderPairWithAttributes("multibuffer.vs", "multibuffer_frag_location.fs", 3,
                                                  GLT_ATTRIBUTE_VERTEX, "vVertex",
                                                  GLT_ATTRIBUTE_NORMAL, "vNormal",
                                                  GLT_ATTRIBUTE_TEXTURE0, "texCoord0");
//    glBindFragDataLocation(processProg, 0, "oStraightColor");
//    glBindFragDataLocation(processProg, 1, "oGreyscale");
//    glBindFragDataLocation(processProg, 2, "oLumAdjColor");
//    glLinkProgram(processProg);
    
    glGenBuffers(3, texBO);
    glGenTextures(1, &texBOTexture);
    
    int count = 0;
    float *fileData = 0;
    fileData = LoadFloatData("LumTan.data", &count);
    if (count > 0) {
        glBindBuffer(GL_TEXTURE_BUFFER_ARB, texBO[0]);
        glBufferData(GL_TEXTURE_BUFFER_ARB, sizeof(float) * count, fileData, GL_STATIC_DRAW);
        delete fileData;
    }
    count = 0;
    fileData = LoadFloatData("LumSin.data", &count);
    if (count > 0) {
        glBindBuffer(GL_TEXTURE_BUFFER_ARB, texBO[1]);
        glBufferData(GL_TEXTURE_BUFFER_ARB, sizeof(float) * count, fileData, GL_STATIC_DRAW);
        delete fileData;
    }
    count = 0;
    fileData = LoadFloatData("LumLinear.data", &count);
    if (count > 0) {
        glBindBuffer(GL_TEXTURE_BUFFER_ARB, texBO[2]);
        glBufferData(GL_TEXTURE_BUFFER_ARB, sizeof(float) * count, fileData, GL_STATIC_DRAW);
        delete fileData;
    }
    
    glBindBuffer(GL_TEXTURE_BUFFER_ARB, 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_BUFFER_ARB, texBOTexture);
    glTexBuffer(GL_TEXTURE_BUFFER_ARB, GL_R32F, texBO[0]);
    glActiveTexture(GL_TEXTURE0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    gltCheckErrors();
}

void SpecialKeys(int key, int x, int y)
{
    static CStopWatch cameraTimer;
    float fTime = cameraTimer.GetElapsedSeconds();
    cameraTimer.Reset(); 
    
    float linear = fTime * 3.0f;
    float angular = fTime * float(m3dDegToRad(60.0f));
    
    if(key == GLUT_KEY_UP)
        cameraFrame.MoveForward(linear);
    
    if(key == GLUT_KEY_DOWN)
        cameraFrame.MoveForward(-linear);
    
    if(key == GLUT_KEY_LEFT)
        cameraFrame.RotateWorld(angular, 0.0f, 1.0f, 0.0f);
    
    if(key == GLUT_KEY_RIGHT)
        cameraFrame.RotateWorld(-angular, 0.0f, 1.0f, 0.0f);
    
    static bool bF2IsDown = false;
    if(key == GLUT_KEY_F2)
    {
        if(bF2IsDown == false)
        {
            bF2IsDown = true;
            bUseFBO = !bUseFBO;
        }
    }
    else
    {
        bF2IsDown = false; 
    }
    
    if(key == GLUT_KEY_F3)
    {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_BUFFER_ARB, texBOTexture);
        //        glTexBufferARB(GL_TEXTURE_BUFFER_ARB, GL_R32F, texBO[0]); // FIX THIS IN GLEE
        glTexBuffer(GL_TEXTURE_BUFFER_ARB, GL_R32F, texBO[0]); // FIX THIS IN GLEE
        glActiveTexture(GL_TEXTURE0);
    }
    else if(key == GLUT_KEY_F4)
    {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_BUFFER_ARB, texBOTexture);
        glTexBuffer(GL_TEXTURE_BUFFER_ARB, GL_R32F, texBO[1]); // FIX THIS IN GLEE
        glActiveTexture(GL_TEXTURE0);
    }
    else if(key == GLUT_KEY_F5)
    {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_BUFFER_ARB, texBOTexture);
        glTexBuffer(GL_TEXTURE_BUFFER_ARB, GL_R32F, texBO[2]); // FIX THIS IN GLEE
        glActiveTexture(GL_TEXTURE0);
    }
    
    // Refresh the Window
    glutPostRedisplay();
}

void ShutdownRC(void)
{
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);
    
    glDeleteTextures(1, &texBOTexture);
    glDeleteTextures(1, textures);
    glDeleteTextures(1, ninjaTex);
    
    glDeleteRenderbuffers(3, renderBufferNames);
    glDeleteRenderbuffers(1, &depthBufferName);
    
    glDeleteFramebuffers(1, &fboName);
    
    glDeleteBuffers(3, texBO);
    
    glUseProgram(0);
    glDeleteProgram(processProg);
    ninja.Free();
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
    
    glBindRenderbuffer(GL_RENDERBUFFER, depthBufferName);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, screenWidth, screenHeight);
    glBindRenderbuffer(GL_RENDERBUFFER, renderBufferNames[0]);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, screenWidth, screenHeight);
    glBindRenderbuffer(GL_RENDERBUFFER, renderBufferNames[1]);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, screenWidth, screenHeight);
    glBindRenderbuffer(GL_RENDERBUFFER, renderBufferNames[2]);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, screenWidth, screenHeight);
}

void UseProcessProgram(M3DVector4f vLightPos, M3DVector4f vColor, int textureUnit)
{
    /*
    glUseProgram(processProg);
    glUniformMatrix4fv(glGetUniformLocation(processProg, "mvMatrix"), 1, GL_FALSE, transformPipeline.GetModelViewMatrix());
    glUniformMatrix4fv(glGetUniformLocation(processProg, "pMatrix"), 1, GL_FALSE, transformPipeline.GetProjectionMatrix());
    glUniform3fv(glGetUniformLocation(processProg, "vLightPos"), 1, vLightPos);
    glUniform4fv(glGetUniformLocation(processProg, "vColor"), 1, vColor);
    glUniform1i(glGetUniformLocation(processProg, "lumCurveSampler"), 1);
    
    if (textureUnit != -1) {
        glUniform1i(glGetUniformLocation(processProg, "bUseTexture"), 1);
        glUniform1i(glGetUniformLocation(processProg, "textureUnit0"), textureUnit);
    } else {
        glUniform1i(glGetUniformLocation(processProg, "bUseTexture"), 0);
    }
    gltCheckErrors(processProg);
    */
    glUseProgram(processProg);
    
    // Set Matricies for Vertex Program
    glUniformMatrix4fv(glGetUniformLocation(processProg, "mvMatrix"),
                       1, GL_FALSE, transformPipeline.GetModelViewMatrix());
    glUniformMatrix4fv(glGetUniformLocation(processProg, "pMatrix"),
                       1, GL_FALSE, transformPipeline.GetProjectionMatrix());
    
    // Set the light position
    glUniform3fv(glGetUniformLocation(processProg, "vLightPos"), 1, vLightPos);
    
    // Set the vertex color for rendered pixels
    glUniform4fv(glGetUniformLocation(processProg, "vColor"), 1, vColor);
    
    // Set the texture unit for the texBO fetch
    glUniform1i(glGetUniformLocation(processProg, "lumCurveSampler"), 1);
    
    // If this geometry is textured, set the texture unit
    if(textureUnit != -1)
    {
        glUniform1i(glGetUniformLocation(processProg, "bUseTexture"), 1);
        glUniform1i(glGetUniformLocation(processProg, "textureUnit0"), textureUnit);
    }
    else
    {
        glUniform1i(glGetUniformLocation(processProg, "bUseTexture"), 0);
    }
    
    gltCheckErrors(processProg);
}

void DrawWorld(GLfloat yRot)
{
    M3DMatrix44f mCamera;
    modelViewMatrix.GetMatrix(mCamera);
    
    M3DVector4f vLightTransformed;
    m3dTransformVector4(vLightTransformed, vLightPos, mCamera);
    
    modelViewMatrix.PushMatrix();
    modelViewMatrix.Translatev(vLightPos);
    if (bUseFBO) {
        UseProcessProgram(vLightPos, vWhite, -1);
    } else {
        shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vWhite);
    }
    spereBatch.Draw();
    modelViewMatrix.PopMatrix();
    
    modelViewMatrix.PushMatrix();
    modelViewMatrix.Translate(0.0f, 0.2f, -2.5f);
    modelViewMatrix.PushMatrix();
    modelViewMatrix.Rotate(yRot, 0.0f, 1.0f, 0.0f);
    modelViewMatrix.Translate(0.0f, -0.61f, 0.0f);
    modelViewMatrix.Scale(0.005, 0.005, 0.005);
    glBindTexture(GL_TEXTURE_2D, ninjaTex[0]);
    if (bUseFBO) {
        UseProcessProgram(vLightTransformed, vWhite, 0);
    } else {
        shaderManager.UseStockShader(GLT_SHADER_TEXTURE_REPLACE, transformPipeline.GetModelViewProjectionMatrix(), 0);
    }
    ninja.Render(0, 0);
    modelViewMatrix.PopMatrix();
}

void RenderScene(void)
{
    /*
    static CStopWatch animationTimer;
    float yRot = animationTimer.GetElapsedSeconds() * 60.0f;
    
    modelViewMatrix.PushMatrix();
    M3DMatrix44f mCamera;
    cameraFrame.GetMatrix(mCamera);
    modelViewMatrix.MultMatrix(mCamera);
    GLfloat vFloorColor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    if (bUseFBO) {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboName);
        glDrawBuffers(3, fboBuffs);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        M3DVector4f vLightTransformed;
        m3dTransformVector4(vLightTransformed, vLightPos, mCamera);
        UseProcessProgram(vLightTransformed, vFloorColor, 0);
    } else {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glDrawBuffers(1, windowBuff);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shaderManager.UseStockShader(GLT_SHADER_TEXTURE_MODULATE, transformPipeline.GetModelViewProjectionMatrix(), vFloorColor, 0);
    }
    
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    floorBatch.Draw();
    DrawWorld(yRot);
    modelViewMatrix.PopMatrix();
    
    if (bUseFBO) {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glDrawBuffers(1, windowBuff);
        glViewport(0, 0, screenWidth, screenHeight);
        
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fboName);
        glReadBuffer(GL_COLOR_ATTACHMENT1);
        glBlitFramebuffer(0, 0, screenWidth / 2, screenHeight,
                          0, 0, screenWidth / 2, screenHeight,
                          GL_COLOR_BUFFER_BIT, GL_NEAREST);
        glReadBuffer(GL_COLOR_ATTACHMENT2);
        glBlitFramebuffer(screenWidth / 2, 0, screenWidth, screenHeight,
                          screenWidth / 2, 0, screenWidth, screenHeight,
                          GL_COLOR_BUFFER_BIT, GL_NEAREST);

        glReadBuffer(GL_COLOR_ATTACHMENT0);
        glBlitFramebuffer(0, 0, screenWidth, screenHeight,
                          (int)(screenWidth * 0.8), (int)(screenHeight * 0.8),
                          screenWidth, screenHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glutSwapBuffers();
    glutPostRedisplay();
     */
    static CStopWatch animationTimer;
    float yRot = animationTimer.GetElapsedSeconds() * 60.0f;
    //    MoveCamera();
    
    modelViewMatrix.PushMatrix();
    M3DMatrix44f mCamera;
    cameraFrame.GetCameraMatrix(mCamera);
    modelViewMatrix.MultMatrix(mCamera);
    
    GLfloat vFloorColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    
    if(bUseFBO)
    {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboName);
        glDrawBuffers(3, fboBuffs);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Need light position relative to the Camera
        M3DVector4f vLightTransformed;
        m3dTransformVector4(vLightTransformed, vLightPos, mCamera);
        UseProcessProgram(vLightTransformed, vFloorColor, 0);
    }
    else
    {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glDrawBuffers(1, windowBuff);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shaderManager.UseStockShader(GLT_SHADER_TEXTURE_MODULATE, transformPipeline.GetModelViewProjectionMatrix(), vFloorColor, 0);
    }
    
    glBindTexture(GL_TEXTURE_2D, textures[0]); // Marble
    floorBatch.Draw();
    DrawWorld(yRot);
    
    modelViewMatrix.PopMatrix();
    
    if(bUseFBO)
    {
        // Direct drawing to the window
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glDrawBuffers(1, windowBuff);
        glViewport(0, 0, screenWidth, screenHeight);
        
        // Source buffer reads from the framebuffer object
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fboName);
        
        // Copy greyscale output to the left half of the screen
        glReadBuffer(GL_COLOR_ATTACHMENT1);
        glBlitFramebuffer(0, 0, screenWidth/2, screenHeight,
                          0, 0, screenWidth/2, screenHeight,
                          GL_COLOR_BUFFER_BIT, GL_NEAREST );
        
        // Copy the luminance adjusted color to the right half of the screen
        glReadBuffer(GL_COLOR_ATTACHMENT2);
        glBlitFramebuffer(screenWidth/2, 0, screenWidth, screenHeight,
                          screenWidth/2, 0, screenWidth, screenHeight,
                          GL_COLOR_BUFFER_BIT, GL_NEAREST );
        
        // Scale the unaltered image to the upper right of the screen
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        glBlitFramebuffer(0, 0, screenWidth, screenHeight,
                          (int)(screenWidth *(0.8)), (int)(screenHeight*(0.8)),
                          screenWidth, screenHeight,
                          GL_COLOR_BUFFER_BIT, GL_LINEAR );
        
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    
    // Do the buffer Swap
    glutSwapBuffers();
    
    // Do it again
    glutPostRedisplay();
}

int main(int argc, char* argv[])
{
    screenWidth = 600;
    screenHeight = 1000;
    bFullScreen = false;
    bAnimated = true;
    bUseFBO = true;
    fboName = 0;
    depthBufferName = 0;
    glewExperimental = GL_TRUE;
    gltSetWorkingDirectory(argv[0]);
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(screenWidth,screenHeight);
    
    glutCreateWindow("FBO Drawbuffers");
    
    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderScene);
    glutSpecialFunc(SpecialKeys);
    
    SetupRC();
    glutMainLoop();
    ShutdownRC();
    return 0;
}



















