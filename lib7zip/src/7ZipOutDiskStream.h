#ifndef __7ZIP_OUT_DISK_STREAM_H__
#define __7ZIP_OUT_DISK_STREAM_H__

class CDiskOutStream : public C7ZipOutStream
{
private:
    void* m_pFile;
    std::wstring m_strFileName;
    std::wstring m_strFileExt;
    int m_nFileSize;
public:
    CDiskOutStream(std::wstring fileName);
    virtual ~CDiskOutStream();

public:
    int GetFileSize() const;
    virtual int Write(const void *data, unsigned int size, unsigned int *processedSize);
    virtual int Seek(__int64 offset, unsigned int seekOrigin, unsigned __int64 *newPosition);
    virtual int SetSize(unsigned __int64 size);
};

#endif