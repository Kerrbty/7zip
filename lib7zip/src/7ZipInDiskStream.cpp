#include "C/7zVersion.h"
#include "CPP/7zip/Archive/IArchive.h"
#include "CPP/Windows/PropVariant.h"
#include "CPP/Common/MyCom.h"
#include "CPP/7zip/ICoder.h"
#include "CPP/7zip/IPassword.h"
#include "Common/ComTry.h"
#include "Windows/PropVariant.h"
using namespace NWindows;

#include "lib7zip.h"
#include "7ZipInDiskStream.h"
#include "7ZipGetFileType.h"

CDiskInStream::CDiskInStream(std::wstring fileName) :
                        m_strFileName(fileName),
                        m_strFileExt(L"7z")
{
    m_pFile = _wfopen(fileName.c_str(), L"rb");
    if (m_pFile) 
    {
        fseek(m_pFile, 0, SEEK_END);
        m_nFileSize = ftell(m_pFile);
        fseek(m_pFile, 0, SEEK_SET);

        // 先根据文件内容判断，无法判断的再使用后缀 
        if ( !C7ZipGetFileType(this, m_strFileExt) )
        {
            // 次要情况使用缀名 
            size_t pos = m_strFileName.find_last_of(L".");
            if (pos != m_strFileName.npos) 
            {
                m_strFileExt = m_strFileName.substr(pos + 1);
            }
        }
    }
}


CDiskInStream::~CDiskInStream()
{
    if (m_pFile)
    {
        fclose(m_pFile);
    }		
}

wstring CDiskInStream::GetExt() const
{
    return m_strFileExt;
}

int CDiskInStream::Read(void *data, unsigned int size, unsigned int *processedSize)
{
    if (!m_pFile)
        return 1;

    int count = fread(data, 1, size, m_pFile);
    if (count >= 0) {
        if (processedSize != NULL)
            *processedSize = count;

        return 0;
    }

    return 1;
}

int CDiskInStream::Seek(__int64 offset, unsigned int seekOrigin, unsigned __int64 *newPosition)
{
    if (!m_pFile)
        return 1;

    int result = fseek(m_pFile, (long)offset, seekOrigin);

    if (!result) {
        if (newPosition)
            *newPosition = ftell(m_pFile);

        return 0;
    }

    return result;
}

int CDiskInStream::GetSize(unsigned __int64 * size)
{
    if (size)
        *size = m_nFileSize;
    return 0;
}