/*
 * Copyright (C) 2010 Roderick Colenbrander
 * Copyright (C) 2012-2013 Robert Beckebans
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

package com.robertbeckebans.tekuum;

import java.io.File;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.DialogInterface;
import android.content.DialogInterface.OnDismissListener;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Handler.Callback;
import android.os.Message;
import android.text.Editable;
import android.util.Log;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.ArrayAdapter;
import android.widget.AutoCompleteTextView;
import android.widget.EditText;

public class Tekuum extends Activity implements Callback
{
	private TekuumView mGLSurfaceView;
	
	private ProgressDialog progressDialog;
	private Handler handlerUI;
	
	
	/**
	 * set by checkGameFiles()
	 */
	private String fs_basepath;
	private String fs_game;
	
	private static Tekuum instance = null;
	
	public static Tekuum getInstance()
	{
		return instance;
	}
	
	public static void showError( String s )
	{
		AlertDialog.Builder builder = new AlertDialog.Builder( getInstance() );
		builder.setMessage( s );
		builder.setPositiveButton( "OK", new DialogInterface.OnClickListener()
		{
			public void onClick( DialogInterface dialog, int id )
			{
				getInstance().finish();
			}
		} );
		AlertDialog dialog = builder.create();
		dialog.show();
	}
	
	private boolean checkGameFiles()
	{
		File sdcard = Environment.getExternalStorageDirectory();
		
		String try_dir = "/sdcard/tekuum";
		//String try_dir = sdcard.getAbsolutePath() + File.separator + "external_sd" + File.separator + "tekuum";
		File game_dir = new File( try_dir );
		if( game_dir.exists() == false )
		{
			showError( "Unable to locate" + try_dir );
			return false;
		}
		fs_basepath = game_dir.getAbsolutePath();
		TekuumJNI.setGameDirectory( fs_basepath );
		
		try_dir = fs_basepath + File.separator + "base";
		File base_dir = new File( try_dir );
		if( base_dir.exists() == false )
		{
			showError( "Unable to locate " + try_dir );
			return false;
		}
		fs_game = base_dir.getAbsolutePath();
		
		/* Check if pak000.pk4 - pak008.pk3 are around */
		for( int i = 0; i < 9; i++ )
		{
			String pak_filename = "pak" + String.format( "%03d", i ) + ".pk4";
			File pak_file = new File( fs_game + File.separator + pak_filename );
			if( pak_file.exists() == false )
			{
				showError( "Unable to locate " + fs_game + File.separator + pak_filename );
				return false;
			}
		}
		return true;
	}
	
	/** Called when the activity is first created. */
	@Override
	public void onCreate( Bundle savedInstanceState )
	{
		super.onCreate( savedInstanceState );
		
		instance = this;
		
		handlerUI = new Handler( this );
		
		TekuumJNI.setGame( this );
		
		// enable fullscreen
		getWindow().setFlags( WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN );
		requestWindowFeature( Window.FEATURE_NO_TITLE );
		
		// keep screen on
		getWindow().setFlags( WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON, WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON );
		
		setContentView( R.layout.main );
		
		startEngine();
	}
	
	private void startEngine()
	{
		/* Don't initialize the game engine when the game files aren't around */
		if( checkGameFiles() )
		{
			showProgress( "Please wait while loading..." );
			
			mGLSurfaceView = new TekuumView( this );
			TekuumJNI.setView( mGLSurfaceView );
			
			setContentView( mGLSurfaceView );
			mGLSurfaceView.requestFocus();
			mGLSurfaceView.setId( 1 );
			
			Bundle extras = getIntent().getExtras();
			if( extras != null )
			{
				/* We use a separate call for disabling audio because a user
				 * can reactivate audio from the q3 console by issuing 's_initsound 1' + 'snd_restart'
				 */
				TekuumJNI.enableAudio( extras.getBoolean( "sound" ) );
				
				/* Enable lightmaps for rendering (default=disabled).
				 * Enabling causes a performance hit of typically 25% (on Milestone + Nexus One).
				 */
				TekuumJNI.enableLightmaps( extras.getBoolean( "lightmaps" ) );
				
				/* Run a timedemo */
				TekuumJNI.enableBenchmark( extras.getBoolean( "benchmark" ) );
				
				/* Show the framerate */
				TekuumJNI.showFramerate( extras.getBoolean( "showfps" ) );
			}
		}
		else
		{
			setContentView( R.layout.main );
		}
	}
	
	@Override
	protected void onDestroy()
	{
		super.onDestroy();
	}
	
	@Override
	protected void onPause()
	{
		Log.d( "Tekuum_Java", "onPause" );
		super.onPause();
		
		if( mGLSurfaceView != null )
		{
			mGLSurfaceView.onPause();
		}
		
		// TODO TekuumJNI.onPause()
		// pause sound thread
	}
	
	@Override
	protected void onResume()
	{
		/* Resume doesn't always seem to work well. On my Milestone it works
		 * but not on the G1. The native code seems to be running but perhaps
		 * we need to issue a 'vid_restart'.
		 */
		Log.d( "Tekuum_Java", "onResume" );
		super.onResume();
		if( mGLSurfaceView != null )
		{
			mGLSurfaceView.onResume();
		}
		
		// TODO TekuumJNI.onResume()
		// unpause sound thread
	}
	
	@Override
	public boolean onPrepareOptionsMenu( Menu menu )
	{
		menu.clear();
		
		menu.add( "Exit" );
		menu.add( "Console" );
		menu.add( "Nodamage" );
		menu.add( "Noclip" );
		menu.add( "Testmap" );
		menu.add( "MC1" );
		menu.add( "MC2" );
		menu.add( "Spawn_monster_zombie_fat" );
		menu.add( "Kill_Monsters" );
		//menu.add( "Start_AndProf" );
		//menu.add( "Stop_AndProf" );
		menu.add( "Toggle_Light" );
		menu.add( "Toggle_Shadows" );
		menu.add( "Toggle_Sound" );
		
		return true;
	}
	
	static final int DIALOG_EXIT_ID = 0;
	static final int DIALOG_CHEAT_KILLMONSTERS = 1;
	static final int DIALOG_CHEAT_NOCLIP = 2;
	static final int DIALOG_PROGRESS_ID = 3;
	static final int DIALOG_CONSOLE_COMMAND = 4;
	
	static final int MSG_UPDATE_PROGRESS = 1;
	
	@Override
	public boolean onOptionsItemSelected( MenuItem item )
	{
		if( "Console".equals( item.toString() ) )
		{
			if( mGLSurfaceView != null )
			{
				mGLSurfaceView.onKeyDown( KeyEvent.KEYCODE_GRAVE, new KeyEvent( KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_GRAVE ) );
				return true;
			}
		}
		else if( "Nodamage".equals( item.toString() ) )
		{
			TekuumJNI.queueConsoleEvent( "nodamage" );
			return true;
		}
		else if( "Noclip".equals( item.toString() ) )
		{
			TekuumJNI.queueConsoleEvent( "noclip" );
			return true;
		}
		else if( "Testmap".equals( item.toString() ) )
		{
			TekuumJNI.queueConsoleEvent( "devmap testmaps/test_box" );
			return true;
		}
		else if( "MC1".equals( item.toString() ) )
		{
			TekuumJNI.queueConsoleEvent( "devmap game/mars_city1" );
			return true;
		}
		else if( "MC2".equals( item.toString() ) )
		{
			TekuumJNI.queueConsoleEvent( "devmap game/mars_city2" );
			return true;
		}
		else if( "Spawn_monster_zombie_fat".equals( item.toString() ) )
		{
			TekuumJNI.queueConsoleEvent( "spawn monster_zombie_fat" );
			return true;
		}
		else if( "Kill_Monsters".equals( item.toString() ) )
		{
			TekuumJNI.queueConsoleEvent( "killMonsters" );
			return true;
		}
		else if( "Start_AndProf".equals( item.toString() ) )
		{
			TekuumJNI.queueConsoleEvent( "startAndroidProfiling" );
			return true;
		}
		else if( "Stop_AndProf".equals( item.toString() ) )
		{
			TekuumJNI.queueConsoleEvent( "stopAndroidProfiling" );
			return true;
		}
		else if( "Toggle_Light".equals( item.toString() ) )
		{
			TekuumJNI.queueConsoleEvent( "toggle r_usePrecomputedLight" );
			return true;
		}
		else if( "Toggle_Shadows".equals( item.toString() ) )
		{
			TekuumJNI.queueConsoleEvent( "toggle r_skipShadows; toggle r_skipStaticShadows; toggle r_skipDynamicShadows; toggle r_skipPrelightShadows" );
			return true;
		}
		else if( "Toggle_Sound".equals( item.toString() ) )
		{
			TekuumJNI.queueConsoleEvent( "toggle s_noSound; s_restart" );
			return true;
		}
		else if( "Exit".equals( item.toString() ) )
		{
			showDialog( DIALOG_EXIT_ID );
			return true;
		}
		
		return false;
	}
	
	@Override
	protected Dialog onCreateDialog( int id )
	{
		switch( id )
		{
			case DIALOG_EXIT_ID:
			{
				AlertDialog.Builder builder = new AlertDialog.Builder( this );
				builder.setMessage( "Are you sure you want to exit?" )
				.setCancelable( false )
				.setPositiveButton( "Yes", new DialogInterface.OnClickListener()
				{
					public void onClick( DialogInterface dialog, int id )
					{
						//finish();
						int pid = android.os.Process.myPid();
						android.os.Process.killProcess( pid );
					}
				} )
				.setNegativeButton( "No", new DialogInterface.OnClickListener()
				{
					public void onClick( DialogInterface dialog, int id )
					{
						dialog.cancel();
					}
				} );
				Dialog dialog = builder.create();
				return dialog;
			}
			
			//case DIALOG_ABOUT_ID:
			// TODO
			//return null;
			
			case DIALOG_CONSOLE_COMMAND:
			{
				// This example shows how to add a custom layout to an AlertDialog
				LayoutInflater factory = LayoutInflater.from( this );
				final View textEntryView = factory.inflate( R.layout.alert_dialog_console_command, null );
				final AutoCompleteTextView input = ( AutoCompleteTextView ) textEntryView .findViewById( R.id.command_edit );
				
				ArrayAdapter<String> adapter = new ArrayAdapter<String>( this, android.R.layout.simple_dropdown_item_1line, CONSOLE_COMMANDS );
				input.setAdapter( adapter );
				
				return new AlertDialog.Builder( this )
					   //.setIcon(R.drawable.alert_dialog_icon)
					   .setTitle( "Type console command" )
					   .setView( textEntryView )
					   .setPositiveButton( "OK", new DialogInterface.OnClickListener()
				{
					public void onClick( DialogInterface dialog, int whichButton )
					{
						//Log.d("Tekuum_Java", "ConsoleDialog.onClick");
						
						Editable editable = input.getText();
						String s = editable.toString();
						TekuumJNI.queueConsoleEvent( s );
					}
				} )
				.setNegativeButton( "Cancel", new DialogInterface.OnClickListener()
				{
					public void onClick( DialogInterface dialog, int whichButton )
					{
						dialog.cancel();
					}
				} )
				.create();
			}
		}
		
		return null;
	}
	
	public void showProgress( String txt )
	{
		if( progressDialog != null )
		{
			Message msg = handlerUI.obtainMessage( MSG_UPDATE_PROGRESS, txt );
			handlerUI.sendMessage( msg );
		}
		else
		{
			progressDialog = new ProgressDialog( this );
			progressDialog.setTitle( "Please wait while starting Tekuum" );
			progressDialog.setMessage( txt );
			progressDialog.setIndeterminate( false );
			progressDialog.setCancelable( true );
			progressDialog.setOnDismissListener( new OnDismissListener()
			{
			
				@Override
				public void onDismiss( DialogInterface dialog )
				{
					if( mGLSurfaceView != null )
					{
						mGLSurfaceView.requestFocus();
					}
				}
			} );
			
			progressDialog.show();
		}
	}
	
	public void hideProgress()
	{
		if( progressDialog != null )
		{
			progressDialog.dismiss();
			progressDialog = null;
		}
	}
	
	@Override
	public boolean handleMessage( Message msg )
	{
	
		switch( msg.what )
		{
			case MSG_UPDATE_PROGRESS:
				if( progressDialog != null )
				{
					progressDialog.setMessage( ( String ) msg.obj );
				}
				break;
		}
		
		return false;
	}
	
	private static final String[] CONSOLE_COMMANDS = new String[]
	{
		// cvars
		"aas_goalArea",
		"aas_pullPlayer",
		"aas_randomPullPlayer",
		"aas_showAreas",
		"aas_showFlyPath",
		"aas_showHideArea",
		"aas_showPath",
		"aas_showPushIntoArea",
		"aas_showWallEdges",
		"aas_test",
		"af_contactFrictionScale",
		"af_forceFriction",
		"af_highlightBody",
		"af_highlightConstraint",
		"af_jointFrictionScale",
		"af_maxAngularVelocity",
		"af_maxLinearVelocity",
		"af_showActive",
		"af_showBodies",
		"af_showBodyNames",
		"af_showConstrainedBodies",
		"af_showConstraintNames",
		"af_showConstraints",
		"af_showInertia",
		"af_showLimits",
		"af_showMass",
		"af_showPrimaryOnly",
		"af_showTimings",
		"af_showTotalMass",
		"af_showTrees",
		"af_showVelocity",
		"af_skipFriction",
		"af_skipLimits",
		"af_skipSelfCollision",
		"af_testSolid",
		"af_timeScale",
		"af_useImpulseFriction",
		"af_useJointImpulseFriction",
		"af_useLinearTime",
		"af_useSymmetry",
		"ai_blockedFailSafe",
		"ai_debugMove",
		"ai_debugScript",
		"ai_debugTrajectory",
		"ai_showCombatNodes",
		"ai_showObstacleAvoidance",
		"ai_showPaths",
		"ai_testPredictPath",
		"bearTurretAngle",
		"bearTurretForce",
		"cm_backFaceCull",
		"cm_debugCollision",
		"cm_drawColor",
		"cm_drawFilled",
		"cm_drawInternal",
		"cm_drawMask",
		"cm_drawNormals",
		"cm_testAngle",
		"cm_testBox",
		"cm_testBoxRotation",
		"cm_testCollision",
		"cm_testLength",
		"cm_testModel",
		//"cm_testOrigin",
		"cm_testRadius",
		"cm_testRandomMany",
		"cm_testReset",
		"cm_testRotation",
		"cm_testTimes",
		"cm_testWalk",
		"com_allowConsole",
		"com_asyncInput",
		"com_asyncSound",
		"com_aviDemoHeight",
		"com_aviDemoSamples",
		"com_aviDemoTics",
		"com_aviDemoWidth",
		"com_compressDemos",
		"com_fixedTic",
		"com_forceGenericSIMD",
		"com_guid",
		"com_journal",
		"com_logDemos",
		"com_machineSpec",
		"com_makingBuild",
		"com_memoryMarker",
		"com_minTics",
		"com_preciseTic",
		"com_preloadDemos",
		"com_product_lang_ext",
		"com_purgeAll",
		"com_showAngles",
		"com_showAsyncStats",
		"com_showDemo",
		"com_showFPS",
		"com_showMemoryUsage",
		"com_showSoundDecoders",
		"com_showTics",
		"com_skipGameDraw",
		"com_skipRenderer",
		"com_speeds",
		"com_timestampPrints",
		"com_updateLoadSize",
		"com_videoRam",
		"com_wipeSeconds",
		"con_noPrint",
		"con_notifyTime",
		"con_speed",
		"decl_show",
		"developer",
		"EntityPlacement",
		"fs_basepath",
		"fs_caseSensitiveOS",
		"fs_cdpath",
		"fs_copyfiles",
		"fs_debug",
		"fs_devpath",
		"fs_game",
		"fs_game_base",
		"fs_restrict",
		"fs_savepath",
		"fs_searchAddons",
		"g_armorProtection",
		"g_armorProtectionMP",
		"g_balanceTDM",
		"g_blobSize",
		"g_blobTime",
		"g_bloodEffects",
		"g_cinematic",
		"g_cinematicMaxSkipTime",
		"g_countDown",
		"g_damageScale",
		"g_debugAnim",
		"g_debugBounds",
		"g_debugCinematic",
		"g_debugDamage",
		"g_debugMove",
		"g_debugMover",
		"g_debugScript",
		"g_debugTriggers",
		"g_debugWeapon",
		"g_decals",
		"g_disasm",
		"g_doubleVision",
		"g_dragDamping",
		"g_dragEntity",
		"g_dragShowSelection",
		"g_dropItemRotation",
		"g_dvAmplitude",
		"g_dvFrequency",
		"g_dvTime",
		"g_editEntityMode",
		"g_exportMask",
		"g_flushSave",
		"g_fov",
		"g_frametime",
		"g_gameReviewPause",
		"g_gravity",
		"g_gunX",
		"g_gunY",
		"g_gunZ",
		"g_healthTakeAmt",
		"g_healthTakeLimit",
		"g_healthTakeTime",
		"g_kickAmplitude",
		"g_kickTime",
		"g_knockback",
		"g_mapCycle",
		"g_maxShowDistance",
		"g_monsters",
		"g_mpWeaponAngleScale",
		"g_muzzleFlash",
		"g_nightmare",
		"g_password",
		"g_projectileLights",
		"g_showActiveEntities",
		"g_showBrass",
		"g_showcamerainfo",
		"g_showCollisionModels",
		"g_showCollisionTraces",
		"g_showCollisionWorld",
		"g_showEnemies",
		"g_showEntityInfo",
		"g_showHud",
		"g_showPlayerShadow",
		"g_showProjectilePct",
		"g_showPVS",
		"g_showTargets",
		"g_showTestModelFrame",
		"g_showTriggers",
		"g_showviewpos",
		"g_skill",
		"g_skipFX",
		"g_skipParticles",
		"g_skipViewEffects",
		"g_spectatorChat",
		"g_stopTime",
		"g_TDMArrows",
		"g_testDeath",
		"g_testHealthVision",
		"g_testModelAnimate",
		"g_testModelBlend",
		"g_testModelRotate",
		"g_testParticle",
		"g_testParticleName",
		"g_testPostProcess",
		"g_timeEntities",
		"g_useDynamicProtection",
		"g_vehicleForce",
		"g_vehicleSuspensionDamping",
		"g_vehicleSuspensionDown",
		"g_vehicleSuspensionKCompress",
		"g_vehicleSuspensionUp",
		"g_vehicleTireFriction",
		"g_vehicleVelocity",
		"g_version",
		"g_viewNodalX",
		"g_viewNodalZ",
		"g_voteFlags",
		"gamedate",
		"gamename",
		"gui_configServerRate",
		"gui_debug",
		"gui_edit",
		"gui_filter_game",
		"gui_filter_gameType",
		"gui_filter_idle",
		"gui_filter_password",
		"gui_filter_players",
		"gui_mediumFontLimit",
		"gui_smallFontLimit",
		"ik_debug",
		"ik_enable",
		"image_highQualityCompression",
		"in_alwaysRun",
		"in_anglespeedkey",
		"in_freeLook",
		"in_mouse",
		"in_pitchspeed",
		"in_toggleCrouch",
		"in_toggleRun",
		"in_toggleZoom",
		"in_yawspeed",
		"lcp_showFailures",
		"logFile",
		"logFileName",
		"m_pitch",
		"m_showMouseRate",
		"m_smooth",
		"m_strafeScale",
		"m_strafeSmooth",
		"m_yaw",
		"mod_validSkins",
		"net_allowCheats",
		"net_channelShowDrop",
		"net_channelShowPackets",
		"net_clientDownload",
		"net_clientLagOMeter",
		"net_clientMaxPrediction",
		"net_clientMaxRate",
		"net_clientPredictGUI",
		"net_clientPrediction",
		"net_clientRemoteConsoleAddress",
		"net_clientRemoteConsolePassword",
		"net_clientSelfSmoothing",
		"net_clientServerTimeout",
		"net_clientShowSnapshot",
		"net_clientShowSnapshotRadius",
		"net_clientSmoothing",
		"net_clientUsercmdBackup",
		"net_forceDrop",
		"net_forceLatency",
		"net_ip",
		"net_LANServer",
		"net_master0",
		"net_master1",
		"net_master2",
		"net_master3",
		"net_master4",
		"net_port",
		"net_serverAllowServerMod",
		"net_serverClientTimeout",
		"net_serverDedicated",
		"net_serverDlBaseURL",
		"net_serverDlTable",
		"net_serverDownload",
		"net_serverDrawClient",
		"net_serverMaxClientRate",
		"net_serverMaxUsercmdRelay",
		"net_serverReloadEngine",
		"net_serverRemoteConsolePassword",
		"net_serverSnapshotDelay",
		"net_serverZombieTimeout",
		"net_socksEnabled",
		"net_socksPassword",
		"net_socksPort",
		"net_socksServer",
		"net_socksUsername",
		"net_verbose",
		"password",
		"pm_air",
		"pm_bboxwidth",
		"pm_bobpitch",
		"pm_bobroll",
		"pm_bobup",
		"pm_crouchbob",
		"pm_crouchheight",
		"pm_crouchrate",
		"pm_crouchspeed",
		"pm_crouchviewheight",
		"pm_deadheight",
		"pm_deadviewheight",
		"pm_jumpheight",
		"pm_maxviewpitch",
		"pm_minviewpitch",
		"pm_modelView",
		"pm_noclipspeed",
		"pm_normalheight",
		"pm_normalviewheight",
		"pm_runbob",
		"pm_runpitch",
		"pm_runroll",
		"pm_runspeed",
		"pm_spectatebbox",
		"pm_spectatespeed",
		"pm_stamina",
		"pm_staminarate",
		"pm_staminathreshold",
		"pm_stepsize",
		"pm_thirdPerson",
		"pm_thirdPersonAngle",
		"pm_thirdPersonClip",
		"pm_thirdPersonDeath",
		"pm_thirdPersonHeight",
		"pm_thirdPersonRange",
		"pm_usecylinder",
		"pm_walkbob",
		"pm_walkspeed",
		"r_alwaysExportGLSL",
		"r_binaryLoadRenderModels",
		"r_brightness",
		"r_centerX",
		"r_centerY",
		"r_checkBounds",
		"r_clear",
		"r_cullDynamicLightTriangles",
		"r_cullDynamicShadowTriangles",
		"r_customHeight",
		"r_customWidth",
		"r_debugArrowStep",
		"r_debugContext",
		"r_debugLineDepthTest",
		"r_debugLineWidth",
		"r_debugPolygonFilled",
		"r_debugRenderToTexture",
		"r_displayGLSLCompilerMessages",
		"r_displayRefresh",
		"r_drawEyeColor",
		"r_drawFlickerBox",
		"r_flareSize",
		"r_forceScreenWidthCentimeters",
		"r_forceShadowCaps",
		"r_forceSoundOpAmplitude",
		"r_forceZPassStencilShadows",
		"r_fullscreen",
		"r_gamma",
		//"r_glDriver                       ""
		"r_ignoreGLErrors",
		"r_jitter",
		"r_jointNameOffset",
		"r_jointNameScale",
		"r_lightAllBackFaces",
		"r_lightScale",
		"r_lodBias",
		"r_logFile",
		"r_logLevel",
		"r_materialOverride",
		"r_maxAnisotropicFiltering",
		"r_mergeModelSurfaces",
		"r_motionBlur",
		"r_multiSamples",
		"r_offsetfactor",
		"r_offsetunits",
		"r_orderIndexes",
		"r_pix",
		"r_requestStereoPixelFormat",
		"r_screenFraction",
		"r_shadowPolygonFactor",
		"r_shadowPolygonOffset",
		"r_showAddModel",
		"r_showBuffers",
		"r_showCenterOfProjection",
		"r_showCull",
		"r_showDemo",
		"r_showDepth",
		"r_showDominantTri",
		"r_showDynamic",
		"r_showEdges",
		"r_showIntensity",
		"r_showLightCount",
		"r_showLightGrid",
		"r_showLights",
		"r_showLightScissors",
		"r_showLines",
		"r_showMemory",
		"r_showNormals",
		"r_showOverDraw",
		"r_showPortals",
		"r_showPrimitives",
		"r_showShadows",
		"r_showSilhouette",
		"r_showSkel",
		"r_showSurfaceInfo",
		"r_showSurfaces",
		"r_showSwapBuffers",
		"r_showTangentSpace",
		"r_showTexturePolarity",
		"r_showTextureVectors",
		"r_showTrace",
		"r_showTris",
		"r_showUnsmoothedTangents",
		"r_showUpdates",
		"r_showVertexCache",
		"r_showVertexCacheTimings",
		"r_showVertexColor",
		"r_showViewEntitys",
		"r_singleArea",
		"r_singleEntity",
		"r_singleLight",
		"r_singleSurface",
		"r_singleTriangle",
		"r_skipAmbient",
		"r_skipBackEnd",
		"r_skipBlendLights",
		"r_skipBump ",
		"r_skipCopyTexture",
		"r_skipDecals",
		"r_skipDeforms",
		"r_skipDiffuse",
		"r_skipDynamicInteractions",
		"r_skipDynamicShadows",
		"r_skipDynamicTextures",
		"r_skipFogLights",
		"r_skipFrontEnd",
		"r_skipGuiShaders",
		"r_skipIntelWorkarounds",
		"r_skipInteractionFastPath",
		"r_skipInteractions",
		"r_skipNewAmbient",
		"r_skipOverlays",
		"r_skipParticles",
		"r_skipPostProcess",
		"r_skipPrelightShadows",
		"r_skipRender",
		"r_skipRenderContext",
		"r_skipROQ",
		"r_skipShaderPasses",
		"r_skipShadows",
		"r_skipSpecular",
		"r_skipStaticInteractions",
		"r_skipStaticShadows",
		"r_skipStripDeadCode",
		"r_skipSubviews",
		"r_skipSuppress",
		"r_skipTranslucent",
		"r_skipUpdates",
		"r_slopNormal",
		"r_slopTexCoord",
		"r_slopVertex",
		"r_subviewOnly",
		"r_swapInterval",
		"r_syncEveryFrame",
		"r_testGamma",
		"r_testGammaBias",
		"r_useAreasConnectedForShadowCulling",
		"r_useCachedDynamicModels",
		"r_useConstantMaterials",
		"r_useEntityCallbacks",
		"r_useEntityPortalCulling",
		"r_useGPUSkinning",
		"r_useLightAreaCulling",
		"r_useLightDepthBounds",
		"r_useLightPortalCulling",
		"r_useLightPortalFlow",
		"r_useLightScissors",
		"r_useLightStencilSelect",
		"r_useNodeCommonChildren",
		//"r_useOpenGL32                    "1"
		//"r_useOpenGLES                    "0"
		"r_useParallelAddLights",
		"r_useParallelAddModels",
		"r_useParallelAddShadows",
		"r_usePortals",
		"r_usePrecomputedLight",
		"r_useScissor",
		"r_useSeamlessCubeMap",
		"r_useShadowDepthBounds",
		"r_useShadowPreciseInsideTest",
		"r_useShadowSurfaceScissor",
		"r_useSilRemap",
		"r_useSRGB",
		"r_useStateCaching",
		"r_useStencilShadowPreload",
		"r_useTrilinearFiltering",
		"r_useUniformArrays",
		"r_useViewBypass",
		"r_vidMode",
		"r_windowHeight",
		"r_windowWidth",
		"r_windowX",
		"r_windowY",
		"r_znear",
		"rb_showActive",
		"rb_showBodies",
		"rb_showInertia",
		"rb_showMass",
		"rb_showTimings",
		"rb_showVelocity",
		"rs_display",
		"rs_dropFraction",
		"rs_dropMilliseconds",
		"rs_enable",
		"rs_forceFractionX",
		"rs_forceFractionY",
		"rs_raiseFraction",
		"rs_raiseFrames",
		"rs_raiseMilliseconds",
		"rs_showResolutionChanges",
		"s_clipVolumes",
		"s_constantAmplitude",
		"s_decompressionLimit",
		"s_doorDistanceAdd",
		"s_dotbias2",
		"s_dotbias6",
		"s_drawSounds",
		"s_enviroSuitCutoffFreq",
		"s_enviroSuitCutoffQ",
		"s_enviroSuitVolumeScale",
		"s_force22kHz",
		"s_globalFraction",
		"s_libOpenAL",
		"s_maxSoundsPerShader",
		"s_meterTopTime",
		"s_minVolume2",
		"s_minVolume6",
		"s_muteEAXReverb",
		"s_noSound",
		"s_numberOfSpeakers",
		"s_playDefaultSound",
		"s_quadraticFalloff",
		"s_realTimeDecoding",
		"s_reverbFeedback",
		"s_reverbTime",
		"s_reverse",
		"s_showLevelMeter",
		"s_showStartSound",
		"s_singleEmitter",
		"s_skipHelltimeFX",
		"s_slowAttenuate",
		"s_spatializationDecay",
		"s_subFraction",
		"s_useEAXReverb",
		"s_useOcclusion",
		"s_useOpenAL",
		"s_volume_dB",
		"sensitivity",
		"si_fragLimit",
		"si_gameType",
		"si_idleServer",
		"si_map",
		"si_maxPlayers",
		"si_name",
		"si_pure",
		"si_serverURL",
		"si_spectators",
		"si_teamDamage",
		"si_timeLimit",
		"si_usePass",
		"si_version",
		"si_warmup",
		"sys_arch",
		"sys_cpustring",
		"sys_lang",
		"sys_videoRam",
		"timescale",
		"ui_autoReload",
		"ui_autoSwitch",
		"ui_chat",
		"ui_name",
		"ui_ready",
		"ui_showGun",
		"ui_skin",
		"ui_spectate",
		"ui_team",
		
		// commands
		"aasStats",
		"addarrow",
		"addChatLine",
		"addline",
		"aviCmdDemo",
		"aviDemo",
		"aviGame",
		"benchmark",
		"bind",
		"bindRagdol",
		"bindunbindtwo",
		"blinkline",
		"centerview",
		"checkNewVersion",
		"clear",
		"clearLights",
		"clientDropWeapon",
		"clientMessageMode",
		"clientVoiceChat",
		"clientVoiceChatTeam",
		"closeViewNotes",
		"collisionModelInfo",
		"combineCubeImages",
		"compressDemo",
		"conDump",
		"connect",
		//"crash",
		"cvar_restart",
		"damage",
		"deleteSelected",
		"demoShot",
		"devmap",
		"dir",
		"dirtree",
		"disasmScript",
		"disconnect",
		"dmap",
		"echo",
		"envshot",
		"error",
		"exec",
		"execMachineSpec",
		"exit",
		"exitCmdDemo",
		"exportmodels",
		"finishBuild",
		"freeze",
		"game_memory",
		"gameError",
		"gameKick",
		"getviewpos",
		"gfxInfo",
		"give",
		"god",
		"heartbeat",
		"hitch",
		"in_restart",
		"keepTestModel",
		"kick",
		"kill",
		"killMonsters",
		"killMoveables",
		"killRagdolls",
		"LANScan",
		"listActiveEntities",
		"listAF",
		"listAnims",
		"listAudios",
		"listBinds",
		"listClasses",
		"listCmds",
		"listCollisionModels",
		"listCvars",
		"listDecls",
		"listDictKeys",
		"listDictValues",
		"listEmails",
		"listEntities",
		"listEntityDefs",
		"listFX",
		"listGameCmds",
		"listGuis",
		"listHuffmanFrequencies",
		"listImages",
		"listLines",
		"listMaterials",
		"listModelDefs",
		"listModels",
		"listModes",
		"listMonsters",
		"listParticles",
		"listPDAs",
		"listRenderEntityDefs",
		"listRendererCmds",
		"listRenderLightDefs",
		"listServers",
		"listSkins",
		"listSoundCmds",
		"listSoundDecoders",
		"listSounds",
		"listSoundShaders",
		"listSpawnArgs",
		"listSystemCmds",
		"listTables",
		"listThreads",
		"listToolCmds",
		"listTypeInfo",
		"listVertexCache",
		"listVideos",
		"loadGame",
		"localizeGuiParmsTest",
		"localizeGuis",
		"localizeMaps",
		"localizeMapsTest",
		"makeAmbientMap",
		//"MakeMegaTexture       processes giant images
		"map",
		"memoryDump",
		"memoryDumpCompressed",
		"modulateLights",
		"nextAnim",
		"nextFrame",
		"nextGUI",
		"nextMap",
		"noclip",
		"notarget",
		"parse",
		"path",
		"playCmdDemo",
		"playDemo",
		"playerModel",
		"popLight",
		"prevAnim",
		"prevFrame",
		"printAF",
		"printAudio",
		"printEmail",
		"printEntityDef",
		"printFX",
		"printMaterial",
		"printMemInfo",
		"printModel",
		"printModelDefs",
		"printParticle",
		"printPDA",
		"printSkin",
		"printSoundShader",
		"printTable",
		"printVideo",
		"promptKey",
		//"quit",
		"rcon",
		"reconnect",
		"recordDemo",
		"recordViewNotes",
		"reexportmodels",
		"regenerateWorld",
		"reloadanims",
		"reloadDecls",
		//"reloadEngine",
		"reloadGuis",
		"reloadImages",
		"reloadLanguage",
		"reloadModels",
		"reloadScript",
		"reloadShaders",
		"reloadSounds",
		"reloadSurface",
		"remove",
		"removeline",
		"reportImageDuplication",
		"reportSurfaceAreas",
		"rescanSI",
		"reset",
		"roq",
		"runAAS",
		"runAASDir",
		"runReach",
		"s_restart",
		"saveGame ",
		"saveLights",
		"saveMoveables",
		"saveParticles",
		"saveRagdolls",
		"saveSelected",
		"say",
		"sayTeam",
		"screenshot",
		"script",
		"serverForceReady",
		"serverInfo",
		"serverMapRestart",
		"serverNextMap",
		"set",
		"seta",
		"setMachineSpec",
		"sets",
		"sett",
		"setu",
		"setviewpos",
		"showDictMemory",
		"showInteractionMemory",
		"showStringMemory",
		"showTriSurfMemory",
		"showViewNotes",
		"sizeDown",
		"sizeUp",
		"spawn",
		"spawnServer",
		"startBuild",
		"stopRecording",
		"takeViewNotes",
		"takeViewNotes2",
		"teleport",
		"testAnim",
		"testBlend",
		"testBoneFx",
		"testDamage",
		"testDeath",
		"testFx",
		"testGUI",
		"testid",
		"testImage",
		"testLight",
		"testmap",
		"testModel",
		"testParticleStopTime",
		"testPointLight",
		"testSave",
		"testSaveGame",
		"testShaderParm",
		"testSIMD",
		"testSkin",
		"testSound",
		"testVideo",
		"timeCmdDemo",
		"timeDemo",
		"timeDemoQuit",
		"toggle",
		"touch",
		"touchFile",
		"touchFileList",
		"touchGui",
		"touchModel",
		"trigger",
		"unbind",
		"unbindall",
		"unbindRagdoll",
		"updateUI",
		"vid_restart",
		"vstr",
		"wait",
		"weaponSplat",
		"where",
		"writeCmdDemo",
		"writeConfig",
		"writeGameState",
		"writePrecache",
	};
}

