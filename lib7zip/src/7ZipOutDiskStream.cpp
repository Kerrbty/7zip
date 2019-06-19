#include "C/7zVersion.h"
#include "CPP/7zip/Archive/IArchive.h"
#include "CPP/Windows/PropVariant.h"
#include "CPP/Common/MyCom.h"
#include "CPP/7zip/ICoder.h"
#include "CPP/7zip/IPassword.h"
#include "Common/ComTry.h"
#include "Windows/PropVariant.h"
using namespace NWindows;

#include <Windows.h>
#include <tchar.h>

#include "lib7zip.h"
#include "7ZipOutDiskStream.h"
#include "7ZipGetFileType.h"

static wchar_t* GetPathStr(wchar_t* szPath)
{
    if (szPath == NULL) 
    {
        return NULL;
    }

    size_t len = wcslen(szPath);
    // if path is '//////' return '/' 
    do {
        if (len == 1 && (szPath[0] == L'\\' || szPath[0] == L'/')) 
        {
            return szPath;
        }

        if (szPath[len - 1] == L'\\' || szPath[len - 1] == L'/') 
        {
            szPath[len - 1] = L'\0';
            len--;
        }
        else 
        {
            break;
        }
    } while (true);

    wchar_t *lastx = wcsrchr(szPath, L'/');
    wchar_t *nlastx = wcsrchr(szPath, L'\\');
    if (lastx < nlastx) 
    {
        lastx = nlastx;
    }
    if (lastx != NULL) 
    {
        *lastx = L'\0';
        if (lastx == szPath)
        {
            *lastx++ = L'.';
            *lastx = L'\0'; 
        }
    }
    return szPath;
}

static bool CreateMulDirectory(const wchar_t *szPath)
{
    bool mksuc = 0;
    if (szPath == NULL) 
    {
        return 1;
    }

    int len = wcslen(szPath);
    wchar_t *lp = (wchar_t*)malloc((len + 2)*sizeof(wchar_t));
    wchar_t *p = lp;
    const wchar_t *q = szPath;

    memset(lp, 0, (len + 2)*sizeof(wchar_t));
    while (*q != L'\0') 
    {
        if (*q == L'\\' || *q == L'/') 
        {
            mksuc = CreateDirectoryW(lp, NULL) ? 0 : 1;
            *p = L'\\';
        }
        else 
        {
            *p = *q;
        }
        p++;
        q++;
    }
    CreateDirectoryW(lp, NULL);
    free(lp);
    return mksuc;
}

CDiskOutStream::CDiskOutStream(std::wstring fileName) :
        m_strFileName(fileName),
        m_strFileExt(L"")
{
	wchar_t* lpFileName = (wchar_t*)malloc((fileName.length()+1)*sizeof(wchar_t));
    if (lpFileName!=NULL)
    {
        wcscpy(lpFileName, fileName.c_str());
        CreateMulDirectory(GetPathStr(lpFileName));
        free(lpFileName);
    }
	
    m_pFile = CreateFileW(fileName.c_str(), GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    m_nFileSize = 0;
    size_t pos = m_strFileName.find_last_of(L".");
    if (pos != m_strFileName.npos)
    {
        m_strFileExt = m_strFileName.substr(pos + 1);
    }
}

CDiskOutStream::~CDiskOutStream()
{
    if (m_pFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_pFile);
    }    
}

int CDiskOutStream::GetFileSize() const
{
    if (m_pFile == INVALID_HANDLE_VALUE)
    {
        return 0;
    }
    return ::GetFileSize(m_pFile, NULL);
}

int CDiskOutStream::Write(const void *data, unsigned int size, unsigned int *processedSize)
{
    if (m_pFile == INVALID_HANDLE_VALUE)
    {
        return 1;
    }
    
    DWORD dwBytes = 0;
    if (WriteFile(m_pFile, data, size, &dwBytes, NULL) && dwBytes>0)
    {
        if (processedSize != NULL)
            *processedSize = dwBytes;
        return 0;
    }
    return 1;
}

int CDiskOutStream::Seek(__int64 offset, unsigned int seekOrigin, unsigned __int64 *newPosition)
{
    if (m_pFile == INVALID_HANDLE_VALUE)
    {
        return 1;
    }
    
    DWORD dwSeek = FILE_BEGIN;
    switch(seekOrigin)
    {
    case SEEK_SET:
        dwSeek = FILE_BEGIN;
        break;
    case SEEK_CUR:
        dwSeek = FILE_CURRENT;
        break;
    case SEEK_END:
        dwSeek = FILE_END;
        break;
    }
    LARGE_INTEGER laSet = {0};
    LARGE_INTEGER laNew = {0};
    laSet.QuadPart = offset;
    if( SetFilePointerEx(m_pFile, laSet, &laNew, dwSeek) )
    {
        if (newPosition)
            *newPosition = laNew.QuadPart;

        return 0;
    }
    return 1;
}

int CDiskOutStream::SetSize(unsigned __int64 size)
{
    if (m_pFile == INVALID_HANDLE_VALUE)
    {
        return 1;
    }

    LARGE_INTEGER laSet = {0};
    LARGE_INTEGER laCur = {0};
    SetFilePointerEx(m_pFile, laSet, &laCur, FILE_CURRENT);

    laSet.QuadPart = size;
    SetFilePointerEx(m_pFile, laSet, &laSet, FILE_BEGIN);
    SetEndOfFile(m_pFile);

    SetFilePointerEx(m_pFile, laCur, &laCur, FILE_CURRENT);
    return 0;
}