//
//  Sprites.cpp
//  VXOpenGL_EX
//
//  Created by vicxia on 30/10/2017.
//  Copyright © 2017 vicxia. All rights reserved.
//

#include <GLTools.h>
#include <GLFrustum.h>
#include <StopWatch.h>
#include <math.h>
#include <stdlib.h>
#include <GLUT/GLUT.h>

GLFrustum viewFrustum;
GLBatch starBatch;

GLuint starShader;
GLint locMVPMatrix;
GLint locTimeStamp;
GLint locTexture;

GLuint starTexture;

#define NUM_STARS 10000

bool LoadTGATexture(const char *szFileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode)
{
    GLbyte *pBits;
    int nWidth, nHeight, nComponents;
    GLenum eFormat;
    
    // Read the texture bits
    pBits = gltReadTGABits(szFileName, &nWidth, &nHeight, &nComponents, &eFormat);
    if(pBits == NULL)
        return false;
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, nComponents, nWidth, nHeight, 0,
                 eFormat, GL_UNSIGNED_BYTE, pBits);
    
    free(pBits);
    
    if(minFilter == GL_LINEAR_MIPMAP_LINEAR ||
       minFilter == GL_LINEAR_MIPMAP_NEAREST ||
       minFilter == GL_NEAREST_MIPMAP_LINEAR ||
       minFilter == GL_NEAREST_MIPMAP_NEAREST)
        glGenerateMipmap(GL_TEXTURE_2D);
    
    return true;
}

void SetupRC(void)
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_POINT_SPRITE);
    GLfloat fColors[4][4] = {{ 1.0f, 1.0f, 1.0f, 1.0f}, // White
                            { 0.67f, 0.68f, 0.82f, 1.0f}, // Blue Stars
                            { 1.0f, 0.5f, 0.5f, 1.0f}, // Reddish
                            { 1.0f, 0.82f, 0.65f, 1.0f}}; // Orange
    
    starBatch.Begin(GL_POINTS, NUM_STARS);
    for (int i = 0; i < NUM_STARS; i++) {
        int iColors = 0;
        if (rand() % 5 == 1) {
            iColors = 1;
        } else if (rand() % 50 == 1) {
            iColors = 2;
        } else if (rand() % 100 == 1) {
            iColors = 3;
        }
        starBatch.Color4fv(fColors[iColors]);
        M3DVector3f vPosition;
        vPosition[0] = float(3000 - rand() % 6000) * 0.1f;
        vPosition[1] = float(3000 - rand() % 6000) * 0.1f;
        vPosition[2] = -float(rand() % 1000) - 1.0f;
        starBatch.Vertex3fv(vPosition);
    }
    starBatch.End();
    
    starShader = gltLoadShaderPairWithAttributes("SpritesVS.glsl", "SpritesFS.glsl", 2, GLT_ATTRIBUTE_VERTEX, "vVertex", GLT_ATTRIBUTE_COLOR, "vColor");
    
    locMVPMatrix = glGetUniformLocation(starShader, "mvpMatrix");
    locTimeStamp = glGetUniformLocation(starShader, "timeStamp");
    locTexture = glGetUniformLocation(starShader, "starImage");
    
    glGenTextures(1, &starTexture);
    glBindTexture(GL_TEXTURE_2D, starTexture);
    LoadTGATexture("star.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
}

void ShutdownRC(void)
{
    glDeleteTextures(1, &starTexture);
}

void RenderScene(void)
{
    static CStopWatch timer;
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glEnable(GL_PROGRAM_POINT_SIZE);
    
    glUseProgram(starShader);
    glUniformMatrix4fv(locMVPMatrix, 1, GL_FALSE, viewFrustum.GetProjectionMatrix());
    glUniform1i(locTexture, 0);
    float fTime = timer.GetElapsedSeconds() * 10.0f;
    fTime = fmod(fTime, 999.0f);
    glUniform1f(locTimeStamp, fTime);
    
    starBatch.Draw();
    
    glutSwapBuffers();
    glutPostRedisplay();
}

void ChangeSize(int w, int h)
{
    glViewport(0, 0, w, h);
    viewFrustum.SetPerspective(35.0f, float(w) / float(h), 1.0f, 1000.0f);
}

int main(int argc, char* argv[])
{
    glewExperimental = GL_TRUE;
    gltSetWorkingDirectory(argv[0]);
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Spaced Out");
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
