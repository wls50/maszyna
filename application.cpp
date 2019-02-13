﻿/*
This Source Code Form is subject to the
terms of the Mozilla Public License, v.
2.0. If a copy of the MPL was not
distributed with this file, You can
obtain one at
http://mozilla.org/MPL/2.0/.
*/

#include "stdafx.h"
#include "application.h"
#include "drivermode.h"
#include "editormode.h"
#include "scenarioloadermode.h"

#include "Globals.h"
#include "simulation.h"
#include "Train.h"
#include "sceneeditor.h"
#include "renderer.h"
#include "uilayer.h"
#include "Logs.h"
#include "screenshot.h"
#include "translation.h"
#include "Train.h"
#include "Timer.h"

#pragma comment (lib, "glu32.lib")
#pragma comment (lib, "dsound.lib")
#pragma comment (lib, "winmm.lib")
#pragma comment (lib, "setupapi.lib")
#pragma comment (lib, "dbghelp.lib")
#pragma comment (lib, "version.lib")

#ifdef __linux__
#include <unistd.h>
#include <sys/stat.h>
#endif

eu07_application Application;
screenshot_manager screenshot_man;

ui_layer uilayerstaticinitializer;

#ifdef _WIN32
extern "C"
{
    GLFWAPI HWND glfwGetWin32Window( GLFWwindow* window );
}

LONG CALLBACK unhandled_handler( ::EXCEPTION_POINTERS* e );
LRESULT APIENTRY WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
extern HWND Hwnd;
extern WNDPROC BaseWindowProc;
#endif

// user input callbacks

void focus_callback( GLFWwindow *window, int focus ) {
	Application.on_focus_change(focus != 0);
}

void window_resize_callback( GLFWwindow *window, int w, int h ) {
    // NOTE: we have two variables which basically do the same thing as we don't have dynamic fullscreen toggle
    // TBD, TODO: merge them?
    Global.iWindowWidth = w;
    Global.iWindowHeight = h;
    Global.fDistanceFactor = std::max( 0.5f, h / 768.0f ); // not sure if this is really something we want to use
    glViewport( 0, 0, w, h );
}

void cursor_pos_callback( GLFWwindow *window, double x, double y ) {
    Application.on_cursor_pos( x, y );
}

void mouse_button_callback( GLFWwindow* window, int button, int action, int mods )
{
    Application.on_mouse_button( button, action, mods );
}

void scroll_callback( GLFWwindow* window, double xoffset, double yoffset ) {
    Application.on_scroll( xoffset, yoffset );
}

void key_callback( GLFWwindow *window, int key, int scancode, int action, int mods ) {

    Application.on_key( key, scancode, action, mods );
}

void char_callback(GLFWwindow *window, unsigned int c)
{
    Application.on_char(c);
}

// public:

void eu07_application::queue_screenshot()
{
    m_screenshot_queued = true;
}

int
eu07_application::init( int Argc, char *Argv[] ) {

    int result { 0 };

    init_debug();
    init_files();
    if( ( result = init_settings( Argc, Argv ) ) != 0 ) {
        return result;
    }

	WriteLog( "Starting MaSzyna rail vehicle simulator (release: " + Global.asVersion + ")" );
	WriteLog( "For online documentation and additional files refer to: http://eu07.pl" );
	WriteLog( "Authors: Marcin_EU, McZapkie, ABu, Winger, Tolaris, nbmx, OLO_EU, Bart, Quark-t, "
	    "ShaXbee, Oli_EU, youBy, KURS90, Ra, hunter, szociu, Stele, Q, firleju and others\n" );

    if( ( result = init_locale() ) != 0 ) {
        return result;
    }
    if( ( result = init_glfw() ) != 0 ) {
        return result;
    }
    if( ( result = init_gfx() ) != 0 ) {
        return result;
    }
    init_callbacks();
    if( ( result = init_audio() ) != 0 ) {
        return result;
    }
    m_taskqueue.init();
    if( ( result = init_modes() ) != 0 ) {
        return result;
    }

	if (!init_network())
		return -1;

    return result;
}

