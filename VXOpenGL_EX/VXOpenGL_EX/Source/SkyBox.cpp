//
//  SkyBox.cpp
//  VXOpenGL_EX
//
//  Created by pf on 29/10/2017.
//  Copyright © 2017 vicxia. All rights reserved.
//
#include <GLTools.h>    // OpenGL toolkit
#include <GLMatrixStack.h>
#include <GLFrame.h>
#include <GLFrustum.h>
#include <GLGeometryTransform.h>
#include <StopWatch.h>

#include <math.h>
#include <stdlib.h>

#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif

GLFrame viewFrame;
GLFrustum viewFurstum;
GLTriangleBatch sphereBatch;
GLBatch     cubeBatch;
GLMatrixStack modelViewMatrix;
GLMatrixStack projectionMatrix;
GLGeometryTransform transformPipline;
GLuint  cubeTexture;
GLint   reflectionShader;
GLint   skyBoxShader;

GLint locMVPReflect, locMVRelect, locNormalRelect, locInvertedCamera;
GLint locMVPSkyBox;

const char *szCubeFaces[6] = { "pos_x.tga", "neg_x.tga", "pos_y.tga", "neg_y.tga", "pos_z.tga", "neg_z.tga" };

GLenum  cube[6] = {  GL_TEXTURE_CUBE_MAP_POSITIVE_X,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
    GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
    GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Z };
void SetupRC(void)
{
    GLbyte *pBytes;
    GLint iWidth, iHeight, iComponents;
    GLenum eFormat;
    int i;
    
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    
    glGenTextures(1, &cubeTexture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTexture);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    for (i = 0; i < 6; i++) {
        pBytes = gltReadTGABits(szCubeFaces[i], &iWidth, &iHeight, &iComponents, &eFormat);
        glTexImage2D(cube[i], 0, iComponents, iWidth, iHeight, 0, eFormat, GL_UNSIGNED_BYTE, pBytes);
        free(pBytes);
    }
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    viewFrame.MoveForward(-4.0f);
    gltMakeSphere(sphereBatch, 1.0f, 52, 26);
    gltMakeCube(cubeBatch, 20);
    
    reflectionShader = gltLoadShaderPairWithAttributes("ReflectionVS.glsl", "ReflectionFS.glsl", 2, GLT_ATTRIBUTE_VERTEX, "vVertex", GLT_ATTRIBUTE_NORMAL, "vNormal");
    
    locMVPReflect = glGetUniformLocation(reflectionShader, "mvpMatrix");
    locMVRelect = glGetUniformLocation(reflectionShader, "mvMatrix");
    locNormalRelect = glGetUniformLocation(reflectionShader, "normalMatrix");
    locInvertedCamera = glGetUniformLocation(reflectionShader, "mInverseCamera");
    
    skyBoxShader = gltLoadShaderPairWithAttributes("SkyBoxVS.glsl", "SkyBoxFS.glsl", 1, GLT_ATTRIBUTE_VERTEX, "vVertex");
    locMVPSkyBox = glGetUniformLocation(skyBoxShader, "mvpMatrix");
}

void ShutdownRC(void)
{
    glDeleteTextures(1, &cubeTexture);
}

void RenderScene(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    M3DMatrix44f mCamera;
    M3DMatrix44f mCameraRotOnly;
    M3DMatrix44f mInverseCamera;
    
    viewFrame.GetCameraMatrix(mCamera, false);
    viewFrame.GetCameraMatrix(mCameraRotOnly, true);
    m3dInvertMatrix44(mInverseCamera, mCameraRotOnly);
    
    modelViewMatrix.PushMatrix();
    modelViewMatrix.MultMatrix(mCamera);
    glUseProgram(reflectionShader);
    glUniformMatrix4fv(locMVPReflect, 1, GL_FALSE, transformPipline.GetModelViewProjectionMatrix());
    glUniformMatrix4fv(locMVRelect, 1, GL_FALSE, transformPipline.GetModelViewMatrix());
    glUniformMatrix3fv(locNormalRelect, 1, GL_FALSE, transformPipline.GetNormalMatrix());
    glUniformMatrix4fv(locInvertedCamera, 1, GL_FALSE, mInverseCamera);
    glEnable(GL_CULL_FACE);
    sphereBatch.Draw();
    glDisable(GL_CULL_FACE);
    modelViewMatrix.PopMatrix();
    
    modelViewMatrix.PushMatrix();
    modelViewMatrix.MultMatrix(mCamera);
    glUseProgram(skyBoxShader);
    glUniformMatrix4fv(locMVPSkyBox, 1, GL_FALSE, transformPipline.GetModelViewProjectionMatrix());
    cubeBatch.Draw();
    modelViewMatrix.PopMatrix();
    glutSwapBuffers();
}

void ChangeSize(int w, int h)
{
    glViewport(0, 0, w, h);
    viewFurstum.SetPerspective(35.0f, float(w) / float(h), 1.0f, 1000.0f);
    projectionMatrix.LoadMatrix(viewFurstum.GetProjectionMatrix());
    transformPipline.SetMatrixStacks(modelViewMatrix, projectionMatrix);
}

void SpecialKeys(int key, int x, int y)
{
    switch (key) {
        case GLUT_KEY_UP:
            viewFrame.MoveForward(0.1f);
            break;
        case GLUT_KEY_DOWN:
            viewFrame.MoveForward(-0.1f);
            break;
        case GLUT_KEY_LEFT:
            viewFrame.RotateLocalY(0.1);
            break;
        case GLUT_KEY_RIGHT:
            viewFrame.RotateLocalY(-0.1);
            break;
    }
    glutPostRedisplay();
}

int main(int argc, char* argv[])
{
    glewExperimental = GL_TRUE;
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800,600);
    glutCreateWindow("OpenGL Cube Maps");
    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderScene);
    glutSpecialFunc(SpecialKeys);
    
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
        return 1;
    }
    
    
    SetupRC();
    
    glutMainLoop();
    
    ShutdownRC();
    
    return 0;
}

