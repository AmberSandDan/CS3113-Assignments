//Jonathan Avezbaki
#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>

#include <cmath>
#include <SDL_opengl.h>
#include <SDL_image.h>

#include "ShaderProgram.h"

using namespace std;
#include <string>

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif


struct Paddle
{
	string image;
	float paddleY;
	GLuint texture;
	Paddle(string img) : image(img), paddleY(0){};
};

struct Ball
{
	Ball(string img, float _ballX, float _ballY, float vBallX, float vBallY)
		: image(img), ballX(_ballX), ballY(_ballY), velocityBallX(vBallX), velocityBallY(vBallY) {};
	string image;
	float ballX;
	float ballY;
	float velocityBallX;
	float velocityBallY;
	GLuint texture;
};

SDL_Window* displayWindow;
ShaderProgram* program;
GLuint LoadTexture(const string image);


//matrices
Matrix projectionMatrix;
Matrix modelMatrix;
Matrix viewMatrix;

//time keeping
float elapsed = 0.0;
float pastElapsed = 0.0;
float deltaElapsed = 0.0;
int counter = 0;
float lastFrameTicks;

SDL_Joystick * playerTwoController = SDL_JoystickOpen(0);

//Function to get change in time from last marked time
//only updates on counter condition b/c otherwise it 
//calculates ridiculously small values for delta
float getDeltaTime()
{
	++counter;
	if (counter % 100 == 0)
	{
		deltaElapsed = elapsed - pastElapsed;
		pastElapsed = elapsed;
	}
	return deltaElapsed;
}

void CollisionDetection(Paddle &padOne, Paddle &padTwo, Ball &ballOne)
{
	//Collision detection for window
	if (ballOne.ballY >= 4.6 || ballOne.ballY <= -4.6)
		ballOne.velocityBallY *= -1;

	//left paddle hit
	if (ballOne.ballX <= -7.38 && abs(ballOne.ballY - padOne.paddleY) > 0.1 && abs(ballOne.ballY - padOne.paddleY) < 2.0)
	{
		ballOne.velocityBallX *= -1;
	}

	//right paddle hit
	if (ballOne.ballX >= 7.38 && abs(ballOne.ballY - padOne.paddleY) > 0.3 && abs(ballOne.ballY - padTwo.paddleY) < 2.0)
	{
		ballOne.velocityBallX *= -1;
	}
	/*if (ballOne.ballX >= 7.38)
	{
		if (ballOne.ballY > padTwo.paddleY)
		{
			if (ballOne.ballY - 0.5 < padTwo.paddleY + 0.9)
				ballOne.velocityBallX *= -1;
		}
		else
		{
			if (ballOne.ballY + 0.5 > padTwo.paddleY - 0.9)
				ballOne.velocityBallX *= -1;
		}
	}*/

	//win condition
	if (ballOne.ballX >= 8.45 || ballOne.ballX <= -8.45)
	{
		ballOne.ballX = 0.0;
		ballOne.ballY = 0.0;
	}
}

void DrawObjects(Paddle padOne, Paddle padTwo, Ball ballOne)
{
	//for drawing
	float texCoords[] = { 0.0, 1.0, 1.0,
		1.0, 1.0, 0.0,
		0.0, 1.0, 1.0,
		0.0, 0.0, 0.0 };

	float vertices[] = { -0.5, -0.5, 0.5,
		-0.5, 0.5, 0.5,
		-0.5, -0.5, 0.5,
		0.5, -0.5, 0.5 };

	program->setModelMatrix(modelMatrix);
	program->setProjectionMatrix(projectionMatrix);
	program->setViewMatrix(viewMatrix);

	//texture coordinates
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program->texCoordAttribute);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	//paddle #1
	modelMatrix.identity();
	modelMatrix.Translate(-3.2, padOne.paddleY, 0.0);
	modelMatrix.Scale(0.4, 1.0, 1.0);
	program->setModelMatrix(modelMatrix);
	glBindTexture(GL_TEXTURE_2D, padOne.texture);
	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program->positionAttribute);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	//paddle #2
	modelMatrix.identity();
	modelMatrix.Translate(3.2, padTwo.paddleY, 0.0);
	modelMatrix.Scale(0.4, 1.0, 1.0);
	program->setModelMatrix(modelMatrix);
	glBindTexture(GL_TEXTURE_2D, padTwo.texture);
	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program->positionAttribute);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	//ball
	modelMatrix.identity();
	modelMatrix.Scale(0.4, 0.4, 0.4);
	modelMatrix.Translate(ballOne.ballX, ballOne.ballY, 0.0);
	program->setModelMatrix(modelMatrix);
	glBindTexture(GL_TEXTURE_2D, ballOne.texture);
	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program->positionAttribute);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}

void Setup(Paddle &padOne, Paddle &padTwo, Ball &ballOne)
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Pong", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif

	//view size
	glViewport(0, 0, 640, 360);
	program = new ShaderProgram(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");

	//Allow transparency
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//sprites
	padTwo.texture = LoadTexture(padTwo.image);
	padOne.texture = LoadTexture(padOne.image);
	ballOne.texture = LoadTexture(ballOne.image);

	//for time keeping
	lastFrameTicks = 0.0f;

	//orthographic projection
	projectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
	glUseProgram(program->programID);
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


int main(int argc, char *argv[])
{
	Paddle padOne = Paddle("paddle.png");
	Paddle padTwo = Paddle("paddle.png");
	Ball ballOne = Ball("ball.png", 0.0f, 0.0f, 0.001f, 0.001f);

	Setup(padOne, padTwo, ballOne);

	SDL_Event event;
	bool done = false;
	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
			else if (event.type == SDL_KEYDOWN){
				float value = 0.0;
				switch (event.key.keysym.sym){
				case 119:
					value = 0.06;
					break;
				case 115:
					value = -0.06;
					break;
				default:
					break;
				}
				if (!(padOne.paddleY > 1.49 && value > 0 || padOne.paddleY < -1.49 && value < 0))
					padOne.paddleY += value;
			}
			if (event.type == SDL_MOUSEWHEEL)
			{
				Sint32 buttonPress = event.wheel.y;
				if (!((padTwo.paddleY > 1.49 && buttonPress > 0) || (padTwo.paddleY < -1.49 && buttonPress < 0)))
					padTwo.paddleY += (buttonPress > 0) ? 0.06 : -0.06;
			}
		}

		//get the elapsed number of ticks
		float ticks = (float)SDL_GetTicks() / 1000.0f;
		elapsed = ticks - lastFrameTicks;
		ballOne.ballX += (getDeltaTime() * 50 * ballOne.velocityBallX);
		ballOne.ballY += (getDeltaTime() * 50 * ballOne.velocityBallY);

		glClear(GL_COLOR_BUFFER_BIT);

		DrawObjects(padOne, padTwo, ballOne);
		CollisionDetection(padOne, padTwo, ballOne);

		SDL_GL_SwapWindow(displayWindow);

	}

	SDL_JoystickClose(playerTwoController);
	SDL_Quit();
	return 0;
}

