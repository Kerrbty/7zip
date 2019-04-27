#ifndef __7ZIP_IN_DISK_STREAM_H__
#define __7ZIP_IN_DISK_STREAM_H__

class CDiskInStream : public C7ZipInStream
{
private:
    FILE * m_pFile;
    std::wstring m_strFileName;
    std::wstring m_strFileExt;
    int m_nFileSize;
public:
    CDiskInStream(std::wstring fileName);
    virtual ~CDiskInStream();

public:
    virtual wstring GetExt() const;
    virtual int Read(void *data, unsigned int size, unsigned int *processedSize);
    virtual int Seek(__int64 offset, unsigned int seekOrigin, unsigned __int64 *newPosition);
    virtual int GetSize(unsigned __int64 * size);
};

#endif // __7ZIP_IN_DISK_STREAM_H__