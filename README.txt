
__________________________________

1) COMPILING ON GNU/LINUX
_________________________


1. You need the following dependencies in order to compile Techyon with all features:
 
	On Debian or Ubuntu:
		TODO

		> apt-get install libcurl4-openssl-dev libsdl1.2-dev libxxf86dga-dev libxxf86vm-dev libglu1-mesa-dev
	
	On Fedora

		> yum install openal-soft-devel alsa-lib-devel libX11-devel libXxf86dga-devel libXxf86vm-devel mesa-libGL-devel mesa-libGLU-devel libcurl-devel


2. Download and extract Premake 4.x to the Techyon/src directory or install it using your
	Linux distribution's package system.

3. Generate the Makefiles using Premake:

	> ./premake4 gmake 
	
4. Compile Techyon targets with

	> make

If you want to build for x86_64 then type:

	> make config=release64


Type "./premake4 --help" or "make help" for more compile options.
