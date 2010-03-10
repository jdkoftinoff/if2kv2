#ifndef __WNINSTALL_H
#define __WNINSTALL_H

#include "wnregistry.h"


struct filecopylist
{
    const char *src;
    const char *dest;
};

struct wn_archive_entry
{
    char src_filename[256];
    char dest_filename[256];
    unsigned long start_offset;
    unsigned long length;
    unsigned long magic_number;
};

#define WN_ARCHIVE_ENTRY_MAGIC (0x3141592)

struct wn_archive_header
{
    unsigned long num_entries;
    unsigned long entry_offset;
    unsigned long magic_number;
};

#define WN_ARCHIVE_HEADER_MAGIC (0x314159)

bool WNInstall_ExtractFileListFromArchive(
                    const char *filename,
                    wn_archive_header *header,
                    wn_archive_entry *entries,
                    unsigned long max_items
                    );

bool WNInstall_InstallFilesFromArchive( const char *fname );
bool WNInstall_CreateArchive(
                        const char *destfname,
                        const filecopylist *files
                        );
bool WNInstall_AppendFile(
                        FILE *dest,
                        const char *srcfile,
                        wn_archive_entry *entry
                        );

bool WNInstall_InstallFiles(
                FILE *f,
                const wn_archive_header *header,
                const wn_archive_entry *entries
                );

bool WNInstall_DeleteFiles(
                FILE *f,
                const wn_archive_header *header,
                const wn_archive_entry *entries
                );

bool WNInstall_DeleteFilesFromArchive( const char *fname );

bool WNInstall_InstallFiles( filecopylist * );
bool WNInstall_DeleteFiles( filecopylist * );


bool WNInstall_AddToRun( const char *key, const char *program );
bool WNInstall_AddToRunOnce( const char *key, const char *program );
bool WNInstall_AddToRunServices( const char *key, const char *program );

bool WNInstall_RemoveFromRun( const char *key );
bool WNInstall_RemoveFromRunOnce( const char *key );
bool WNInstall_RemoveFromRunServices( const char *key );

bool WNInstall_CalcPath( const char *orig, char *dest, int destlen );

HWND WNInstall_CreateInvisibleWindow(
                                    HINSTANCE instance,
                                    const char *clsname,
                                    const char *title,
                                    WNDPROC proc
                                    );
                                    
LRESULT WNInstall_SendMsgToWindow(
                            const char *clsname,
                            const char *title,
                            UINT msg,
                            WPARAM wparam,
                            LPARAM lparam
                            );

int WNInstall_CreateShortCut(
                            LPCSTR pszShortcutFile,
                            LPCSTR pszIconFile,
                            int iconindex,
                            LPCSTR pszExe,
                            LPCSTR pszArg,
                            LPCSTR workingdir,
                            int showmode,
                            int hotkey
                            );
                            
bool WNInstall_MoveFileOnReboot(const char * pszExisting, const char * pszNew);

void WNInstall_CreateDirectories(const char *directory);

bool WNInstall_FileExists(const char *buf);

void WNInstall_RemoveDirectories(const char *dirs, bool recurse);

int WNInstall_RegDeleteKeys(HKEY thiskey, const char * lpSubKey);

void WNInstall_GetUserShellFolder(const char *name, char *out);

bool WNInstall_IsWin95();
bool WNInstall_IsWin98();
bool WNInstall_IsWinME();
bool WNInstall_IsWinNT4();
bool WNInstall_IsWin2000();
bool WNInstall_IsWinXP();

int WNInstall_InternetGetConnectedState(LPDWORD pdwFlags);
bool WNInstall_InternetIsConnected();

bool WNInstall_OpenBrowser( const char *url );

#endif
