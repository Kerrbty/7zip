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

CDiskOutStream::CDiskOutStream(std::wstring fileName) :
        m_strFileName(fileName),
        m_strFileExt(L"")
{
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