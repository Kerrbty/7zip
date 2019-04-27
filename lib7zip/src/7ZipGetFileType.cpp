#include "7ZipGetFileType.h"
#include <windows.h>
#include <tchar.h>

#define PE_HEADER_LENGTH  (0x00001000)

const bool AnalyzeZipFile(C7ZipInStream *InStream, wstring &Ext)
{
    return false;
}

const bool AnalyzeGzFile(C7ZipInStream *InStream, wstring &Ext)
{
    return false;
}


const bool AnalyzeInPETail(unsigned char* pbuf, bool* bFindType, wstring &Ext)
{
    if (memcmp(pbuf, "\x52\x61\x72\x21\x1A\x07", 6) == 0)
    {
        Ext = L"rar";
        *bFindType = true;
        return true;
    }
    else if (memcmp(pbuf, "\x00\x00\x00\x00\xEF\xBE\xAD\xDE\x4E\x75\x6C\x6C\x73\x6F\x66\x74\x49\x6E\x73\x74", 0x10) == 0)
    {
        Ext = L"nsis";
        *bFindType = true;
        return true;
    }
    return false;
}

const bool AnalyzePEFile64(C7ZipInStream *InStream, wstring &Ext)
{
    bool bFindType = false;

    unsigned char* pImageBuf = new unsigned char[PE_HEADER_LENGTH];
    InStream->Seek(0, SEEK_SET, NULL);
    InStream->Read(pImageBuf, PE_HEADER_LENGTH, NULL);
    do 
    {
        unsigned __int64 uiFileSize = 0;
        InStream->GetSize(&uiFileSize);
        PIMAGE_DOS_HEADER MzHeader = (PIMAGE_DOS_HEADER)pImageBuf;
        PIMAGE_NT_HEADERS64 PeHeader = 
            (PIMAGE_NT_HEADERS64)((unsigned char*)MzHeader + MzHeader->e_lfanew);

        unsigned short SectionNum = PeHeader->FileHeader.NumberOfSections; 
        PIMAGE_SECTION_HEADER SectionHeader = (PIMAGE_SECTION_HEADER)( (unsigned char*)PeHeader + 
            sizeof(PeHeader->FileHeader) + 
            sizeof(PeHeader->Signature) +
            PeHeader->FileHeader.SizeOfOptionalHeader ); // 节表项的开始
        // 判断段表位置 
        if ( (unsigned char*)SectionHeader-pImageBuf >PE_HEADER_LENGTH )
        {
            break;
        }

        // 找到PE段尾部 
        unsigned long ulOff = 0;
        for (unsigned long i=0; i<SectionNum; i++) // 将节一个个复制到内存中
        {
            unsigned long ulsize = SectionHeader[i].PointerToRawData + SectionHeader[i].SizeOfRawData;
            if (ulsize > ulOff)
            {
                ulOff = ulsize;
            }
        } 
        // 查看PE是否有签名数据，如果紧跟PE末端则跳过签名数据，否则不跳 
        if (PeHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_SECURITY].VirtualAddress < ulOff+0x100)
        {
            unsigned long ulsize = 
                PeHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_SECURITY].VirtualAddress + 
                PeHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_SECURITY].Size;
            if (ulsize > ulOff)
            {
                ulOff = ulsize;
            }
        }
        if (ulOff+0x20 >= uiFileSize)
        {
            // 尾部没有数据 或者 非正常结尾PE  
            break;
        }

        InStream->Seek(ulOff, SEEK_SET, NULL);

        memset(pImageBuf, 0, PE_HEADER_LENGTH);
        InStream->Read(pImageBuf, 0x20, NULL);
        if (AnalyzeInPETail(pImageBuf, &bFindType, Ext))
        {
            break;
        }
        else if (ulOff+0x1000 <= uiFileSize)
        {
            // 这个判断是没有依据的 
            Ext = L"7z";
            bFindType = true;
            break;
        }
    }while(false);
    delete  []pImageBuf;

    return bFindType;
}

