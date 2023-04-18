// Ray Tracer Assignment OpenGL Code
// You probably don't need to modify this for the assignment
// Winter 2021


#include "common.h"
#include "raytracer.h"

#define STB_IMAGE_IMPLEMENTATION
#include "utils/stb_image.h"

#include <iostream>
#define M_PI 3.14159265358979323846264338327950288
#include <cmath>

#include <glm/glm.hpp>

BS::thread_pool pool;
std::chrono::steady_clock::time_point t1;
bool finished = false;
const char *WINDOW_TITLE = "Ray Tracing";
const double FRAME_RATE_MS = 1;

colour3 texture[1<<16]; // big enough for a row of pixels
point3 vertices[2]; // xy+u for start and end of line
GLuint Window;
int vp_width, vp_height;
float drawing_y = 0;

point3 eye;
float d = 1;

//----------------------------------------------------------------------------

point3 s(int x, int y) {
	float aspect_ratio = (float)vp_width / vp_height;
	float h = d * (float)tan((M_PI * fov) / 180.0 / 2.0);
	float w = h * aspect_ratio;
   
	float top = h;
	float bottom = -h;
	float left = -w;
	float right = w;
   
	float u = left + (right - left) * (x + 0.5f) / vp_width;
	float v = bottom + (top - bottom) * (y + 0.5f) / vp_height;
   
	return point3(u, v, -d);
}

//----------------------------------------------------------------------------

// OpenGL initialization
void init(char *fn) {
	choose_scene(fn);
   
	// Create a vertex array object
	GLuint vao;
	glGenVertexArrays( 1, &vao );
	glBindVertexArray( vao );

	// Create and initialize a buffer object
	GLuint buffer;
	glGenBuffers( 1, &buffer );
	glBindBuffer( GL_ARRAY_BUFFER, buffer );
	glBufferData( GL_ARRAY_BUFFER, sizeof(vertices), NULL, GL_STATIC_DRAW );

	// Load shaders and use the resulting shader program
	GLuint program = InitShader( "v.glsl", "f.glsl" );
	glUseProgram( program );

	// set up vertex arrays
	GLuint vPos = glGetAttribLocation( program, "vPos" );
	glEnableVertexAttribArray( vPos );
	glVertexAttribPointer( vPos, 3, GL_FLOAT, GL_FALSE, 0, 0 );

	Window = glGetUniformLocation( program, "Window" );

	// glClearColor( background_colour[0], background_colour[1], background_colour[2], 1 );
	glClearColor( 0.7, 0.7, 0.8, 1 );

	// set up a 1D texture for each scanline of output
	GLuint texture;
	glGenTextures( 1, &texture);
	glBindTexture( GL_TEXTURE_1D, texture);
	glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

	t1 = std::chrono::high_resolution_clock::now();

	std::cout << "-----------------------------" << std::endl;
	std::cout << "Threadpool size: " << pool.get_thread_count() << std::endl;
}

//----------------------------------------------------------------------------

void display( void ) {
	// draw one scanline at a time, to each buffer; only clear when we draw the first scanline
	// (when fract(drawing_y) == 0.0, draw one buffer, when it is 0.5 draw the other)
	
	if (drawing_y <= 0.5) {
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		glFlush();
		glFinish();
		glutSwapBuffers();

		drawing_y += 0.5;

	} else if (drawing_y >= 1.0 && drawing_y <= vp_height + 0.5) {
		int y = int(drawing_y) - 1;

		// only recalculate if this is a new scanline
		if (drawing_y == int(drawing_y)) {

			//https://github.com/bshoshany/thread-pool

			auto loop = [y](const int a, const int b)
			{
				for (int i = a; i < b; ++i) {
					bool res = trace(eye, s(i, y), texture[i], false);
					if (!res) {
						texture[i] = background_colour;
					}
				}
			};
			auto futures = pool.parallelize_loop(vp_width, loop);

			/*for (int x = 0; x < vp_width; x++) {
				bool res = trace(eye, s(x, y), texture[x], false);
				if (!res) {
					texture[x] = background_colour;
				}
			}*/

			// to ensure a power-of-two texture, get the next highest power of two
			// https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
			unsigned int v; // compute the next highest power of 2 of 32-bit v
			v = vp_width;
			v--;
			v |= v >> 1;
			v |= v >> 2;
			v |= v >> 4;
			v |= v >> 8;
			v |= v >> 16;
			v++;

			futures.wait();
			glTexImage1D( GL_TEXTURE_1D, 0, GL_RGB, v, 0, GL_RGB, GL_FLOAT, texture );
			vertices[0] = point3(0, y, 0);
			vertices[1] = point3(v, y, 1);
			glBufferSubData( GL_ARRAY_BUFFER, 0, 2 * sizeof(point3), vertices);
		}
		glDrawArrays( GL_LINES, 0, 2 );
		
		glFlush();
		glFinish();
		glutSwapBuffers();
		
		drawing_y += 0.5;
	}
	else {
		if (finished == false) {
			finished = true;
			auto t2 = std::chrono::high_resolution_clock::now();
			auto time = t2 - t1;
			auto time_sec = std::chrono::duration_cast<std::chrono::seconds>(time);
			auto time_min = std::chrono::duration_cast<std::chrono::minutes>(time);
			auto time_hour = std::chrono::duration_cast<std::chrono::hours>(time);
			std::cout << "Render time: " << time_hour.count() << " hours / " << time_min.count() << " mins / " << time_sec.count() << " secs" << std::endl;
		}
	}


}

//----------------------------------------------------------------------------

void keyboard( unsigned char key, int x, int y ) {
	switch( key ) {
	case 033: // Escape Key
	case 'q': case 'Q':
		exit( EXIT_SUCCESS );
		break;
	case ' ':
		drawing_y = 1;
		break;
	}
}

//----------------------------------------------------------------------------

void mouse( int button, int state, int x, int y ) {
	y = vp_height - y - 1;
	if ( state == GLUT_DOWN ) {
		switch( button ) {
		case GLUT_LEFT_BUTTON:
			colour3 c;
			point3 uvw = s(x, y);
			std::cout << std::endl;
			if (trace(eye, uvw, c, true)) {
				std::cout << "HIT @ ( " << uvw.x << "," << uvw.y << "," << uvw.z << " )\n";
				std::cout << "      colour = ( " << c.r << "," << c.g << "," << c.b << " )\n";
			} else {
				std::cout << "MISS @ ( " << uvw.x << "," << uvw.y << "," << uvw.z << " )\n";
			}
			break;
		}
	}
}

//----------------------------------------------------------------------------

void update( void ) {
}

//----------------------------------------------------------------------------

void reshape( int width, int height ) {
	glViewport( 0, 0, width, height );

	// GLfloat aspect = GLfloat(width)/height;
	// glm::mat4  projection = glm::ortho( -aspect, aspect, -1.0f, 1.0f, -1.0f, 1.0f );
	// glUniformMatrix4fv( Projection, 1, GL_FALSE, glm::value_ptr(projection) );
	vp_width = width;
	vp_height = height;
	glUniform2f( Window, width, height );
	drawing_y = 0;
}
