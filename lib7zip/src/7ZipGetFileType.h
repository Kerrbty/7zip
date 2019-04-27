#ifndef __7ZIP_GET_FILE_TYPE_H__
#define __7ZIP_GET_FILE_TYPE_H__
#include "lib7zip.h"

// 将一个文件留输入，输出文件类型 
bool C7ZipGetFileType(C7ZipInStream *InStream, wstring &Ext);

#endif // __7ZIP_GET_FILE_TYPE_H__