double eu07_application::generate_sync() {
	if (Timer::GetDeltaTime() == 0.0)
		return 0.0;
	double sync = 0.0;
	for (const TDynamicObject* vehicle : simulation::Vehicles.sequence()) {
		 glm::vec3 pos = vehicle->GetPosition();
		 sync += pos.x + pos.y + pos.z;
	}
	sync += Random(1.0, 100.0);
	return sync;
}

int
eu07_application::run() {

    // main application loop
    while (!glfwWindowShouldClose( m_windows.front() ) && !m_modestack.empty())
    {
        Timer::subsystem.mainloop_total.start();
        glfwPollEvents();

		// -------------------------------------------------------------------
		// multiplayer command relaying logic can seem a bit complex
		//
		// we are simultaneously:
		// => master (not client) OR slave (client)
		// => server OR not
		//
		// trivia: being client and server is possible

		double frameStartTime = Timer::GetTime();

		if (m_modes[m_modestack.top()]->is_command_processor()) {
			// active mode is doing real calculations (e.g. drivermode)

			int loop_remaining = MAX_NETWORK_PER_FRAME;
			while (--loop_remaining > 0)
			{
				command_queue::commands_map commands_to_exec;
				command_queue::commands_map local_commands = simulation::Commands.pop_intercept_queue();
				double slave_sync;

				// if we're the server
				if (m_network && m_network->servers)
				{
					// fetch from network layer command requests received from clients
					command_queue::commands_map remote_commands = m_network->servers->pop_commands();

					// push these into local queue
					add_to_dequemap(local_commands, remote_commands);
				}

				// if we're slave
				if (m_network && m_network->client)
				{
					// fetch frame info from network layer,
					auto frame_info = m_network->client->get_next_delta(MAX_NETWORK_PER_FRAME - loop_remaining);

					// use delta and commands received from master
					double delta = std::get<0>(frame_info);
					Timer::set_delta_override(delta);
					slave_sync = std::get<1>(frame_info);
					add_to_dequemap(commands_to_exec, std::get<2>(frame_info));

					// and send our local commands to master
					m_network->client->send_commands(local_commands);

					if (delta == 0.0)
						loop_remaining = -1;
				}
				// if we're master
				else {
					// just push local commands to execution
					add_to_dequemap(commands_to_exec, local_commands);

					loop_remaining = -1;
				}

				// send commands to command queue
				simulation::Commands.push_commands(commands_to_exec);

				// do actual frame processing
				if (!m_modes[ m_modestack.top() ]->update())
					goto die;

				// update continuous commands
				simulation::Commands.update();

				double sync = generate_sync();

				// if we're the server
				if (m_network && m_network->servers)
				{
					// send delta, sync, and commands we just executed to clients
					double delta = Timer::GetDeltaTime();
					double render = Timer::GetDeltaRenderTime();
					m_network->servers->push_delta(render, delta, sync, commands_to_exec);
				}

				// if we're slave
				if (m_network && m_network->client)
				{
					// verify sync
					if (sync != slave_sync) {
						WriteLog("net: desync! calculated: " + std::to_string(sync)
						         + ", received: " + std::to_string(slave_sync), logtype::net);
					}

					// set total delta for rendering code
					double totalDelta = Timer::GetTime() - frameStartTime;
					Timer::set_delta_override(totalDelta);
				}
			}

			if (!loop_remaining) {
				// loop break forced by counter
				float received = m_network->client->get_frame_counter();
				float awaiting = m_network->client->get_awaiting_frames();

				// TODO: don't meddle with mode progresbar
				m_modes[m_modestack.top()]->set_progress(100.0f, 100.0f * (received - awaiting) / received);
			} else {
				m_modes[m_modestack.top()]->set_progress(0.0f, 0.0f);
			}
		} else {
			// active mode is loader

			// clear local command queue
			simulation::Commands.pop_intercept_queue();

			// do actual frame processing
			if (!m_modes[ m_modestack.top() ]->update())
				goto die;
		}

		// -------------------------------------------------------------------

		m_taskqueue.update();
		opengl_texture::reset_unit_cache();

        if (!GfxRenderer.Render())
            break;

        GfxRenderer.SwapBuffers();

        if (m_modestack.empty())
            return 0;

        m_modes[ m_modestack.top() ]->on_event_poll();

        if (m_screenshot_queued) {
            m_screenshot_queued = false;
            screenshot_man.make_screenshot();
        }

		if (m_network)
			m_network->update();

		auto frametime = Timer::subsystem.mainloop_total.stop();
		if (Global.minframetime.count() != 0.0f && (Global.minframetime - frametime).count() > 0.0f)
			std::this_thread::sleep_for(Global.minframetime - frametime);
    }
    die:

    return 0;
}

