/*
 * Kwaak3 - Java to quake3 interface
 *
 * Copyright (C) 2010 Roderick Colenbrander
 * Copyright (C) 2012 Robert Beckebans
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <android/log.h>

//#ifndef DEBUG
//#define DEBUG 1
//#endif

//#define USE_JNI

#include "com_robertbeckebans_tekuum_TekuumJNI.h"
#include "tekuumjni_public.h"

// structure containing functions exported by the engine
jniExport_t je;

/* Callbacks to Android */
jmethodID android_setMenuState;
jmethodID android_requestRender;

// handles to class com.robertbeckebans.tekuum.Tekuum
static jclass class_Tekuum = NULL;
static jobject object_Tekuum = NULL;
static jmethodID method_Tekuum_showProgress;
static jmethodID method_Tekuum_hideProgress;
//static jmethodID method_Tekuum_setProgressText;

/* Contains the game directory e.g. /mnt/sdcard/tekuum */
static char* game_dir = NULL;

/* Containts the path to /data/data/(package_name)/libs */
static char* lib_dir = NULL;

static JavaVM* jVM;
static jboolean audioEnabled = 1;
static jboolean benchmarkEnabled = 0;
static jboolean lightmapsEnabled = 0;
static jboolean showFramerateEnabled = 0;
static jobject tekuumRendererObj = 0;
static jobject tekuumViewObj = 0;

static int init = 0;

#if defined(USE_JNI)

static void* libdl;

//#define DEBUG
typedef enum fp_type
{
	FP_TYPE_NONE = 0,
	FP_TYPE_VFP  = 1,
	FP_TYPE_NEON = 2
} fp_type_t;

static fp_type_t fp_support()
{
	char buf[80];
	FILE* fp = fopen( "/proc/cpuinfo", "r" );
	if( !fp )
	{
		__android_log_print( ANDROID_LOG_DEBUG, "Tekuum_JNI", "Unable to open /proc/cpuinfo\n" );
		return FP_TYPE_NONE;
	}
	
	while( fgets( buf, 80, fp ) != NULL )
	{
		char* features = strstr( buf, "Features" );
		
		if( features )
		{
			fp_type_t fp_supported_type = FP_TYPE_NONE;
			char* feature;
			features += strlen( "Features" );
			feature = strtok( features, ": " );
			while( feature )
			{
				/* We prefer Neon if it is around, else VFP is also okay */
				if( !strcmp( feature, "neon" ) )
					return FP_TYPE_NEON;
				else if( !strcmp( feature, "vfp" ) )
					fp_supported_type = FP_TYPE_VFP;
					
				feature = strtok( NULL, ": " );
			}
			return fp_supported_type;
		}
	}
	return FP_TYPE_NONE;
}

const char* get_tekuum_library()
{
	/* We ship a library with Neon FPU support. This boosts performance a lot but it only works on a few CPUs. */
	
	fp_type_t fp_supported_type = fp_support();
	//if( fp_supported_type == FP_TYPE_NEON )
	//    return "libtekuum-neon.so";
	//else
	//if( fp_supported_type == FP_TYPE_VFP )
	return "libtekuum-v7a.so";
	
	//return "libtekuum.so";
}

void get_tekuum_library_path( char* libName, char* path )
{
	const char* libtekuum = get_tekuum_library();
	if( lib_dir )
	{
		sprintf( path, "%s/%s", lib_dir, libtekuum );
	}
	else
	{
		__android_log_print( ANDROID_LOG_ERROR, "Tekuum_JNI", "Library path not set, trying /data/data/com.robertbeckebans.tekuumyon.android/lib" );
		sprintf( path, "/data/data/com.robertbeckebans.tekuum/lib/%s", libtekuum );
	}
	
	sprintf( libName, "%s", libtekuum );
}

static void* dlsym_safe( void* handle, const char* symbol )
{
	const char* error;
	void* func = NULL;
	
	func = dlsym( handle, symbol );
	if( ( func == NULL ) && ( error = dlerror() ) != NULL )
	{
		__android_log_print( ANDROID_LOG_ERROR, "Tekuum_JNI", "Unable to load symbol %s: %s\n", symbol, error );
	}
	else
	{
//#ifdef DEBUG
		__android_log_print( ANDROID_LOG_DEBUG, "Tekuum_JNI", "Loaded symbol %s at %p\n", symbol, func );
//#endif
	}
	
	return func;
}

#else
jniExport_t* GetEngineJavaAPI( int apiVersion, jniImport_t* jimp );
#endif

