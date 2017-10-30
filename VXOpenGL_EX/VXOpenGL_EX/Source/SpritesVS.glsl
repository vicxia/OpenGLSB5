#version 410

in vec4 vVertex;
in vec4 vColor;

uniform mat4 mvpMatrix;
uniform float timeStamp;

out vec4 vStarColor;

void main(void)
{
    vec4 vNewVertex = vVertex;
    vStarColor = vColor;
    vNewVertex.z += timeStamp;
    if (vNewVertex.z > -1.0) {
        vNewVertex.z -= 999.0;
    }
    
    gl_PointSize = 30 - sqrt(-vNewVertex.z);
    if (gl_PointSize < 4.0) {
        vStarColor = smoothstep(0.0, 4.0, gl_PointSize) * vStarColor;
    }
    
    gl_Position = mvpMatrix * vNewVertex;
}