const bool AnalyzePEFile32(C7ZipInStream *InStream, wstring &Ext)
{
    bool bFindType = false;
    // 根据PEID的判断法则 
    unsigned __int64 uiFileSize = 0;
    InStream->GetSize(&uiFileSize);
    if (uiFileSize < 0x400)
    {
        return bFindType;
    }

    unsigned char* pImageBuf = new unsigned char[PE_HEADER_LENGTH];
    InStream->Seek(0, SEEK_SET, NULL);
    InStream->Read(pImageBuf, PE_HEADER_LENGTH, NULL);
    do 
    {
        PIMAGE_DOS_HEADER MzHeader = (PIMAGE_DOS_HEADER)pImageBuf;
        PIMAGE_NT_HEADERS32 PeHeader = 
            (PIMAGE_NT_HEADERS32)((unsigned char*)MzHeader + MzHeader->e_lfanew);
        if ((unsigned __int64)MzHeader->e_lfanew>PE_HEADER_LENGTH || PeHeader->Signature != IMAGE_NT_SIGNATURE)
        {
            break;
        }

        // 目前只支持x86/x64分析 
        if (PeHeader->FileHeader.Machine == IMAGE_FILE_MACHINE_AMD64)
        {
            bFindType = AnalyzePEFile64(InStream, Ext);
            break;
        }
        else if (PeHeader->FileHeader.Machine != IMAGE_FILE_MACHINE_I386)
        {
            break;
        }
        
        unsigned short SectionNum = PeHeader->FileHeader.NumberOfSections; 
        PIMAGE_SECTION_HEADER SectionHeader = (PIMAGE_SECTION_HEADER)( (unsigned char*)PeHeader + 
            sizeof(PeHeader->FileHeader) + 
            sizeof(PeHeader->Signature) +
            PeHeader->FileHeader.SizeOfOptionalHeader ); // 节表项的开始
        // 判断段表位置 
        if ( (unsigned char*)SectionHeader-pImageBuf >PE_HEADER_LENGTH )
        {
            break;
        }

        // 找到PE段尾部 
        unsigned long ulOff = 0;
        for (unsigned long i=0; i<SectionNum; i++) // 将节一个个复制到内存中
        {
            unsigned long ulsize = SectionHeader[i].PointerToRawData + SectionHeader[i].SizeOfRawData;
            if (ulsize > ulOff)
            {
                ulOff = ulsize;
            }
        } 
        // 查看PE是否有签名数据，如果紧跟PE末端则跳过签名数据，否则不跳 
        if (PeHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_SECURITY].VirtualAddress < ulOff+0x100)
        {
            unsigned long ulsize = 
                PeHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_SECURITY].VirtualAddress + 
                PeHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_SECURITY].Size;
            if (ulsize > ulOff)
            {
                ulOff = ulsize;
            }
        }
        if (ulOff+0x20 >= uiFileSize)
        {
            // 尾部没有数据 或者 非正常结尾PE  
            break;
        }

        InStream->Seek(ulOff, SEEK_SET, NULL);

        memset(pImageBuf, 0, PE_HEADER_LENGTH);
        InStream->Read(pImageBuf, 0x20, NULL); 
        if (AnalyzeInPETail(pImageBuf, &bFindType, Ext))
        {
            break;
        }
        else if (ulOff+0x1000 <= uiFileSize)
        {
            // 这个判断是没有依据的 
            Ext = L"7z";
            bFindType = true;
            break;
        }
    } while (false);
    delete []pImageBuf;

    return bFindType;
}

const bool AnalyzeTarFile(C7ZipInStream *InStream, wstring &Ext)
{
    return false;
}

const bool Analyze7zFile(C7ZipInStream *InStream, wstring &Ext)
{
    return false;
}

const bool AnalyzeRarFile(C7ZipInStream *InStream, wstring &Ext)
{
    return false;
}

const bool AnalyzeBz2File(C7ZipInStream *InStream, wstring &Ext)
{
    return false;
}


bool C7ZipGetFileType(C7ZipInStream *InStream, wstring &Ext)
{
    Ext = L"";
    // 保存当前位置 
    unsigned __int64 curPosition = 0;
    InStream->Seek(0, SEEK_CUR, &curPosition);

    // 设置到头部,读取0x400 
#define READ_BUF_SIZE  (0x00000100) 
    unsigned int readlen = 0;
    unsigned char* pbuf = new unsigned char[READ_BUF_SIZE];
    InStream->Seek(0, SEEK_SET, NULL);
    do 
    {
        if( InStream->Read(pbuf, 0x20, &readlen) == 0 && readlen>0x10)
        {
            if (pbuf[0] == 0x1F)
            {
                if (pbuf[1] == 0xA0) // 1F0A 
                {
                    if (!AnalyzeTarFile(InStream, Ext))
                    {
                        Ext = L"tar";
                    }
                }
                else if (pbuf[1] == 0x8B) // 1F8B 
                {
                    if (pbuf[2] == 0x08) // 1F8B08 
                    {
                        if (!AnalyzeGzFile(InStream, Ext))
                        {
                            Ext = L"gz";
                        }
                    }
                }
            }
            else if (pbuf[0] == 0x37)
            {
                if (pbuf[1] == 0x7A)
                {
                    if (pbuf[2] == 0xBC)
                    {
                        if (pbuf[0] == 0xAF)
                        {
                            if (pbuf[0] == 0x27)
                            {
                                if (pbuf[0] == 0x1C)
                                {
                                    if (!Analyze7zFile(InStream, Ext))
                                    {
                                        Ext = L"7z";
                                    }
                                }
                            }
                        }
                    }
                }
            }
            else if (pbuf[0] == 0x42)
            {
                if (pbuf[1] == 0x5A)
                {
                    if (pbuf[2] == 0x68)
                    {
                        if (!AnalyzeBz2File(InStream, Ext))
                        {
                            Ext = L"bz2";
                        }
                    }
                }
            }
            else if (pbuf[0] == 0x4D)
            {
                if (pbuf[1] == 0x5A)
                {
                    if (!AnalyzePEFile32(InStream, Ext))
                    {
                        Ext = L"exe";
                    }
                }
            }
            else if (pbuf[0] == 0x50)
            {
                if (pbuf[1] == 0x4B) // 504B 
                {
                    if (pbuf[2] == 0x03) // 504B03 
                    {
                        if (pbuf[3] == 0x04) // 504B0304 
                        {
                            if (!AnalyzeZipFile(InStream, Ext))
                            {
                                Ext = L"zip";
                            }
                        }
                    }
                }
            }
            else if (pbuf[0] == 0x52)
            {
                if (pbuf[1] == 0x61) // 5261 
                {
                    if (pbuf[2] == 0x72) // 526172 
                    {
                        if (pbuf[3] == 0x21) // 52617221 
                        {
                            if (pbuf[4] == 0x1A) // 526172211A 
                            {
                                if (pbuf[5] == 0x07) // 526172211A07 
                                {
                                    if (!AnalyzeRarFile(InStream, Ext))
                                    {
                                        Ext = L"rar";
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    } while (false);
    delete []pbuf;

    // 设置会原位置 
    InStream->Seek(curPosition, SEEK_SET, NULL);
    if (Ext.length()>0)
    {
        return true;
    }
    return false;
}