//
//Brandon HObbs
// CS-330
// 6-17-2023
//

#include <GLFW\glfw3.h>
#include "linmath.h"
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <iostream>
#include <vector>
#include <windows.h>
#include <time.h>
// GLM Math Header inclusions
#include <glm/glm.hpp>


using namespace std;

const float DEG2RAD = 3.14159 / 180;

void processInput(GLFWwindow* window);
glm::vec2 preventStuckBalls(glm::vec2 vector);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

enum BRICKTYPE { REFLECTIVE, DESTRUCTABLE, PADDLE };
enum ONOFF { ON, OFF };
glm::vec2 normal = glm::vec2(0, 1);


class Brick
{
public:
	float red, green, blue;
	float x, y, width, height;
	BRICKTYPE brick_type;
	ONOFF onoff;
	//the bricks need health or else its too easy
	int health;

	Brick(BRICKTYPE bt, float xx, float yy, float ww, float hh, float rr, float gg, float bb, int _health)
	{
		brick_type = bt; x = xx; y = yy, width = ww, height = hh,  red = rr, green = gg, blue = bb; health = _health;
		onoff = ON;
	};

	void drawBrick()
	{
		if (onoff == ON)
		{

			double halfside = width / 2;
			double halfHeight = height / 2;

			glColor3d(red, green, blue);
			glBegin(GL_POLYGON);

			glVertex2d(x + halfside, y + halfHeight);
			glVertex2d(x + halfside, y - halfHeight);
			glVertex2d(x - halfside, y - halfHeight);
			glVertex2d(x - halfside, y + halfHeight);

			glEnd();
		}
	}
};

class Circle
{
public:
	float red, green, blue;
	float radius;
	float x;
	float y;
	float speed = 0.01;
	int direction; // 1=up 2=right 3=down 4=left 5 = up right   6 = up left  7 = down right  8= down left
	glm::vec2 vector; //velocity vector
	int life;

	Circle(double xx, double yy, int dir, float rad, float r, float g, float b, glm::vec2 _vector, int _life = 50)
	{
		x = xx;
		y = yy;
		red = r;
		green = g;
		blue = b;
		radius = rad;
		direction = dir;
		vector = _vector;
		life = _life;
	}

	bool CheckCollision(Brick* brk)
	{
		if (brk->brick_type == REFLECTIVE)
		{
			if ((x > brk->x - brk->width && x <= brk->x + brk->width) && (y > brk->y - brk->width && y <= brk->y + brk->width))
			{
				//nothing here
			}
		}
		else if (brk->brick_type == DESTRUCTABLE)
		{
			if ((x > brk->x - brk->width && x <= brk->x + brk->width) && 
				(y > brk->y - brk->width && y <= brk->y + brk->width) &&
				(brk->health >= 0))//does not count if "broken"
			{
				//ok, a ball made contact drop the block health by one
     			brk->health--;
				
				//now check if the block is out of health
				//if it is turn it off
				if(brk->health <= 0)
					brk->onoff = OFF;

				//now reflect the ball elsewhere
				glm::vec2 ballVector = glm::vec2(vector.x, vector.y);

				//a ball can bounce off the top, botto, or side of a block
				//the normal vector depends on the side
				//because it's too difficyult to determine what face the ball hits
				//set the normals randomly for fun
				direction = GetRandomDirection();

				switch (direction) {
				case 1:
					normal = glm::normalize(glm::vec2(-1.0f, 1.0f));
					break;
				case 2:
					normal = glm::normalize(glm::vec2(1.0f, 1.0f));
					break;
				case 3:
					normal = glm::normalize(glm::vec2(1.0f, -1.0f));
					break;
				case 4:
					normal = glm::normalize(glm::vec2(-1.0f, -1.0f));
					break;
				default:
					break;
				}

				vector = ballVector - 2 * glm::dot(ballVector, normal) * normal; //calculate the new reflected velcity vector

				speed *= 1.05; //speed up the ball for some chaos

				x += vector.x * speed;
				y += vector.y * speed;

				//only destroy the ball if the block is still there
				if (brk->onoff == ON)
					return true;
			}
		}
		// make a brick type called PADDLE
		// reuse the logic from the REFLECTIVE
		else if (brk->brick_type == PADDLE)
		{
			//x + radius should be between paddle +- width/2
			//y + radius should equal paddle + height/2

			//check to make sure the x coordinates of the ball are within the width
			//x >= brk->x - brk->width / 2 && x <= brk->x + brk->width/2
			//check to see if the ball has hit the paddle upper surafce
			//but not beyond the lower surface (speed can be weird)
			//y + radius < brk->y + brk->height / 2 && y >= brk->y - brk->height / 2
			// 
			// old
			//(x > brk->x - brk->width && x <= brk->x + brk->width) && (y > brk->y - brk->height && y <= brk->y + brk->height)
			if ((y <= brk->y + brk->height / 2 + radius && y > brk->y - brk->height / 2)
				&& (x > brk->x - brk->width / 2 && x < brk->x + brk->width / 2))
			{
				glm::vec2 ballVector = glm::vec2(vector.x, vector.y);
				normal = glm::vec2(0.0f, 1.0f);
				vector = ballVector - 2 * glm::dot(ballVector, normal) * normal; //calculate the new reflected velcoty vector
				//new position
				x += vector.x * speed;
				y += vector.y * speed;

				return true;
			}
		}

		return false;
	}

