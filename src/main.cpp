#define GL_GLEXT_PROTOTYPES

#include <iostream>
#include <SDL2/SDL.h>
#include <cstring>
#include <GL/gl.h>
#include <GL/glext.h>

#include "../include/glm/glm.hpp"
#include "../include/FileLoader.h"
#include "../include/Shader.h"
#include "../include/ShaderProgram.h"

using namespace std;
using namespace glm;

const float triMesh[9] = {-1.0f, -1.0f, 0.0f,
                           0.0f,  1.0f, 0.0f,
                           1.0f, -1.0f, 0.0f};

const float screenMesh[18] = {-1.0f, -1.0f, 0.0f,
                               1.0f, -1.0f, 0.0f,
                               1.0f,  1.0f, 0.0f,
                               1.0f,  1.0f, 0.0f,
                              -1.0f,  1.0f, 0.0f,
                              -1.0f, -1.0f, 0.0f};



const int width = 840, height = 680;

GLuint vboId, vaoId, finalVboId, finalVaoId, frameBuffId, colorTexId, realTex, depthTexId,
       shaderProgramId, vShaderId, fShaderId;

Shader *vshader, *fshader, *finalVShader, *finalFShader;
ShaderProgram *program, *finalProgram;

vector<vec3> vertexPos, vertexNormals;
vector<vec2> vertexUv;
bool triangles;

void Init()
{
    glEnable(GL_DEPTH_TEST);

    glClearColor(0.0, 0.0, 0.0, 1.0);

    ReadOBJ("luigi.obj", vertexPos, vertexUv, vertexNormals, triangles);

    int modelDataSize = vertexPos.size() * sizeof(vec3) + vertexUv.size() * sizeof(vec2);
    int vertexUvOffset = vertexPos.size() * sizeof(vec3);
    char *modelData = new char[modelDataSize];
    memcpy(modelData, &vertexPos[0], vertexUvOffset);
    memcpy((modelData + vertexUvOffset), &vertexUv[0], vertexUv.size() * sizeof(vec2));

    //Creamos shaders
    vshader = new Shader(); if( !vshader->Create("vshader", VertexShader) ) std::cout << "FUUUU" << std::endl;
    fshader = new Shader(); if( !fshader->Create("fshader", FragmentShader) ) std::cout << "FUUUU" << std::endl;
    program = new ShaderProgram();
    program->AttachShader(*vshader);
    program->AttachShader(*fshader);
    program->Link();

    //Creamos vbo
    glGenBuffers(1, &vboId);
    glBindBuffer(GL_ARRAY_BUFFER, vboId);
    glBufferData(GL_ARRAY_BUFFER, vertexPos.size() * sizeof(vec3) + vertexUv.size() * sizeof(vec2), &modelData[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //real texture
    glGenTextures(1, &realTex);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, realTex);
    unsigned char *data = 0;
    int n, texWidth, texHeight;
    data = ReadTexture("luigiD.jpg", n, texWidth, texHeight);
    glTexImage2D(GL_TEXTURE_2D, 0, (n == 4 ? GL_RGBA : GL_RGB), texWidth, texHeight, 0,
                 (n == 4 ? GL_RGBA : GL_RGB), GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D, 0);

    //Creamos vao
    glEnableClientState(GL_VERTEX_ARRAY);
        glGenVertexArrays(1, &vaoId);

        glBindVertexArray(vaoId);

            glBindBuffer(GL_ARRAY_BUFFER, vboId);

                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
                glEnableVertexAttribArray(0);

                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*) (vertexPos.size() * sizeof(vec3)));
                glEnableVertexAttribArray(1);

            glBindBuffer(GL_ARRAY_BUFFER, 0);

        glBindVertexArray(0);
    glDisableClientState(GL_VERTEX_ARRAY);

    /// Creamos framebuffer/////
    //color texture
    glGenTextures(1, &colorTexId);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorTexId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D, 0);

    //depthTexture
    glGenTextures(1, &depthTexId);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, depthTexId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    //framebuffer
    glGenFramebuffers(1, &frameBuffId);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffId);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexId, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,  GL_TEXTURE_2D, depthTexId, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //Creamos shaders finales
    finalVShader = new Shader(); if( !finalVShader->Create("finalVShader", VertexShader) ) std::cout << "FUUUU" << std::endl;
    finalFShader = new Shader(); if( !finalFShader->Create("finalFShader", FragmentShader) ) std::cout << "FUUUU" << std::endl;
    finalProgram = new ShaderProgram();
    finalProgram->AttachShader(*finalVShader);
    finalProgram->AttachShader(*finalFShader);
    finalProgram->Link();

    //Creamos vbo final
    glGenBuffers(1, &finalVboId);
    glBindBuffer(GL_ARRAY_BUFFER, finalVboId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(screenMesh), screenMesh, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //Creamos vao final
    glEnableClientState(GL_VERTEX_ARRAY);
    glGenVertexArrays(1, &finalVaoId);
    glBindVertexArray(finalVaoId);
    glBindBuffer(GL_ARRAY_BUFFER, finalVboId);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(glGetAttribLocation(finalProgram->GetId(), "position"));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glDisableClientState(GL_VERTEX_ARRAY);

}

