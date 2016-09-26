#ifdef _WINDOWS
	#include <GL/glew.h>
#endif
#include <SDL.h>

#include <SDL_opengl.h>
#include <SDL_image.h>

#include "ShaderProgram.h"

using namespace std;
#include <iostream>
#include <string>

#ifdef _WINDOWS
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;
GLuint LoadTexture(const string image);
int main(int argc, char *argv[])
{
	//enable transparency 
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
	#ifdef _WINDOWS
		glewInit();
	#endif
	
	//view size
	glViewport(0, 0, 640, 360);
	ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");

	//sprites
	GLuint marioTexture = LoadTexture("mario.jpg");
	GLuint mewtwoTexture = LoadTexture("medal.png");
	GLuint pikaTexture = LoadTexture("pika.jpg");

	//matrices
	Matrix projectionMatrix;
	Matrix modelMatrix;
	modelMatrix.Translate(1.0, 0.7, 0.0);
	modelMatrix.Rotate(45.0 * (3.1415926 / 180.0));
	Matrix viewMatrix;

	//for time keeping
	float lastFrameTicks = 0.0f;
	float angle = 0.0;

	//orthographic projection
	projectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
	glUseProgram(program.programID);

	SDL_Event event;
	bool done = false;
	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
		}
		
		//get the elapsed number of ticks
		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;
		angle += elapsed;

		//Drawing
		glClear(GL_COLOR_BUFFER_BIT);

		//mewtwo
		modelMatrix.Translate(1.9, 0.5, 0.0);
		modelMatrix.Rotate(angle * 20 * (3.1415926 / 180.0));
		program.setModelMatrix(modelMatrix);
		program.setProjectionMatrix(projectionMatrix);
		program.setViewMatrix(viewMatrix);
		glBindTexture(GL_TEXTURE_2D, mewtwoTexture);
		float vertices[] = { -0.5, -0.5, 0.5,
							 -0.5,  0.5, 0.5,
							 -0.5, -0.5, 0.5,
							  0.5, -0.5, 0.5 }; 
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program.positionAttribute);

		//texture coordinates
		float texCoords[] = { 0.0, 1.0, 1.0,
							  1.0, 1.0, 0.0,
							  0.0, 1.0, 1.0,
							  0.0, 0.0, 0.0 };
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		
		//mario
		modelMatrix.identity();
		program.setModelMatrix(modelMatrix);
		glBindTexture(GL_TEXTURE_2D, marioTexture);
		float verticesTwo[] = { -0.5, -0.5, 0.5,
								-0.5, 0.5, 0.5,
								-0.5, -0.5, 0.5,
								0.5, -0.5, 0.5 };
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, verticesTwo);
		glEnableVertexAttribArray(program.positionAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		//pikachu
		modelMatrix.Translate(1.1, 0.7, 0.0);
		program.setModelMatrix(modelMatrix);
		glBindTexture(GL_TEXTURE_2D, pikaTexture);
		float verticesThree[] = { -0.5, -0.5, 0.5,
								-0.5, 0.5, 0.5,
								-0.5, -0.5, 0.5,
								0.5, -0.5, 0.5 };
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, verticesThree);
		glEnableVertexAttribArray(program.positionAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);
		SDL_GL_SwapWindow(displayWindow);
		
	}

	SDL_Quit();
	return 0;
}

GLuint LoadTexture(const string image)
{
	//My image 
	//Note to self, image has to be in the same folder as the rest of these files
	//Folder found by right clicking the project and opening explorer
	SDL_Surface *surface = IMG_Load(image.c_str());
	string error = IMG_GetError();
	if (error != "")
		cerr << error;

	//create a new texture id
	GLuint textureID;
	glGenTextures(1, &textureID);

	//bind a texture to a texture target
	glBindTexture(GL_TEXTURE_2D, textureID);

	//Set the texture data of the specified texture target. Slight modifications depending on OS and image format
	#if defined(_WINDOWS)
		if (surface->format->BytesPerPixel == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
		else if (surface->format->BytesPerPixel == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, surface->w, surface->h, 0, GL_RGB, GL_UNSIGNED_BYTE, surface->pixels);
		else
			cerr << "Having trouble reading image, it will not work with this program";
	#else
		if (surface->format->BytesPerPixel == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_BGRA, surface->w, surface->h, 0, GL_BGRA, GL_UNSIGNED_BYTE, surface->pixels);
		else if (surface->format->BytesPerPixel == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_BGR, surface->w, surface->h, 0, GL_BGR, GL_UNSIGNED_BYTE, surface->pixels);
		else
			cerr << "Having trouble reading image, it will not work with this program";
	#endif	


	//Set a texture parameter of the specified texture target
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	SDL_FreeSurface(surface);

	return textureID;
}