	int GetRandomDirection()
	{
		return (rand() % 4) + 1;
	}

	float MoveOneStep()
	{
		glm::vec2 ballVector = glm::vec2(vector.x, vector.y);
		//glm::vec2 reflection;

		//set the normal vectors for the walls
		//no need for the bottom wall	
		if (y > 1 + radius) {//upper wall
			normal = glm::vec2(0.0f, -1.0f);
			vector = ballVector - 2 * glm::dot(ballVector, normal) * normal;			
		}			
		else if (x > 1 + radius) {//right wall
			normal = glm::vec2(-1.0f, 0.0f);
			vector = ballVector - 2 * glm::dot(ballVector, normal) * normal;
		}			
		else if (x < -1 - radius) {//left wall
			normal = glm::vec2(1.0f, 0.0f);
			vector = ballVector - 2 * glm::dot(ballVector, normal) * normal;
		}

		vector = preventStuckBalls(vector);

		x += vector.x * speed;
		y += vector.y * speed;

		return y;
	}

	void DrawCircle()
	{
		glColor3f(red, green, blue);
		glBegin(GL_POLYGON);
		for (int i = 0; i < 360; i++) {
			float degInRad = i * DEG2RAD;
			glVertex2f((cos(degInRad) * radius) + x, (sin(degInRad) * radius) + y);
		}
		glEnd();
	}
};

vector<Brick> brickVector;
vector<Circle> world;
Brick paddle(PADDLE, -0.0, -0.9, 0.6, 0.1, 0.01, 0.06, 0.97, 1);

