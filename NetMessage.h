#ifndef _NETMESSAGE_LKAJSLDFJASLDFJKLASDF_H__
#define _NETMESSAGE_LKAJSLDFJASLDFJKLASDF_H__
#include "string.h"
#include "assert.h"
#include "vector"
#include "stdio.h"


typedef unsigned char	UInt8;
typedef signed char	Int8;
#ifndef WIN32
typedef long long Int64;
#else
typedef __int64 Int64;
#endif

class MemBuffer;

class NetMessage
{
	void    checkBufSize(int size);

	// -1 : unknown 
	// -2 : need 4 byte HeadSize
	// >=0 : size known
    int m_size;
	int m_headSize; //2 or 4
	int m_bufStartPos;

public:
	NetMessage(int size = 512);
	~NetMessage();
	int     getSize() const          { return m_size; }
	int     getHeadSize() const      { return m_headSize; }
    void    createBuf(int bufSize = 0);
    char*   getBuf(int pos);
	char*   getReadBuf();
    int     getReadCap();
    int     getSendCap();
    void    onReceive(int size);
    void    onSend(int size);
	int     getCurrentPos() const       { return m_curPos; }
	char*   getCurrentBuf()          { return getBuf(m_curPos); }
    
	bool    isEnd() const;
	void    ResetForSend()					{ m_curPos = m_bufStartPos; }
	void    ResetForRead()					{ m_curPos = m_bufStartPos + m_headSize; }
    void wFloat(float v);
	float rFloat();
    void wShort(short v);
	short rShort();
	void wUShort(unsigned short v);
	unsigned short rUShort();
	void wInt(int v);
	int rInt();
	void wUInt(unsigned int v);
	unsigned int rUInt();
	void wInt8(Int8 v);
	Int8 rInt8();
	void wUInt8(UInt8 v);
	UInt8 rUInt8();
	void wInt64(Int64 v);    
	Int64 rInt64();

	void wDouble(double v);
	double rDouble();
	void wBoolean(bool v);
	bool rBoolean();
	
	void wChar(char v);
	char rChar();
	void wString(const char* str, int sz);
	const char* rString(int& sz);

	void wRaw(const char* buf, int sz);
	const char* rRaw(int sz);

	void dump();
	void beginForWrite();
	void endForWrite();
private:
  
    int     m_curPos;
    MemBuffer* m_buffer;
	void setBufValue(char value);
	char getBufValue();
};

#endif