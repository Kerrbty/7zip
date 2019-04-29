// Test7Zip.cpp : Defines the entry point for the console application.
//
#include <Windows.h>
#include <tchar.h>
#include "lib7zip.h"
#include <iostream>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")


void mkpath(WCHAR* lpPath)
{
    WCHAR* lpCopyPath = new WCHAR[wcslen(lpPath)+1];
    wcscpy(lpCopyPath, lpPath);

    PathRemoveFileSpecW(lpCopyPath);
    if ( !PathFileExists(lpCopyPath) )
    {
        mkpath(lpCopyPath);
        CreateDirectoryW(lpCopyPath, NULL);    
    }
    delete []lpCopyPath;
}


int wmain(int argc, WCHAR* argv[])
{
    C7ZipLibrary lib;

    if (!lib.Initialize()) {
        wprintf(L"initialize fail!\n");
        return 1;
    }

    for (int i=1; i<argc; i++)
    {
        C7ZipArchive * pArchive = NULL;

        CDiskInStream stream(argv[i]);
        wprintf(L"File Formats: %ls\n", stream.GetExt().c_str());

        WCHAR* lpPath = new WCHAR[2014];
        wcscpy(lpPath, argv[i]);
        PathRemoveFileSpecW(lpPath);
        wcscat(lpPath, L"\\out\\");
        CreateDirectoryW(lpPath, NULL);
        int len = wcslen(lpPath);
        if (lib.OpenArchive(&stream, &pArchive)) {
            unsigned int numItems = 0;

            pArchive->GetItemCount(&numItems);

            wprintf(L"numItems: %d\n", numItems);

            for(unsigned int i = 0;i < numItems;i++) {
                C7ZipArchiveItem * pArchiveItem = NULL;

                if (pArchive->GetItemInfo(i, &pArchiveItem)) {
                    wprintf(L"%d,%ls,%d\n", pArchiveItem->GetArchiveIndex(),
                        pArchiveItem->GetFullPath().c_str(),
                        pArchiveItem->IsDir());

//                     wprintf(L"get all properties\n");
//                     for(lib7zip::PropertyIndexEnum index = lib7zip::kpidPackSize;
//                         index <= lib7zip::kpidIsDir;
//                         index = (lib7zip::PropertyIndexEnum)(index + 1)) {
//                             wstring strVal = L"";
// 
//                             if ( pArchiveItem->GetStringProperty(index, strVal) == 0)
//                             {
//                                 wprintf(L"file name=%s\n", strVal.c_str());
//                             }
//                     }
                    wcscpy(lpPath+len, pArchiveItem->GetFullPath().c_str());
                    if (pArchiveItem->IsDir())
                    {
                        CreateDirectoryW(lpPath, NULL);
                    }
                    else
                    {
                        mkpath(lpPath);
                        CDiskOutStream oStream(lpPath);
                        pArchive->Extract(pArchiveItem, &oStream);
                    }
                } //if
            }//for
            
        }
        else {
            wprintf(L"open archive Test7Zip.7z fail\n");
        }
        if (pArchive != NULL)
            delete pArchive;
    }


    return 0;
}
