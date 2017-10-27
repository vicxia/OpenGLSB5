//
//  DiffuseLightSpere.cpp
//  VXOpenGL_EX
//
//  Created by vicxia on 27/10/2017.
//  Copyright © 2017 vicxia. All rights reserved.
//

#include <GLTools.h>
#include <GLMatrixStack.h>
#include <GLFrame.h>
#include <GLFrustum.h>
#include <GLGeometryTransform.h>
#include <StopWatch.h>
#include <GLUT/GLUT.h>

GLFrame viewFrame;
GLFrustum viewFrustum;
GLTriangleBatch sphereBatch;
GLMatrixStack modeoViewMatrix;
GLMatrixStack projectionMatrix;
GLGeometryTransform transformPipeline;

GLint diffuseShader;
GLuint locDiffColor;
GLuint locAmbientColor;
GLuint locSpecularColor;
GLuint locLightPos;
GLuint locMVPMatrix;
GLuint locMVMatrix;
GLuint locNormalMatrix;

void SetupRC()
{
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    
    viewFrame.MoveForward(4.0f);
    gltMakeSphere(sphereBatch, 1.0f, 26, 13);
    diffuseShader = gltLoadShaderPairWithAttributes("ADSPhoneVertex.glsl", "ADSPhoneFragment.glsl", 2, GLT_ATTRIBUTE_VERTEX, "vVertex", GLT_ATTRIBUTE_NORMAL, "vNormal");
    locAmbientColor = glGetUniformLocation(diffuseShader, "ambientColor");
    locSpecularColor = glGetUniformLocation(diffuseShader, "specularColor");
    locDiffColor = glGetUniformLocation(diffuseShader, "diffuseColor");
    locLightPos = glGetUniformLocation(diffuseShader, "vLightPosition");
    locMVPMatrix = glGetUniformLocation(diffuseShader, "mvpMatrix");
    locMVMatrix = glGetUniformLocation(diffuseShader, "mvMatrix");
    locNormalMatrix = glGetUniformLocation(diffuseShader, "normalMatrix");
}

void ShutDownRC()
{
    glDeleteProgram(diffuseShader);
}

void RenderScene(void)
{
    static CStopWatch rotTimer;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    modeoViewMatrix.PushMatrix(viewFrame);
    modeoViewMatrix.Rotate(rotTimer.GetElapsedSeconds() * 20, 0.0f, 1.0f, 0.0f);
    GLfloat vEyeLight[] = { -100.0f, 100.0f, 100.0f};
    GLfloat vDiffuseColor[] = {0.0f, 0.0f, 0.8f, 1.0f};
    GLfloat vSpecularColor[] = {0.0f, 0.0f, 1.0f, 1.0f};
    GLfloat vAmbientColor[] = {0.0f, 0.0f, 0.1f, 0.3f};
    glUseProgram(diffuseShader);
    glUniform4fv(locDiffColor, 1, vDiffuseColor);
    glUniform4fv(locSpecularColor, 1, vSpecularColor);
    glUniform4fv(locAmbientColor, 1, vAmbientColor);
    glUniform3fv(locLightPos, 1, vEyeLight);
    glUniformMatrix4fv(locMVPMatrix, 1, GL_FALSE, transformPipeline.GetModelViewProjectionMatrix());
    glUniformMatrix4fv(locMVMatrix, 1, GL_FALSE, transformPipeline.GetModelViewMatrix());
    glUniformMatrix3fv(locNormalMatrix, 1, GL_FALSE, transformPipeline.GetNormalMatrix());
    sphereBatch.Draw();
    modeoViewMatrix.PopMatrix();
    glutSwapBuffers();
    glutPostRedisplay();
}

void ChangeSize(int w, int h)
{
    glViewport(0, 0, w, h);
    viewFrustum.SetPerspective(35.0f, float(w) / float(h), 1.0f, 100.0f);
    projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
    transformPipeline.SetMatrixStacks(modeoViewMatrix, projectionMatrix);
}

int main(int argc, char * argv[])
{
    glewExperimental = GL_TRUE;
    gltSetWorkingDirectory(argv[0]);
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_SINGLE);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Diffuse Light");
    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderScene);
    
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
    }
    err = glGetError();
    
    SetupRC();
    glutMainLoop();
    ShutDownRC();
    return 0;
}