// issues request for a worker thread to perform specified task. returns: true if task was scheduled
bool
eu07_application::request( python_taskqueue::task_request const &Task ) {

    auto const result { m_taskqueue.insert( Task ) };
    if( ( false == result )
     && ( Task.input != nullptr ) ) {
        // clean up allocated resources since the worker won't
        delete Task.input;
    }
    return result;
}

// ensures the main thread holds the python gil and can safely execute python calls
void
eu07_application::acquire_python_lock() {

    m_taskqueue.acquire_lock();
}

// frees the python gil and swaps out the main thread
void
eu07_application::release_python_lock() {

    m_taskqueue.release_lock();
}

void
eu07_application::exit() {

    SafeDelete( simulation::Train );
    SafeDelete( simulation::Region );

    ui_layer::shutdown();

    for( auto *window : m_windows ) {
        glfwDestroyWindow( window );
    }
    m_taskqueue.exit();
    glfwTerminate();
}

void
eu07_application::render_ui() {

    if( m_modestack.empty() ) { return; }

    m_modes[ m_modestack.top() ]->render_ui();
}

bool
eu07_application::pop_mode() {
    if( m_modestack.empty() ) { return false; }

    m_modes[ m_modestack.top() ]->exit();
    m_modestack.pop();
    return true;
}

bool
eu07_application::push_mode( eu07_application::mode const Mode ) {

    if( Mode >= mode::count_ ) { return false; }

    m_modes[ Mode ]->enter();
    m_modestack.push( Mode );

    return true;
}

void
eu07_application::set_title( std::string const &Title ) {

    glfwSetWindowTitle( m_windows.front(), Title.c_str() );
}

void
eu07_application::set_progress( float const Progress, float const Subtaskprogress ) {

    if( m_modestack.empty() ) { return; }

    m_modes[ m_modestack.top() ]->set_progress( Progress, Subtaskprogress );
}

void
eu07_application::set_cursor( int const Mode ) {

    ui_layer::set_cursor( Mode );
}

void
eu07_application::set_cursor_pos( double const Horizontal, double const Vertical ) {

    glfwSetCursorPos( m_windows.front(), Horizontal, Vertical );
}

glm::dvec2 eu07_application::get_cursor_pos() const {
    glm::dvec2 pos;
    if( !m_windows.empty() ) {
        glfwGetCursorPos( m_windows.front(), &pos.x, &pos.y );
    }
    return pos;
}

void
eu07_application::get_cursor_pos( double &Horizontal, double &Vertical ) const {

    glfwGetCursorPos( m_windows.front(), &Horizontal, &Vertical );
}

void
eu07_application::on_key( int const Key, int const Scancode, int const Action, int const Mods ) {

    if (ui_layer::key_callback(Key, Scancode, Action, Mods))
        return;

    if( m_modestack.empty() ) { return; }

    m_modes[ m_modestack.top() ]->on_key( Key, Scancode, Action, Mods );
}

void
eu07_application::on_cursor_pos( double const Horizontal, double const Vertical ) {

    if( m_modestack.empty() ) { return; }

    m_modes[ m_modestack.top() ]->on_cursor_pos( Horizontal, Vertical );
}

void
eu07_application::on_mouse_button( int const Button, int const Action, int const Mods ) {

    if (ui_layer::mouse_button_callback(Button, Action, Mods))
        return;

    if( m_modestack.empty() ) { return; }

    m_modes[ m_modestack.top() ]->on_mouse_button( Button, Action, Mods );
}

