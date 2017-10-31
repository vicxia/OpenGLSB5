//
//  PixelBuffers.cpp
//  VXOpenGL_EX
//
//  Created by vicxia on 31/10/2017.
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
#include <GLUT/GLUT.h>

static GLfloat vGreen[] = { 0.0f, 1.0f, 0.0f, 1.0f };
static GLfloat vWhite[] = { 1.0f, 1.0f, 1.0f, 1.0f };
static GLfloat vLightPos[] = { 0.0f, 3.0f, 0.0f, 1.0f };

GLsizei     screenWidth;            // Desired window or desktop width
GLsizei  screenHeight;            // Desired window or desktop height

GLboolean bFullScreen;            // Request to run full screen
GLboolean bAnimated;            // Request for continual updates

GLShaderManager shaderManager;
GLMatrixStack   modelViewMatrix;
GLMatrixStack   projectionMatrix;
M3DMatrix44f    orthoMatrix;
GLFrustum       viewFrustum;
GLFrame         cameraFrame;
GLGeometryTransform transformPipeline;

GLTriangleBatch torusBatch;
GLBatch         floorBatch;
GLBatch         screenQuad;

GLuint          textures[1];
GLuint          blurTextures[6];
GLuint          pixBuffObjs[1];
GLuint          curBlurTarget;
bool            bUsePBOPath;
GLfloat         speedFactor;
GLuint          blurProg;
void            *pixelData;
GLuint          pixelDataSize;

void MoveCamera(void);
void DrawWorld(GLfloat yRot, GLfloat xPos);
bool LoadBMPTexture(const char *szFileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode);
void SetupBlurProg(void);

void AdvanceBlurTarget() { curBlurTarget = (curBlurTarget + 1) % 6; }
GLuint GetBlurTarget0() {return (1 + (curBlurTarget + 5) % 6); }
GLuint GetBlurTarget1() {return (1 + (curBlurTarget + 4) % 6); }
GLuint GetBlurTarget2() {return (1 + (curBlurTarget + 3) % 6); }
GLuint GetBlurTarget3() {return (1 + (curBlurTarget + 2) % 6); }
GLuint GetBlurTarget4() {return (1 + (curBlurTarget + 1) % 6); }
GLuint GetBlurTarget5() {return (1 + (curBlurTarget + 0) % 6); }

void UpdateFrameCount()
{
    static int iFrames = 0;
    static CStopWatch frameTimer;
    if (iFrames == 0) {
        frameTimer.Reset();
        iFrames++;
    }
    iFrames++;
    if (iFrames == 101) {
        float fps = 100.0f / frameTimer.GetElapsedSeconds();
        if (bUsePBOPath) {
            printf("Pix_buffs - Using PBOs %0.1f fps\n", fps);
        } else {
            printf("Pix_buffs - Using Client mem copies %.1f fps\n", fps);
        }
        frameTimer.Reset();
        iFrames = 1;
    }
}

bool LoadBMPTexture(const char *szFileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode)
{
    GLbyte *pBytes;
    GLint iWidth, iHeight;
    pBytes = gltReadBMPBits(szFileName, &iWidth, &iHeight);
    if (pBytes == NULL) {
        return false;
    }
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
    
    if(minFilter == GL_LINEAR_MIPMAP_LINEAR || minFilter == GL_LINEAR_MIPMAP_NEAREST || minFilter == GL_NEAREST_MIPMAP_LINEAR || minFilter == GL_NEAREST_MIPMAP_NEAREST) {
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
    }
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, iWidth, iHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, pBytes);
    return true;
}

void SetupRC(void)
{
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
    }
    
    shaderManager.InitializeStockShaders();
}

void ShutdownRC(void)
{
    
}
void ChangeSize(int nWidth, int nHeight)
{
}
void ProccessKeys(unsigned char key, int x, int y)
{
    static CStopWatch cameraTimer;
    float fTime = cameraTimer.GetElapsedSeconds();
    float linear = fTime * 12.0f;
    cameraTimer.Reset();
    
    // Alternate between PBOs and local memory when 'P' is pressed
    if(key == 'P' || key == 'p')
        bUsePBOPath = (bUsePBOPath)? GL_FALSE : GL_TRUE;
    
    // Speed up movement
    if(key == '+')
    {
        speedFactor += linear/2;
        if(speedFactor > 6)
            speedFactor = 6;
    }
    
    // Slow down moement
    if(key == '-')
    {
        speedFactor -= linear/2;
        if(speedFactor < 0.5)
            speedFactor = 0.5;
    }
}

void SetupBlurProg(void)
{
}

void DrawWorld(GLfloat yRot, GLfloat xPos)
{
}

void RenderScene(void)
{
    
}

int main(int argc, char* argv[])
{
    screenWidth  = 800;
    screenHeight = 600;
    bFullScreen = false;
    bAnimated   = true;
    bUsePBOPath = false;
    blurProg    = 0;
    speedFactor = 1.0f;
    
    glewExperimental = GL_TRUE;
    gltSetWorkingDirectory(argv[0]);
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(screenWidth,screenHeight);
    
    glutCreateWindow("Pix Buffs");
    
    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderScene);
    glutKeyboardFunc(ProccessKeys);
    
    SetupRC();
    glutMainLoop();
    ShutdownRC();
    return 0;
}
