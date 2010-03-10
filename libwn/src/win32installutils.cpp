#if 0 // THIS IS JUST FOR REFERENCE!
#include <vcl.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#pragma hdrstop

#include <memory>
#include "jdk_world.h"
#include "win32installutils.h"
#include "jdk_util.h"

TRegistry *settings=0;

void Install_Registry(bool erase, const char *loc)
{
    settings = new TRegistry;

    settings->LazyWrite = false;
    settings->RootKey = HKEY_LOCAL_MACHINE;

    if( erase )
    {
        settings->DeleteKey( loc );
    }
    if( !settings->OpenKey( loc, true ) )
    {
        throw "cant open key";
    }
}

void Install_RegistryEnd()
{
    delete settings;
}

bool Install_InstallFiles( filecopylist *list )
{
	char sysdir[1024];

    GetSystemDirectory( sysdir, sizeof(sysdir) );


    for( ;list->src;++list )
    {
        char realdest[1024];
        strcpy( realdest, sysdir );
        strcat( realdest, "\\" );
        strcat( realdest, list->dest );

        CopyFile( list->src, realdest, false );
    }
    return true;
}

bool Install_DeleteFiles( filecopylist *list )
{
	char sysdir[1024];

    GetSystemDirectory( sysdir, sizeof(sysdir) );


    for( ;list->src;++list )
    {
        char realdest[1024];
        strcpy( realdest, sysdir );
        strcat( realdest, "\\" );
        strcat( realdest, list->dest );

        DeleteFile( realdest );
    }
    return true;
}

bool Install_AddToRun( const char *key, const char *program )
{
    std::auto_ptr<TRegistry> r( new TRegistry );
    r->LazyWrite = false;
    r->RootKey = HKEY_LOCAL_MACHINE;
    if( r->OpenKey( "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", false ) )
    {
        r->WriteString( key, program );
        return true;
    }
    return false;
}

bool Install_AddToRunOnce( const char *key, const char *program )
{
    std::auto_ptr<TRegistry> r( new TRegistry );
    r->LazyWrite = false;
    r->RootKey = HKEY_LOCAL_MACHINE;
    if( r->OpenKey( "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce", false ) )
    {
        r->WriteString( key, program );
        return true;
    }
    return false;

}

bool Install_RemoveFromRun( const char *key )
{
    std::auto_ptr<TRegistry> r( new TRegistry );
    r->LazyWrite = false;
    r->RootKey = HKEY_LOCAL_MACHINE;
    if( r->OpenKey( "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", false ) )
    {
        r->DeleteValue( key );
        return true;
    }
    return false;

}

bool Install_RemoveFromRunOnce( const char *key )
{
    std::auto_ptr<TRegistry> r( new TRegistry );
    r->LazyWrite = false;
    r->RootKey = HKEY_LOCAL_MACHINE;
    if( r->OpenKey( "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce", false ) )
    {
        r->DeleteValue( key );
        return true;
    }
    return false;

}


bool Install_CalcPath( const char *orig, char *dest, int destsize )
{
    if( !orig || !*orig )
        return false;

    if( strchr( orig, '\\' ) )
    {
        jdk_strncpy( dest, orig, destsize );
    }
    else
    {
        GetSystemDirectory( dest, destsize );
        jdk_strncat( dest, '\\', destsize );
        jdk_strncat( dest, orig, destsize );
    }
    return true;
}


HWND Install_CreateInvisibleWindow(
                                    HINSTANCE instance,
                                    const char *clsname,
                                    const char *title,
                                    WNDPROC proc
                                    )
{
    HWND w=0;

    WNDCLASS *cls = new WNDCLASS;
    cls->style = 0;
    cls->lpfnWndProc = proc;
    cls->cbClsExtra = 0;
    cls->cbWndExtra = 0;
    cls->hInstance = instance;
    cls->hIcon = 0;
    cls->hCursor = 0;
    cls->hbrBackground = 0;
    cls->lpszMenuName = 0;
    cls->lpszClassName = new char[ strlen(clsname)+1 ];
    strcpy( (char *)cls->lpszClassName, clsname );

    if( RegisterClass( cls ) )
    {
        w = CreateWindow(
                        clsname,
                        title,
                        0, // invisible
                        0,0,10,10,
                        0, // no parent
                        0, // no menu
                        instance,
                        0  // no createparam
                        );
    }

    return w;
}


LRESULT Install_SendMsgToWindow(
                            const char *clsname,
                            const char *title,
                            UINT msg,
                            WPARAM wparam,
                            LPARAM lparam
                            )
{
    HWND w = FindWindow( clsname, title );
    if( w )
    {
        return SendMessage( w, msg, wparam, lparam );
    }
    return -1;
}

#endif