void
eu07_application::on_scroll( double const Xoffset, double const Yoffset ) {

    if (ui_layer::scroll_callback(Xoffset, Yoffset))
        return;

    if( m_modestack.empty() ) { return; }

    m_modes[ m_modestack.top() ]->on_scroll( Xoffset, Yoffset );
}

void eu07_application::on_char(unsigned int c) {
    if (ui_layer::char_callback(c))
        return;
}

void eu07_application::on_focus_change(bool focus) {
	if( Global.bInactivePause && !m_network->client) {// jeśli ma być pauzowanie okna w tle
		command_relay relay;
		relay.post(user_command::focuspauseset, focus ? 1.0 : 0.0, 0.0, GLFW_PRESS, 0);
	}
}

GLFWwindow *
eu07_application::window( int const Windowindex ) {

    if( Windowindex >= 0 ) {
        return (
            Windowindex < m_windows.size() ?
                m_windows[ Windowindex ] :
                nullptr );
    }
    // for index -1 create a new child window
    glfwWindowHint( GLFW_VISIBLE, GL_FALSE );
    auto *childwindow = glfwCreateWindow( 1, 1, "eu07helper", nullptr, m_windows.front() );
    if( childwindow != nullptr ) {
        m_windows.emplace_back( childwindow );
    }
    return childwindow;
}

// private:

void
eu07_application::init_debug() {

#if defined(_MSC_VER) && defined (_DEBUG)
    // memory leaks
    _CrtSetDbgFlag( _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG ) | _CRTDBG_LEAK_CHECK_DF );
    // floating point operation errors
    auto state { _clearfp() };
    state = _control87( 0, 0 );
    // this will turn on FPE for #IND and zerodiv
    state = _control87( state & ~( _EM_ZERODIVIDE | _EM_INVALID ), _MCW_EM );
#endif
#ifdef _WIN32
    ::SetUnhandledExceptionFilter( unhandled_handler );
#endif
}

void
eu07_application::init_files() {

#ifdef _WIN32
    DeleteFile( "log.txt" );
    DeleteFile( "errors.txt" );
    CreateDirectory( "logs", NULL );
#elif __linux__
	unlink("log.txt");
	unlink("errors.txt");
	mkdir("logs", 0755);
#endif
}

int
eu07_application::init_settings( int Argc, char *Argv[] ) {

    Global.LoadIniFile( "eu07.ini" );
#ifdef _WIN32
    if( ( Global.iWriteLogEnabled & 2 ) != 0 ) {
        // show output console if requested
        AllocConsole();
    }
#endif

	Global.asVersion = VERSION_INFO;

    // process command line arguments
    for( int i = 1; i < Argc; ++i ) {

        std::string token { Argv[ i ] };

        if( token == "-s" ) {
            if( i + 1 < Argc ) {
                Global.SceneryFile = ToLower( Argv[ ++i ] );
            }
        }
        else if( token == "-v" ) {
            if( i + 1 < Argc ) {
				Global.local_start_vehicle = ToLower( Argv[ ++i ] );
            }
        }
        else {
            std::cout
                << "usage: " << std::string( Argv[ 0 ] )
                << " [-s sceneryfilepath]"
                << " [-v vehiclename]"
                << std::endl;
            return -1;
        }
    }

    return 0;
}

int
eu07_application::init_locale() {

    locale::init();

    return 0;
}

