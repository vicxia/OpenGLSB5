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
GLBatch triangleBatch;
GLMatrixStack modeoViewMatrix;
GLMatrixStack projectionMatrix;
GLGeometryTransform transformPipeline;

GLint triangleShader;
GLuint textureID;
GLuint locDiffColor;
GLuint locAmbientColor;
GLuint locSpecularColor;
GLuint locLightPos;
GLuint locMVPMatrix;
GLuint locMVMatrix;
GLuint locNormalMatrix;

bool LoadTexture(const char *szFileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode)
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

void SetupRC()
{
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);

    GLfloat vVerts[] = {
        -0.5f, 0.0f, 0.0f,
         0.5f, 0.0f, 0.0f,
         0.0f, 0.5f, 0.0f
    };
    
    GLfloat vTexCoords[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.5f, 1.0f
    };
    triangleBatch.Begin(GL_TRIANGLES, 3, 1);
    triangleBatch.CopyVertexData3f(vVerts);
    triangleBatch.CopyTexCoordData2f(vTexCoords, 0);
    triangleBatch.End();
    
    triangleShader = gltLoadShaderPairWithAttributes("TexturedIdentityVertex.glsl", "TexturedIdentityFragment.glsl", 2, GLT_ATTRIBUTE_VERTEX, "vVertex", GLT_ATTRIBUTE_TEXTURE0, "vTexCoords");
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    LoadTexture("stone.tga", GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
}

void ShutDownRC()
{
    glDeleteProgram(triangleShader);
    glDeleteTextures(1, &textureID);
}

void RenderScene(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glUseProgram(triangleShader);
    glBindTexture(GL_TEXTURE_2D, textureID);
    GLint iTextureUniform = glGetUniformLocation(triangleShader, "colorMap");
    glUniform1i(iTextureUniform, 0);
    triangleBatch.Draw();
    glutSwapBuffers();
}


void ChangeSize(int w, int h)
{
    glViewport(0, 0, w, h);
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
