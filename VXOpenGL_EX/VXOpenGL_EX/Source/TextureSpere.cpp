//
//  TextureSpere.cpp
//  VXOpenGL_EX
//
//  Created by pf on 28/10/2017.
//  Copyright © 2017 vicxia. All rights reserved.
//

#include <GLTools.h>    // OpenGL toolkit
#include <GLMatrixStack.h>
#include <GLFrame.h>
#include <GLFrustum.h>
#include <GLGeometryTransform.h>
#include <StopWatch.h>
#include <GLUT/GLUT.h>

GLFrame viewFrame;
GLFrustum viewFrustum;
GLTriangleBatch spereBatch;
GLMatrixStack modelViewMatrix;
GLMatrixStack projectionMatrix;
GLGeometryTransform transformPipeline;

GLuint shader;
GLuint texture0;
GLuint texture1;
GLint locAmbient;
GLint locDiffuse;
GLint locSpecular;
GLint locLight;
GLint locMVPMatrix;
GLint locMVMatrix;
GLint locNormalMatrix;
GLint locTexture0;
GLint locTexture1;
GLint locDissolve;

bool LoadTGATextureJ(const char *szFileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode)
{
    GLbyte *pBits;
    int nWidth, nHeight, nComponents;
    GLenum eFormat;
    
    pBits = gltReadTGABits(szFileName, &nWidth, &nHeight, &nComponents, &eFormat);
    if (pBits == NULL) {
        return false;
    }
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, nComponents, nWidth, nHeight, 0, eFormat, GL_UNSIGNED_BYTE, pBits);
    free(pBits);
    if (minFilter == GL_LINEAR_MIPMAP_LINEAR ||
        minFilter == GL_LINEAR_MIPMAP_NEAREST ||
        minFilter == GL_NEAREST_MIPMAP_LINEAR ||
        minFilter == GL_NEAREST_MIPMAP_NEAREST) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    return true;
}

void SetupRC(void)
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    
    viewFrame.MoveForward(4.0f);
    gltMakeSphere(spereBatch, 1.0f, 26, 13);
    
    shader = gltLoadShaderPairWithAttributes("DissolveVertex.glsl", "DissolveFragment.glsl", 3, GLT_ATTRIBUTE_VERTEX, "vVertex", GLT_ATTRIBUTE_NORMAL, "vNormal", GLT_ATTRIBUTE_TEXTURE0, "vTexture0");
    locAmbient = glGetUniformLocation(shader, "ambientColor");
    locDiffuse = glGetUniformLocation(shader, "diffuseColor");
    locSpecular = glGetUniformLocation(shader, "specularColor");
    locLight = glGetUniformLocation(shader, "vLightPosition");
    locMVPMatrix = glGetUniformLocation(shader, "mvpMatrix");
    locMVMatrix = glGetUniformLocation(shader, "mvMatrix");
    locNormalMatrix = glGetUniformLocation(shader, "normalMatrix");
    locTexture0 = glGetUniformLocation(shader, "colorMap0");
    locTexture1 = glGetUniformLocation(shader, "colorMap1");
    locDissolve = glGetUniformLocation(shader, "dissolveFactor");
    glGenTextures(1, &texture0);
    glBindTexture(GL_TEXTURE_2D, texture0);
    LoadTGATextureJ("CoolTexture.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINE, GL_CLAMP_TO_EDGE);
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);
    LoadTGATextureJ("Clouds.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINE, GL_CLAMP_TO_EDGE);
}

void ShutdownRC(void)
{
    glDeleteShader(shader);
    glDeleteTextures(1, &texture0);
    glDeleteTextures(1, &texture1);
}

void ChangeSize(int w, int h)
{
    glViewport(0, 0, w, h);
    viewFrustum.SetPerspective(35.0f, float(w) / float(h), 1.0f, 100.0f);
    projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
    transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);
}

void RenderScene(void)
{
    static CStopWatch rotTimer;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    modelViewMatrix.PushMatrix(viewFrame);
    modelViewMatrix.Rotate(rotTimer.GetElapsedSeconds() * 20, 0, 1.0f, 0);
    GLfloat vEyeLight[] = {-100.0f, 100.0f, 100.0f};
    GLfloat vAmbientColor[] = {0.2f, 0.2f, 0.2f, 1.0f};
    GLfloat vDiffuseColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat vSpecularColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    
    glUseProgram(shader);
    glUniform4fv(locAmbient, 1, vAmbientColor);
    glUniform4fv(locDiffuse, 1, vDiffuseColor);
    glUniform4fv(locSpecular, 1, vSpecularColor);
    glUniform3fv(locLight, 1, vEyeLight);
    glUniformMatrix4fv(locMVPMatrix, 1, GL_FALSE, transformPipeline.GetModelViewProjectionMatrix());
    glUniformMatrix4fv(locMVMatrix, 1, GL_FALSE, transformPipeline.GetModelViewMatrix());
    glUniformMatrix3fv(locNormalMatrix, 1, GL_FALSE, transformPipeline.GetNormalMatrix());
    glBindTexture(GL_TEXTURE_2D, texture0);
    glUniform1i(locTexture0, 0);
    glBindTexture(GL_TEXTURE_2D, texture1);
    glUniform1i(locTexture1, 0);
    
    float fFactor = fmod(rotTimer.GetElapsedSeconds(), 10.0f) / 10.0f;
    glUniform1f(locDissolve, fFactor);
    spereBatch.Draw();
    modelViewMatrix.PopMatrix();
    
    glutSwapBuffers();
    glutPostRedisplay();
}

int main(int argc, char* argv[])
{
    glewExperimental = GL_TRUE;
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Texture Spere");
    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderScene);
    
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
