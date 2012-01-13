/*
===========================================================================

Techyon GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company. 
Copyright (C) 2011 Robert Beckebans

This file is part of the Techyon GPL Source Code (Doom 3 Source Code).

Techyon Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Techyon Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Techyon Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Techyon Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Techyon Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#include <QTimer>

#include "GameGLWidget.h"

#include "../../idlib/precompiled.h"
#include "../../renderer/tr_local.h"
#pragma hdrstop

GameGLWidget::GameGLWidget():
	QGLWidget(QGLFormat(QGL::SampleBuffers))
{
	timer = new QTimer(this);
	// every 10 milliseconds = 100 fps
	timer->setInterval(10);
	timer->setSingleShot(false);
	timer->start();

	connect(timer, SIGNAL(timeout()), this, SLOT(onTimer()));
}

void GameGLWidget::keyPressEvent(QKeyEvent *event)
{

}

void GameGLWidget::mousePressEvent(QMouseEvent *event)	
{

}

void GameGLWidget::mouseMoveEvent(QMouseEvent *event)
{

}
	
void GameGLWidget::initializeGL()
{
	glConfig.colorBits = format().redBufferSize() + format().greenBufferSize() + format().blueBufferSize();
	glConfig.depthBits = format().depthBufferSize();
	glConfig.stencilBits = format().stencilBufferSize();
}

void GameGLWidget::paintGL()
{
#ifdef DEBUG
	Sys_MemFrame();
#endif

	//makeCurrent();

	// run the game
	common->Frame();

	//doneCurrent();
}

void GameGLWidget::resizeGL(int width, int height)
{
	glConfig.vidWidth = width;
	glConfig.vidHeight = height;
}

void GameGLWidget::onTimer()
{
	updateGL();
}