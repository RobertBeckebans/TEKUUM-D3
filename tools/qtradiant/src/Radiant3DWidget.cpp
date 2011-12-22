#include "Radiant3DWidget.h"

Radiant3DWidget::Radiant3DWidget(QWidget *parent) :
	QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{
	rotx = 0.0f;
	timer = new QTimer(this);
	timer->setInterval(10);
	timer->setSingleShot(false);
	timer->start();

	connect(timer, SIGNAL(timeout()), this, SLOT(onTimer()));
}

void Radiant3DWidget::onTimer()
{
	rotx += 1.0f;
	updateGL();
}

void Radiant3DWidget::initializeGL()
{
	qglClearColor(QColor(0, 0, 32, 0));

	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_TEXTURE_2D);
	//glEnable(GL_CULL_FACE);
	//glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glShadeModel(GL_SMOOTH);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glColor4f(1.0, 1.0, 1.0, 1.0);
}

void Radiant3DWidget::resizeGL(int width, int height)
{
	//int size = qMin(width, height);
	//glViewport((width - size) / 2, (height - size) / 2, size, size);
	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (GLfloat)width / (GLfloat)height, 0.1f, 1500.0 );
	/*
#ifdef QT_OPENGL_ES_1
	glOrthof(-0.5, +0.5, -0.5, +0.5, 4.0, 15.0);
#else
	glOrtho(-0.5, +0.5, -0.5, +0.5, 4.0, 15.0);
#endif
*/
	glMatrixMode(GL_MODELVIEW);
	paintGL();
}

void Radiant3DWidget::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	glEnable(GL_TEXTURE_2D);

	glTranslatef(0.0f, 0.0f, -2.0f);
	glRotatef(rotx, 0.5f, 1.0f, -0.5f);

	float size = 0.25f;
	glBegin(GL_QUADS);

	glColor3f(0.7f, 0.0f, 0.0f);
	glVertex3f(-size, -size, -size);
	glVertex3f( size, -size, -size);
	glVertex3f( size,  size, -size);
	glVertex3f(-size,  size, -size);

	glVertex3f(-size, -size,  size);
	glVertex3f( size, -size,  size);
	glVertex3f( size,  size,  size);
	glVertex3f(-size,  size,  size);

	glColor3f(0.0f, 0.0f, 0.7f);

	glVertex3f(-size, -size, -size);
	glVertex3f(-size, -size,  size);
	glVertex3f(-size,  size,  size);
	glVertex3f(-size,  size, -size);

	glVertex3f( size, -size, -size);
	glVertex3f( size, -size,  size);
	glVertex3f( size,  size,  size);
	glVertex3f( size,  size, -size);

	glColor3f(0.0f, 0.7f, 0.0f);

	glVertex3f(-size, -size, -size);
	glVertex3f(-size, -size,  size);
	glVertex3f( size, -size,  size);
	glVertex3f( size, -size, -size);

	glVertex3f(-size, size, -size);
	glVertex3f(-size, size,  size);
	glVertex3f( size, size,  size);
	glVertex3f( size, size, -size);

	glEnd();
}