static void JI_SetMenuState( int state )
{
	JNIEnv* env;
	( *jVM )->GetEnv( jVM, ( void** ) &env, JNI_VERSION_1_4 );
#ifdef DEBUG
	__android_log_print( ANDROID_LOG_DEBUG, "Tekuum_JNI", "JI_SetMenuState state=%d", state );
#endif
	
	( *env )->CallVoidMethod( env, tekuumRendererObj, android_setMenuState, state );
}

static void JI_RequestRender()
{
	JNIEnv* env;
	( *jVM )->GetEnv( jVM, ( void** ) &env, JNI_VERSION_1_4 );
#ifdef DEBUG
	__android_log_print( ANDROID_LOG_DEBUG, "Tekuum_JNI", "JI_RequestRender" );
#endif
	
	( *env )->CallVoidMethod( env, tekuumViewObj, android_requestRender );
}

static void JI_ShowProgressDialog( const char* msg )
{
	JNIEnv* env;
	jstring jmsg;
	
	( *jVM )->GetEnv( jVM, ( void** ) &env, JNI_VERSION_1_4 );
#ifdef DEBUG
	__android_log_print( ANDROID_LOG_DEBUG, "Tekuum_JNI", "JI_ShowProgressDialog" );
#endif
	
	jmsg = ( *env )->NewStringUTF( env, msg );
	
	( *env )->CallVoidMethod( env, object_Tekuum, method_Tekuum_showProgress, jmsg );
}

/*
static void JI_SetProgressDialogText(const char *msg)
{
    JNIEnv *env;
    jstring jmsg;

    (*jVM)->GetEnv(jVM, (void**) &env, JNI_VERSION_1_4);
#ifdef DEBUG
    __android_log_print(ANDROID_LOG_DEBUG, "Tekuum_JNI", "JI_ShowProgressDialog");
#endif

    jmsg = (*env)->NewStringUTF(env, msg);

    (*env)->CallVoidMethod(env, object_Tekuum, method_Tekuum_setProgressText, jmsg);
}
*/

static void JI_HideProgressDialog()
{
	JNIEnv* env;
	( *jVM )->GetEnv( jVM, ( void** ) &env, JNI_VERSION_1_4 );
#ifdef DEBUG
	__android_log_print( ANDROID_LOG_DEBUG, "Tekuum_JNI", "JI_ShowProgressDialog" );
#endif
	
	( *env )->CallVoidMethod( env, object_Tekuum, method_Tekuum_hideProgress );
}


static void load_libtekuum()
{
	jniImport_t			ji;
	jniExport_t*  	   	jniExport;
	
#if defined(USE_JNI)
	GetEngineJavaAPI_t	GetEngineAPI;
	
	char				libtekuum_name[80];
	char				libtekuum_path[80];
	
	get_tekuum_library_path( libtekuum_name, libtekuum_path );
	
#ifdef DEBUG
	__android_log_print( ANDROID_LOG_DEBUG, "Tekuum_JNI", "Attempting to load %s\n", libtekuum_path );
#endif
	
	libdl = dlopen( libtekuum_path, RTLD_NOW );
	if( !libdl )
	{
		__android_log_print( ANDROID_LOG_ERROR, "Tekuum_JNI", "Unable to load %s: %s\n", libtekuum_name, dlerror() );
		return;
	}
	
	GetEngineAPI = dlsym_safe( libdl, "GetEngineJavaAPI" );
	if( !GetEngineAPI )
	{
		return;
	}
#endif
	
	ji.SetMenuState = JI_SetMenuState;
	
	ji.ShowProgressDialog = JI_ShowProgressDialog;
	ji.HideProgressDialog = JI_HideProgressDialog;
	//ji.SetProgressDialogText = JI_SetProgressDialogText;
	
	ji.RequestRender = JI_RequestRender;
	
	__android_log_print( ANDROID_LOG_DEBUG, "Tekuum_JNI", "Calling GetEngineAPI...\n" );
	
#if defined(USE_JNI)
	jniExport = GetEngineAPI( ENGINE_JNI_API_VERSION, &ji );
	
	if( !libdl )
	{
		__android_log_print( ANDROID_LOG_ERROR, "Tekuum_JNI", "Unable to initialize %s\n", libtekuum_name );
		return;
	}
#else
	jniExport = GetEngineJavaAPI( ENGINE_JNI_API_VERSION, &ji );
#endif
	
	je = *jniExport;
	
	init = 1;
}



int JNI_OnLoad( JavaVM* vm, void* reserved )
{
	JNIEnv* env;
	jVM = vm;
	
#ifdef DEBUG
	__android_log_print( ANDROID_LOG_DEBUG, "Tekuum_JNI", "JNI_OnLoad called" );
#endif
	
	if( ( *vm )->GetEnv( vm, ( void** ) &env, JNI_VERSION_1_4 ) != JNI_OK )
	{
		__android_log_print( ANDROID_LOG_ERROR, "Tekuum_JNI", "Failed to get the environment using GetEnv()" );
		return -1;
	}
	
	if( !init )
	{
		load_libtekuum();
	}
	
	return JNI_VERSION_1_4;
}

