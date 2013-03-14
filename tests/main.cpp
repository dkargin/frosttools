#include "stdafx.h"
#include "datarenderer.h"
#include <gl\glut.h>
#include <stdlib.h>
#include <malloc.h>
#include <time.h>
#include <math.h>

void Init( void );

DataRenderer renderer(0);

void Display( void )
{
	renderer.start();
	renderer.render();
	renderer.finish();	
}

void Keyboard( unsigned char key, int x, int y )
{
	float tFire;
	switch( key )
	{
	case 'q':		
		exit(0);
		break;	
	default:
		return;

	}
	glutPostRedisplay();
}
void Mouse( int button, int state, int x, int y )
{
	if (state == GLUT_DOWN)
	{
		if(button == GLUT_LEFT_BUTTON)
		{}
		else if (button == GLUT_RIGHT_BUTTON)
		{}
	}
}

void Reshape( int w, int h )
{
	renderer.setViewport(0,0,w,h);
	//glViewport( 0,0, w, h );
	glutPostRedisplay();
}

void Idle( void )
{	
	glutPostRedisplay();
}



void Init( void )
{

	GLfloat ambientLight[] = {0.2f, 0.2f, 0.2f, 1.0f};
	GLfloat diffuseLight[] = {0.5f, 0.5f, 0.5f, 1.0f};

	srand( (unsigned)time(0) );

	glDepthFunc( GL_LESS );
	glEnable( GL_DEPTH_TEST );
	glEnable( GL_CULL_FACE );
	glCullFace( GL_BACK );
	glShadeModel(GL_SMOOTH);	
	
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);	
}

int window_width=400;
int window_height=300;
int main(int argc, char * argv[])
{
	glutInit( &argc, argv );
	glutInitWindowSize( window_width, window_height );
	glutInitWindowPosition( 50, 50 );
	glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL );
	int window=glutCreateWindow( "FrostTools testing" );
	

	Init();


	glutDisplayFunc( Display );
	glutKeyboardFunc( Keyboard );
	glutMouseFunc( Mouse );
	glutReshapeFunc( Reshape );
	glutIdleFunc( Idle );
	glutMainLoop();

	return 0;

}