int main(void) {
	srand(time(NULL));

	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	GLFWwindow* window = glfwCreateWindow(480, 480, "BHobbs Random World of Circles", NULL, NULL);
	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	glfwSetKeyCallback(window, keyCallback);

	//easy bricks
	Brick brick2(DESTRUCTABLE, -1.0, 0.33, 0.2, 0.2, 0, 1, 0, 1);
	brickVector.push_back(brick2);

	Brick brick3(DESTRUCTABLE, -0.8, 0.33, 0.2, 0.2, 0, 1, 0, 4);
	brickVector.push_back(brick3);

	Brick brick4(DESTRUCTABLE, -0.6, 0.33, 0.2, 0.2, 0, 1, 0, 3);
	brickVector.push_back(brick4);

	Brick brick5(DESTRUCTABLE, 1.0, 0.33, 0.2, 0.2, 0, 1, 0, 2);
	brickVector.push_back(brick5);

	//medium bricks
	Brick brick6(DESTRUCTABLE, -0.2, 0.33, 0.2, 0.2, 0.66, 0.13, 0.56, 12);
	brickVector.push_back(brick6);

	Brick brick7(DESTRUCTABLE, -0.0, 0.33, 0.2, 0.2, 0.66, 0.13, 0.56, 16);
	brickVector.push_back(brick7);

	Brick brick8(DESTRUCTABLE, 0.2, 0.33, 0.2, 0.2, 0.66, 0.13, 0.56, 18);
	brickVector.push_back(brick8);

	Brick brick9(DESTRUCTABLE, 0.4, 0.33, 0.2, 0.2, 0.66, 0.13, 0.56, 13);
	brickVector.push_back(brick9);

	//hard bricks
	Brick brick10(DESTRUCTABLE, 0.6, 0.33, 0.2, 0.2, 1, 0, 0, 25);
	brickVector.push_back(brick10);

	Brick brick11(DESTRUCTABLE, 0.8, 0.33, 0.2, 0.2, 1, 0, 0, 30);
	brickVector.push_back(brick11);

	Brick brick12(DESTRUCTABLE, -.4, 0.33, 0.2, 0.2, 1, 0, 0, 20);
	brickVector.push_back(brick12);


	while (!glfwWindowShouldClose(window)) {
		//Setup View
		bool circleBreak = false;
		float ratio;
		int width, height;

		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float)height;
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);

		processInput(window);

		//Movement
		for (int i = 0; i < world.size(); i++)
		{
			//allow the balls to break if they hit a block
			//if the ball life is <=0 then break it
			for (vector<Brick>::iterator iter = brickVector.begin(); iter != brickVector.end(); iter++) {
				if (world[i].CheckCollision(&*iter)) {
					//remove one from life
					world[i].life--;
					if (world[i].life <= 0) {
						circleBreak = true;
						world.erase(world.begin() + i);
						break;
					}
				}
			}			
			//do not draw the ball if broken
			if (!circleBreak){						
				//check if the ball contacts the paddle
				//ifit does move
				if (world[i].CheckCollision(&paddle)) {
					world[i].MoveOneStep();
					world[i].DrawCircle();
				}
				//if it does not make contact
				else {
					//check if the circle is beyond the paddle
					//if it is delete it
					// else draw it
					if (world[i].MoveOneStep() < -1.0) {
						world[i].DrawCircle();
						world.erase(world.begin() + i);
					}
					else {
						world[i].MoveOneStep();
						world[i].DrawCircle();
					}
				}
			}
		}

		paddle.drawBrick();

		for (Brick iter : brickVector) {

			iter.drawBrick();
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate;
	exit(EXIT_SUCCESS);
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	// use arrow keys to move paddle left and right
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS && paddle.x > -1.0f)
		paddle.x -= 0.05;

	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS && paddle.x < 1.0f)
		paddle.x += 0.05;
}

glm::vec2 preventStuckBalls(glm::vec2 vector) {
	//prvent the circle from moving parallel to X-axis
	//if they do then they'll just bounce back and forth forever
	//so give them a bit of velocity in y
	if ((abs(vector.x) == 1) && abs(vector.y) == 0) {
		vector.x = 0.9;
		vector.y = -0.1;
	}
	else if ((abs(vector.x) == 0) && abs(vector.y) == 1) {
		vector.x = -0.1;
		vector.y = 0.9;
	}

	return vector;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
	{
		double r, g, b, vX, vY;
		int dir = (rand() % 8) + 1;

		//set the velocity values between -5 and 5
		vX = -5 + (rand() % 11);
		vY = -5 + (rand() % 11);
		//vX = .7;
		//vY = -.7;
		r = rand() / 10000;
		g = rand() / 10000;
		b = rand() / 10000;

		//prevent black circles
		if (r == 1.0 && g == 1.0)
			b == 0.0;

		//make all the vY negative so they head towards the paddle
		if (vY > 0)
			vY = -1 * vY;

		glm::vec2 circleDir = glm::vec2(vX, vY); //velocity vector
		circleDir = preventStuckBalls(circleDir); //make sure the balls do not bounce forever
		circleDir = glm::normalize(circleDir); //give it a speed of 1

		Circle B(0, 0, 1, 0.05, r, g, b, circleDir);
		world.push_back(B);
	}
}