JNIEXPORT void JNICALL Java_com_robertbeckebans_tekuum_TekuumJNI_enableBenchmark( JNIEnv* env, jclass c, jboolean enable )
{
	benchmarkEnabled = enable;
}

JNIEXPORT void JNICALL Java_com_robertbeckebans_tekuum_TekuumJNI_enableLightmaps( JNIEnv* env, jclass c, jboolean enable )
{
	lightmapsEnabled = enable;
}

JNIEXPORT void JNICALL Java_com_robertbeckebans_tekuum_TekuumJNI_showFramerate( JNIEnv* env, jclass c, jboolean enable )
{
	showFramerateEnabled = enable;
}

JNIEXPORT void JNICALL Java_com_robertbeckebans_tekuum_TekuumJNI_setGame( JNIEnv* env, jclass c, jobject obj )
{
	object_Tekuum = obj;
	
	( *jVM )->GetEnv( jVM, ( void** ) &env, JNI_VERSION_1_4 );
	object_Tekuum = ( jobject )( *env )->NewGlobalRef( env, obj );
	class_Tekuum = ( *env )->GetObjectClass( env, object_Tekuum );
	
	method_Tekuum_showProgress = ( *env )->GetMethodID( env, class_Tekuum, "showProgress", "(Ljava/lang/String;)V" );
	method_Tekuum_hideProgress = ( *env )->GetMethodID( env, class_Tekuum, "hideProgress", "()V" );
	//method_Tekuum_setProgressText = (*env)->GetMethodID(env, class_Tekuum, "setProgressText", "(Ljava/lang/String;)V");
}

JNIEXPORT void JNICALL Java_com_robertbeckebans_tekuum_TekuumJNI_setRenderer( JNIEnv* env, jclass c, jobject obj )
{
	tekuumViewObj = obj;
	jclass tekuumRendererClass;
	
	( *jVM )->GetEnv( jVM, ( void** ) &env, JNI_VERSION_1_4 );
	tekuumRendererObj = ( jobject )( *env )->NewGlobalRef( env, obj );
	tekuumRendererClass = ( *env )->GetObjectClass( env, tekuumRendererObj );
	
	android_setMenuState = ( *env )->GetMethodID( env, tekuumRendererClass, "setMenuState", "(I)V" );
}

JNIEXPORT void JNICALL Java_com_robertbeckebans_tekuum_TekuumJNI_setView( JNIEnv* env, jclass c, jobject obj )
{
	tekuumRendererObj = obj;
	jclass tekuumViewClass;
	
	( *jVM )->GetEnv( jVM, ( void** ) &env, JNI_VERSION_1_4 );
	tekuumViewObj = ( jobject )( *env )->NewGlobalRef( env, obj );
	tekuumViewClass = ( *env )->GetObjectClass( env, tekuumViewObj );
	
	android_requestRender = ( *env )->GetMethodID( env, tekuumViewClass, "requestRender", "()V" );
}


JNIEXPORT void JNICALL Java_com_robertbeckebans_tekuum_TekuumJNI_initGame( JNIEnv* env, jclass c, jint width, jint height )
{
	int i;
	static char* argv[5];
	int argc = 0;
	
	static char fs_basepath[256];
	
#if 0
	sprintf( fs_basepath, "+set fs_basepath %s", game_dir );
	argv[argc] = fs_basepath;
	argc++;
#endif
	
	/* TODO: integrate settings with tekuum, right now there is no synchronization */
	
#if 0
	if( !audioEnabled )
	{
		argv[argc] = strdup( "+set s_initsound 0" );
		argc++;
	}
	
	if( lightmapsEnabled )
		argv[argc] = strdup( "+set r_vertexlight 0" );
	else
		argv[argc] = strdup( "+set r_vertexlight 1" );
	argc++;
	
	if( showFramerateEnabled )
		argv[argc] = strdup( "+set com_showFPS 1" );
	else
		argv[argc] = strdup( "+set com_showFPS 0" );
	argc++;
	
	if( benchmarkEnabled )
	{
		argv[argc] = strdup( "+timedemo 1" );
		argc++;
	}
#endif
	
#ifdef DEBUG
	__android_log_print( ANDROID_LOG_DEBUG, "Tekuum_JNI", "initGame(%d, %d)", width, height );
#endif
	
	for( i = 0; i < argc; i++ )
	{
		__android_log_print( ANDROID_LOG_DEBUG, "Tekuum_JNI", "initGame(argc=%d, %s)", i, argv[i] );
	}
	
	/* In the future we might want to pass arguments using argc/argv e.g. to start a benchmark at startup, to load a mod or whatever */
	je.Main( argc, argv );
	
	je.SetResolution( width, height );
}