int
eu07_application::init_glfw() {

    if( glfwInit() == GLFW_FALSE ) {
        ErrorLog( "Bad init: failed to initialize glfw" );
        return -1;
    }

    // match requested video mode to current to allow for
    // fullwindow creation when resolution is the same
    auto *monitor { glfwGetPrimaryMonitor() };
    auto const *vmode { glfwGetVideoMode( monitor ) };

    glfwWindowHint( GLFW_RED_BITS, vmode->redBits );
    glfwWindowHint( GLFW_GREEN_BITS, vmode->greenBits );
    glfwWindowHint( GLFW_BLUE_BITS, vmode->blueBits );
    glfwWindowHint( GLFW_REFRESH_RATE, vmode->refreshRate );

    glfwWindowHint(GLFW_SRGB_CAPABLE, !Global.gfx_shadergamma);

    if (!Global.gfx_usegles)
    {
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    }
    else
    {
        glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    }

    glfwWindowHint( GLFW_AUTO_ICONIFY, GLFW_FALSE );
    if (Global.gfx_skippipeline && Global.iMultisampling > 0) {
        glfwWindowHint( GLFW_SAMPLES, 1 << Global.iMultisampling );
    }

    auto *window {
        glfwCreateWindow(
            Global.iWindowWidth,
            Global.iWindowHeight,
            Global.AppName.c_str(),
            ( Global.bFullScreen ?
                monitor :
                nullptr ),
            nullptr ) };

    if( window == nullptr ) {
        ErrorLog( "Bad init: failed to create glfw window" );
        return -1;
    }

    glfwMakeContextCurrent( window );
    glfwSwapInterval( Global.VSync ? 1 : 0 ); //vsync

#ifdef _WIN32
// setup wrapper for base glfw window proc, to handle copydata messages
    Hwnd = glfwGetWin32Window( window );
    BaseWindowProc = ( WNDPROC )::SetWindowLongPtr( Hwnd, GWLP_WNDPROC, (LONG_PTR)WndProc );
    // switch off the topmost flag
    ::SetWindowPos( Hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
#endif

	if (Global.captureonstart)
	{
		Global.ControlPicking = false;
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
	else
		Global.ControlPicking = true;

    m_windows.emplace_back( window );

    return 0;
}

void
eu07_application::init_callbacks() {

    auto *window { m_windows.front() };
    glfwSetFramebufferSizeCallback( window, window_resize_callback );
    glfwSetCursorPosCallback( window, cursor_pos_callback );
    glfwSetMouseButtonCallback( window, mouse_button_callback );
    glfwSetKeyCallback( window, key_callback );
    glfwSetScrollCallback( window, scroll_callback );
    glfwSetCharCallback(window, char_callback);
    glfwSetWindowFocusCallback( window, focus_callback );
    {
        int width, height;
        glfwGetFramebufferSize( window, &width, &height );
        window_resize_callback( window, width, height );
    }
}

int
eu07_application::init_gfx() {

    if (!Global.gfx_usegles)
    {
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            ErrorLog( "Bad init: failed to initialize glad" );
            return -1;
        }
    }
    else
    {
        if (!gladLoadGLES2Loader((GLADloadproc)glfwGetProcAddress))
        {
            ErrorLog( "Bad init: failed to initialize glad" );
            return -1;
        }
    }

    if (!ui_layer::init(m_windows.front()))
        return -1;

    if (!GfxRenderer.Init(m_windows.front()))
        return -1;

    return 0;
}

int
eu07_application::init_audio() {

    if( Global.bSoundEnabled ) {
        Global.bSoundEnabled &= audio::renderer.init();
    }
    // NOTE: lack of audio isn't deemed a failure serious enough to throw in the towel
    return 0;
}

int
eu07_application::init_modes() {

    // NOTE: we could delay creation/initialization until transition to specific mode is requested,
    // but doing it in one go at the start saves us some error checking headache down the road

    // create all application behaviour modes
    m_modes[ mode::scenarioloader ] = std::make_shared<scenarioloader_mode>();
    m_modes[ mode::driver ] = std::make_shared<driver_mode>();
    m_modes[ mode::editor ] = std::make_shared<editor_mode>();
    // initialize the mode objects
    for( auto &mode : m_modes ) {
        if( false == mode->init() ) {
            return -1;
        }
    }
    // activate the default mode
    push_mode( mode::scenarioloader );

    return 0;
}

bool eu07_application::init_network() {
	if (!Global.network_servers.empty() || Global.network_client) {
		// create network manager
		m_network.emplace();
	}

	for (auto const &pair : Global.network_servers) {
		// create all servers
		m_network->create_server(pair.first, pair.second);
	}

	if (Global.network_client) {
		// create client
		m_network->connect(Global.network_client->first, Global.network_client->second);
	} else {
		// we're simulation master
		if (!Global.random_seed)
			Global.random_seed = std::random_device{}();
		Global.random_engine.seed(Global.random_seed);
		Global.ready_to_load = true;
	}

	return true;
}