float appTime = 0.0, rot = 0.0;

void RenderScene()
{
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffId);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(vaoId);
    program->Use();

    glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, realTex);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glDrawBuffer(GL_DEPTH_ATTACHMENT);

    mat4 transform(1.0f);
    vec3 axis(0.0, 1.0, 0.0), axis2(1.0, 0.0, 0.0), translate(0.0, -2.0, -10.0), scale(0.03);
    mat4 T = glm::translate(transform, translate);
    mat4 R = glm::rotate_slow(transform, rot, axis);
    mat4 R2 = glm::rotate_slow(transform, rot, axis2);
    mat4 S = glm::scale(transform, scale);
    transform = T * R * S;

    mat4 projection(1.0f);
    projection = perspective(45.0f * 3.1415f/180.0f, 1.0f, 0.1f, 100.0f);

    glUniform1f(glGetUniformLocation(program->GetId(), "time"), appTime);
    glUniform1i(glGetUniformLocation(program->GetId(), "realTex"), 2);
    glUniformMatrix4fv(glGetUniformLocation(program->GetId(), "transform"), 1, GL_FALSE, value_ptr(transform));
    glUniformMatrix4fv(glGetUniformLocation(program->GetId(), "projection"), 1, GL_FALSE, value_ptr(projection));

    glDrawArrays(triangles ? GL_TRIANGLES : GL_QUADS, 0, vertexPos.size());

    program->UnUse();
    glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    /// RENDER FINAL IMAGE /////////////////
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(finalVaoId);
    finalProgram->Use();
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, colorTexId);
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, depthTexId);

    glUniform1i(glGetUniformLocation(finalProgram->GetId(), "renderedSceneTex"), 0);
    glUniform1i(glGetUniformLocation(finalProgram->GetId(), "depthSceneTex"), 1);
    glUniform1f(glGetUniformLocation(finalProgram->GetId(), "width"), width);
    glUniform1f(glGetUniformLocation(finalProgram->GetId(), "height"), height);
    glUniform1f(glGetUniformLocation(finalProgram->GetId(), "time"), appTime);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    finalProgram->UnUse();
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    //////////////////////////////////

    appTime += 0.03;
    rot += 0.04;
}


int main(int argc, char* argv[])
{
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window* win;
    win = SDL_CreateWindow("Pruebas OpenGL", 100, 100, width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);

    SDL_GLContext context;
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    context = SDL_GL_CreateContext(win);

    Init();
    bool running = true;
    while(running)
    {
        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            if(event.type == SDL_QUIT)
                running = false;
        }

        RenderScene();
        SDL_GL_SwapWindow(win);
        SDL_Delay(25);
    }

    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;
}
