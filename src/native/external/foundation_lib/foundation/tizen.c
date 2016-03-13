/* tizen.c  -  Foundation library  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
 *
 * This library provides a cross-platform foundation library in C11 providing basic support data types and
 * functions to write applications and games in a platform-independent fashion. The latest source code is
 * always available at
 *
 * https://github.com/rampantpixels/foundation_lib
 *
 * This library is put in the public domain; you can redistribute it and/or modify it without any restrictions.
 *
 */

#include <foundation/platform.h>
#include <foundation/tizen.h>

#if FOUNDATION_PLATFORM_TIZEN


static app_event_handler_h _event_handlers[5];


static bool _tizen_app_create( void* data )
{
    FOUNDATION_UNUSED( data );
    return true;
}


static void _tizen_app_control( app_control_h control, void* data )
{
    FOUNDATION_UNUSED( control );
    FOUNDATION_UNUSED( data );
}


static void _tizen_app_pause( void* data )
{
    FOUNDATION_UNUSED( data );
}


static void _tizen_app_resume( void* data )
{
    FOUNDATION_UNUSED( data );
}


static void _tizen_app_terminate( void* data )
{
    FOUNDATION_UNUSED( data );
}


static void _tizen_app_lang_changed( app_event_info_h event_info, void* data )
{
    FOUNDATION_UNUSED( event_info );
    FOUNDATION_UNUSED( data );
}


static void _tizen_app_orient_changed( app_event_info_h event_info, void* data )
{
    FOUNDATION_UNUSED( event_info );
    FOUNDATION_UNUSED( data );
}


static void _tizen_app_region_changed( app_event_info_h event_info, void* data )
{
    FOUNDATION_UNUSED( event_info );
    FOUNDATION_UNUSED( data );
}


static void _tizen_app_low_battery( app_event_info_h event_info, void* data )
{
    FOUNDATION_UNUSED( event_info );
    FOUNDATION_UNUSED( data );
}


static void _tizen_app_low_memory( app_event_info_h event_info, void* data )
{
    FOUNDATION_UNUSED( event_info );
    FOUNDATION_UNUSED( data );
}


int tizen_initialize( void )
{
    ui_app_add_event_handler( &_event_handlers[APP_EVENT_LOW_BATTERY], APP_EVENT_LOW_BATTERY, _tizen_app_low_battery, 0 );
    ui_app_add_event_handler( &_event_handlers[APP_EVENT_LOW_MEMORY], APP_EVENT_LOW_MEMORY, _tizen_app_low_memory, 0 );
    ui_app_add_event_handler( &_event_handlers[APP_EVENT_DEVICE_ORIENTATION_CHANGED], APP_EVENT_DEVICE_ORIENTATION_CHANGED, _tizen_app_orient_changed, 0 );
    ui_app_add_event_handler( &_event_handlers[APP_EVENT_LANGUAGE_CHANGED], APP_EVENT_LANGUAGE_CHANGED, _tizen_app_lang_changed, 0 );
    ui_app_add_event_handler( &_event_handlers[APP_EVENT_REGION_FORMAT_CHANGED], APP_EVENT_REGION_FORMAT_CHANGED, _tizen_app_region_changed, 0 );
    return 0;
}


void tizen_shutdown( void )
{
}


void tizen_start_main_thread( void )
{
}


int tizen_app_main( int argc, char** argv )
{
    ui_app_lifecycle_callback_s event_callback;
    memset( &event_callback, 0, sizeof( event_callback ) );
    event_callback.create = _tizen_app_create;
    event_callback.terminate = _tizen_app_terminate;
    event_callback.pause = _tizen_app_pause;
    event_callback.resume = _tizen_app_resume;
    event_callback.app_control = _tizen_app_control;

    return ui_app_main( argc, argv, &event_callback, 0 );
}


#endif
