// Test7Zip.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "lib7zip.h"
#include <iostream>

#include <locale>
#include <iostream>
#include <string>
#include <sstream>

wstring widen( const string& str )
{
	std::wostringstream wstm ;
      wstm.imbue(std::locale("en_US.utf8"));
      const std::ctype<wchar_t>& ctfacet =
		  std::use_facet< std::ctype<wchar_t> >( wstm.getloc() ) ;
      for( size_t i=0 ; i<str.size() ; ++i )
      wstm << ctfacet.widen( str[i] ) ;
      return wstm.str() ;
}

string narrow( const wstring& str )
{
	std::ostringstream stm ;
      stm.imbue(std::locale("C"));
      const std::ctype<char>& ctfacet =
		  std::use_facet< std::ctype<char> >( stm.getloc() ) ;
      for( size_t i=0 ; i<str.size() ; ++i )
      stm << ctfacet.narrow( str[i], 0 ) ;
      return stm.str() ;
}

class TestMultiVolumes : public C7ZipMultiVolumes
{
private:
	FILE * m_pFile;
	wstring m_strFileName;
	int m_nFileSize;
	wstring m_strCurVolume;
	bool m_done;

public:
	TestMultiVolumes(wstring fileName) :
	  m_pFile(NULL),
	  m_strFileName(fileName),
	  m_done(false)
	{
	}

	virtual ~TestMultiVolumes()
	{
		if (m_pFile)
			fclose(m_pFile);
	}

public:
	virtual wstring GetFirstVolumeName() {
		m_strCurVolume = m_strFileName;
		MoveToVolume(m_strCurVolume);
		return m_strCurVolume;
	}

	virtual bool MoveToVolume(const wstring& volumeName) {
		m_strCurVolume = volumeName;
		wprintf(L"move to volume:%ls\n", volumeName.c_str());

		if (m_pFile)
			fclose(m_pFile);
		m_pFile = NULL;
		string f = narrow(volumeName);
		wprintf(L"narrow volume:%s\n", f.c_str());

		m_pFile = fopen(f.c_str(), "rb");

		if (!m_pFile)
			m_done = true;
		else {
		fseek(m_pFile, 0, SEEK_END);
		m_nFileSize = ftell(m_pFile);
		fseek(m_pFile, 0, SEEK_SET);
		}

		return !m_done;
	}

	virtual C7ZipInStream * OpenCurrentVolumeStream() {
		return new CDiskInStream(m_strCurVolume);
	}

	virtual unsigned __int64 GetCurrentVolumeSize() {
		wprintf(L"get current volume size:%ls\n", m_strCurVolume.c_str());
		return m_nFileSize;
	}
};

#ifdef _WIN32
int _tmain(int argc, _TCHAR* argv[])
#else
int main(int argc, char * argv[])
#endif
{
	C7ZipLibrary lib;

	if (!lib.Initialize())
	{
		wprintf(L"initialize fail!\n");
		return 1;
	}

	WStringArray exts;

	if (!lib.GetSupportedExts(exts))
	{
		wprintf(L"get supported exts fail\n");
		return 1;
	}

	size_t size = exts.size();

	for(size_t i = 0; i < size; i++)
	{
		wstring ext = exts[i];

		for(size_t j = 0; j < ext.size(); j++)
		{
			wprintf(L"%c", (char)(ext[j] &0xFF));
		}

		wprintf(L"\n");
	}

	C7ZipArchive * pArchive = NULL;

	TestMultiVolumes volumes(L"./compress/test.7z.001");
	CDiskOutStream oStream(L"./out/TestMultiResult.txt");
	if (lib.OpenMultiVolumeArchive(&volumes, &pArchive))
	{
		unsigned int numItems = 0;

		pArchive->GetItemCount(&numItems);

		wprintf(L"%d\n", numItems);

		for(unsigned int i = 0;i < numItems;i++)
		{
			C7ZipArchiveItem * pArchiveItem = NULL;

			if (pArchive->GetItemInfo(i, &pArchiveItem))
			{
				wprintf(L"%d,%ls,%d\n", pArchiveItem->GetArchiveIndex(),
					pArchiveItem->GetFullPath().c_str(),
					pArchiveItem->IsDir());
			}
				//set archive password or item password
				pArchive->SetArchivePassword(L"test");
				if (i==0) {
					//Or set password for each archive item
					//pArchiveItem->SetArchiveItemPassword(L"test");
					pArchive->Extract(pArchiveItem, &oStream);
				}
		}
	}
	else
	{
		wprintf(L"open archive test.7z.001 fail\n");
	}

	if (pArchive != NULL)
		delete pArchive;

	return 0;
}
