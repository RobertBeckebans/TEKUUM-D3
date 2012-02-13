/*
===========================================================================

Copyright (C) 2012 Robert Beckebans

===========================================================================
*/

#ifndef __FRAMEBUFFER_H__
#define __FRAMEBUFFER_H__

class Framebuffer
{
public:

							Framebuffer(const char *name, int width, int height);

	static void				Init();
	static void				Shutdown();

	// deletes OpenGL object but leaves structure intact for reloading
	void					PurgeFramebuffer();

	void					Bind();
	static void				BindNull();

	void					AddColorBuffer(int format, int index);
	void					AddDepthBuffer(int format);

	void					AttachImage2D(int target, const idImage* image, int index);
	void					AttachImage3D(const idImage* image);
	void					AttachImageDepth(const idImage* image);

	// check for OpenGL errors
	void					Check();

private:
	idStr					fboName;

	// FBO object
	uint32_t				frameBuffer;

	uint32_t				colorBuffers[16];
	int						colorFormat;

	uint32_t				depthBuffer;
	int						depthFormat;

	uint32_t				stencilBuffer;
	int						stencilFormat;

	int						width;
	int						height;

	static idList<Framebuffer*>	framebuffers;
};

#endif // __FRAMEBUFFER_H__