// Test7Zip.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "lib7zip.h"
#include <iostream>

const wchar_t * index_names[] = {
		L"kpidPackSize", //(Packed Size)
		L"kpidAttrib", //(Attributes)
		L"kpidCTime", //(Created)
		L"kpidATime", //(Accessed)
		L"kpidMTime", //(Modified)
		L"kpidSolid", //(Solid)
		L"kpidEncrypted", //(Encrypted)
		L"kpidUser", //(User)
		L"kpidGroup", //(Group)
		L"kpidComment", //(Comment)
		L"kpidPhySize", //(Physical Size)
		L"kpidHeadersSize", //(Headers Size)
		L"kpidChecksum", //(Checksum)
		L"kpidCharacts", //(Characteristics)
		L"kpidCreatorApp", //(Creator Application)
		L"kpidTotalSize", //(Total Size)
		L"kpidFreeSpace", //(Free Space)
		L"kpidClusterSize", //(Cluster Size)
		L"kpidVolumeName", //(Label)
		L"kpidPath", //(FullPath)
		L"kpidIsDir", //(IsDir)
};

#ifdef _WIN32
int _tmain(int argc, _TCHAR* argv[])
#else
	int main(int argc, char * argv[])
#endif
{
	C7ZipLibrary lib;

	if (!lib.Initialize()) {
		wprintf(L"initialize fail!\n");
		return 1;
	}

	WStringArray exts;

	if (!lib.GetSupportedExts(exts)) {
		wprintf(L"get supported exts fail\n");
		return 1;
	}

	size_t size = exts.size();

	for(size_t i = 0; i < size; i++) {
		wstring ext = exts[i];

		for(size_t j = 0; j < ext.size(); j++) {
			wprintf(L"%c", (char)(ext[j] &0xFF));
		}

		wprintf(L"\n");
	}

	C7ZipArchive * pArchive = NULL;

	CDiskInStream stream(L"./compress/Test7ZipCryptFileName.7z");

	if (!lib.OpenArchive(&stream, &pArchive)) {
		wprintf(L"open archive Test7ZipCryptFileName.7z without password, LastError(%X) == NEED_PASSWORD(%X)\n",
				lib.GetLastError(), lib7zip::LIB7ZIP_NEED_PASSWORD);
	}

	stream.Seek(0, FILE_BEGIN, NULL);

	if (lib.OpenArchive(&stream, &pArchive, wstring(L"hello"))) {
		unsigned int numItems = 0;

		pArchive->GetItemCount(&numItems);

		wprintf(L"%d, LastError=%X\n", numItems, lib.GetLastError());

		wprintf(L"get all archive properties\n");
		for(lib7zip::PropertyIndexEnum index = lib7zip::kpidPackSize;
			index <= lib7zip::kpidIsDir;
			index = (lib7zip::PropertyIndexEnum)(index + 1)) {
			wstring strVal = L"";
			unsigned __int64 val = 0;
			bool bVal = false;

			bool result = pArchive->GetUInt64Property(index, val);

			wprintf(L"\n\nGetProperty:%d %ls\n", (int)index,
					index_names[(int)index]);

			wprintf(L"UInt64 result:%ls val=%ld\n",
					result ? L"true" : L"false",
					val);

			result = pArchive->GetBoolProperty(index, bVal);

			wprintf(L"Bool result:%ls val=%ls\n",
					result ? L"true" : L"false",
					bVal ? L"true" : L"false");

			result = pArchive->GetStringProperty(index, strVal);

			wprintf(L"String result:%ls val=%ls\n",
					result ? L"true" : L"false",
					strVal.c_str());

			result = pArchive->GetFileTimeProperty(index, val);

			wprintf(L"FileTime result:%ls val=%ld\n",
					result ? L"true" : L"false",
					val);
		}

		wprintf(L"=============================================\n");

		for(unsigned int i = 0;i < numItems;i++) {
			C7ZipArchiveItem * pArchiveItem = NULL;

			if (pArchive->GetItemInfo(i, &pArchiveItem)) {
				wprintf(L"%d,%ls,%d\n", pArchiveItem->GetArchiveIndex(),
						pArchiveItem->GetFullPath().c_str(),
						pArchiveItem->IsDir());

				wprintf(L"get all properties\n");
				for(lib7zip::PropertyIndexEnum index = lib7zip::kpidPackSize;
					index <= lib7zip::kpidIsDir;
					index = (lib7zip::PropertyIndexEnum)(index + 1)) {
					wstring strVal = L"";
					unsigned __int64 val = 0;
					bool bVal = false;

					bool result = pArchiveItem->GetUInt64Property(index, val);

					wprintf(L"\n\nGetProperty:%d %ls\n", (int)index,
							index_names[(int)index]);

					wprintf(L"UInt64 result:%ls val=%ld\n",
							result ? L"true" : L"false",
							val);

					result = pArchiveItem->GetBoolProperty(index, bVal);

					wprintf(L"Bool result:%ls val=%ls\n",
							result ? L"true" : L"false",
							bVal ? L"true" : L"false");

					result = pArchiveItem->GetStringProperty(index, strVal);

					wprintf(L"String result:%ls val=%ls\n",
							result ? L"true" : L"false",
							strVal.c_str());

					result = pArchiveItem->GetFileTimeProperty(index, val);

					wprintf(L"FileTime result:%ls val=%ld\n",
							result ? L"true" : L"false",
							val);
				}

				//set archive password or item password
				pArchive->SetArchivePassword(L"hello");
			} //if
		}//for
	}
	else {
		wprintf(L"open archive Test7ZipCryptFileName.7z with password fail, LastError=%X\n", lib.GetLastError());
	}

	if (pArchive != NULL)
		delete pArchive;

	return 0;
}