JNIEXPORT void JNICALL Java_com_robertbeckebans_tekuum_TekuumJNI_drawFrame( JNIEnv* env, jclass c )
{
#ifdef DEBUG
	__android_log_print( ANDROID_LOG_DEBUG, "Tekuum_JNI", "nextFrame()" );
#endif
	
	je.DrawFrame();
}

JNIEXPORT void JNICALL Java_com_robertbeckebans_tekuum_TekuumJNI_queueKeyEvent( JNIEnv* env, jclass c, jint key, jint state )
{
#ifdef DEBUG
	__android_log_print( ANDROID_LOG_DEBUG, "Tekuum_JNI", "queueKeyEvent(%d, %d)", key, state );
#endif
	
	je.QueueKeyEvent( key, state );
}

JNIEXPORT void JNICALL Java_com_robertbeckebans_tekuum_TekuumJNI_queueMotionEvent( JNIEnv* env, jclass c, jint action, jfloat x, jfloat y, jfloat pressure )
{
#ifdef DEBUG
	__android_log_print( ANDROID_LOG_DEBUG, "Tekuum_JNI", "queueMotionEvent(%d, %f, %f, %f)", action, x, y, pressure );
#endif
	
	je.QueueMotionEvent( action, x, y, pressure );
}

JNIEXPORT void JNICALL Java_com_robertbeckebans_tekuum_TekuumJNI_queueTrackballEvent( JNIEnv* env, jclass c, jint action, jfloat x, jfloat y )
{
#ifdef DEBUG
	__android_log_print( ANDROID_LOG_DEBUG, "Tekuum_JNI", "queueTrackballEvent(%d, %f, %f)", action, x, y );
#endif
	
	je.QueueTrackballEvent( action, x, y );
}

JNIEXPORT void JNICALL Java_com_robertbeckebans_tekuum_TekuumJNI_queueJoystickEvent( JNIEnv* env, jclass c, jint axis, jfloat x )
{
#ifdef DEBUG
	__android_log_print( ANDROID_LOG_DEBUG, "Tekuum_JNI", "queueJoystickEvent(%d, %f)", axis, x );
#endif
	
	je.QueueJoystickEvent( axis, x );
}

JNIEXPORT void JNICALL Java_com_robertbeckebans_tekuum_TekuumJNI_queueConsoleEvent( JNIEnv* env, jclass c, jstring js )
{
	char* s;
	
	if( js == NULL )
		return;
		
	s = ( char* )( *env )->GetStringUTFChars( env, js, 0 );
	
#if 1 //def DEBUG
	__android_log_print( ANDROID_LOG_DEBUG, "Tekuum_JNI", "queueConsoleEvent(%s)", s );
#endif
	
	je.QueueConsoleEvent( s );
	
	( *env )->ReleaseStringUTFChars( env, js, s );
}

JNIEXPORT void JNICALL Java_com_robertbeckebans_tekuum_TekuumJNI_setGameDirectory( JNIEnv* env, jclass c, jstring jpath )
{
	jboolean iscopy;
	const jbyte* path = ( *env )->GetStringUTFChars( env, jpath, &iscopy );
	game_dir = strdup( path );
	//setenv("GAME_PATH", game_dir, 1);
	( *env )->ReleaseStringUTFChars( env, jpath, path );
	
#ifdef DEBUG
	__android_log_print( ANDROID_LOG_DEBUG, "Tekuum_JNI", "game path=%s\n", game_dir );
#endif
}

JNIEXPORT void JNICALL Java_com_robertbeckebans_tekuum_TekuumJNI_setLibraryDirectory( JNIEnv* env, jclass c, jstring jpath )
{
	jboolean iscopy;
	const jbyte* path = ( *env )->GetStringUTFChars( env, jpath, &iscopy );
	lib_dir = strdup( path );
	( *env )->ReleaseStringUTFChars( env, jpath, path );
	
#ifdef DEBUG
	__android_log_print( ANDROID_LOG_DEBUG, "Tekuum_JNI", "library path=%s\n", lib_dir );
#endif
}

JNIEXPORT jboolean JNICALL Java_com_robertbeckebans_tekuum_TekuumJNI_isConsoleActive( JNIEnv* env, jclass c )
{
	return je.IsConsoleActive();
}

JNIEXPORT jboolean JNICALL Java_com_robertbeckebans_tekuum_TekuumJNI_isMenuActive( JNIEnv* env, jclass c )
{
	return je.IsMenuActive();
}


