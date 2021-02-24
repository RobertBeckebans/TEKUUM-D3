#ifndef TEKUUMJNI_PUBLIC_H_
#define TEKUUMJNI_PUBLIC_H_

const int ENGINE_JNI_API_VERSION = 7;

#if defined(__cplusplus)
extern "C" {
#endif

// functions that are imported by the libtekuum.so engine to the Java tekuumjni bridge
typedef struct
{
	// ui
	void	( *SetMenuState )( int shown );

	void	( *ShowProgressDialog )( const char* text );
	void	( *HideProgressDialog )( void );

	// renderer
	void ( *RequestRender )( void );
} jniImport_t;

// functions that are exported by the libtekuum.so engine to the Java tekuumjni bridge
typedef struct
{
	int	( *Main )( int argc, char** argv );
	void	( *DrawFrame )( void );
	void	( *QueueKeyEvent )( int key, int state );
	void	( *QueueMotionEvent )( int action, float x, float y, float pressure );
	void	( *QueueTrackballEvent )( int action, float x, float y );
	void	( *QueueJoystickEvent )( int axis, float x );
	void	( *QueueConsoleEvent )( const char* s );
	void	( *SetResolution )( int width, int height );
	int	( *IsConsoleActive )( void );
	int	( *IsMenuActive )( void );
} jniExport_t;

typedef jniExport_t* ( *GetEngineJavaAPI_t )( int apiVersion, jniImport_t* jimp );

#if defined(__cplusplus)
}
#endif

#endif /* TEKUUMJNI_PUBLIC_H_ */
