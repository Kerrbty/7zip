#ifndef __7ZIP_GET_FILE_TYPE_H__
#define __7ZIP_GET_FILE_TYPE_H__
#include "lib7zip.h"

// ��һ���ļ������룬����ļ����� 
bool C7ZipGetFileType(C7ZipInStream *InStream, wstring &Ext);

#endif // __7ZIP_GET_FILE_TYPE_H__