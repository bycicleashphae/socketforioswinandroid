#include "assert.h"
#include "string.h"
#include "stdio.h"


#include "NetMessage.h"


#include <list>
#ifndef WIN32
#include "arpa/inet.h"
#endif
class MemBuffer;
class CMessageBufPool
{
public:
	CMessageBufPool();

	~CMessageBufPool();

	static CMessageBufPool*     getInstance();

	void Init();

	MemBuffer* require(int size = 0);

	void release(MemBuffer* pBuf);

private:
	static bool m_hasInit;
	std::list<MemBuffer*> m_pool;
};
class MemBuffer
{
public:
	MemBuffer(int size);

	~MemBuffer();

	void enlarge(int newSize = 0);

	int getSize()   const
	{
		return m_size;
	}

	char*   getBuf(int pos);

private:
	int     m_size;
	char*   m_buf;
};


MemBuffer::MemBuffer(int size)
{
	m_size = size;
	m_buf = (char*)malloc(getSize());
	if (m_buf == NULL)
	{
		printf("create buffer failed \n");
		m_size = 0;
	}
}
void NetMessage::createBuf(int size)
{
	m_buffer = CMessageBufPool::getInstance()->require(size);
}

int NetMessage::getReadCap()
{
	if(m_size >= 0)
	{
		return m_size - m_curPos + m_headSize;
	}
	else if(m_size == -1)
	{
		return 2;
	}
	else //if(m_size == -2)
	{
		return 4;
	}
}
MemBuffer::~MemBuffer()
{
	free(m_buf);
	m_size = 0;
	m_buf = NULL;
}

void MemBuffer::enlarge(int newSize)
{
	if (newSize == 0)
	{
		m_size *= 2;
	}
	void *pNewBuf = realloc((void*)m_buf, getSize());
	if (pNewBuf == NULL)
	{
		printf("realloc buffer failed \n");
		m_size = 0;
		free(m_buf);
	}
	m_buf = (char*)pNewBuf;
}

char* MemBuffer::getBuf(int pos)
{
	if (pos >= getSize())
	{
		printf("pos: %d  %d \n", pos, getSize());
	}
	return m_buf + pos;
}

//-----------------------------------------------
bool CMessageBufPool::m_hasInit = false;

CMessageBufPool::CMessageBufPool()
{
	m_hasInit = true;
}

CMessageBufPool::~CMessageBufPool()
{
	std::list<MemBuffer*>::iterator iter = m_pool.begin();
	for (; iter != m_pool.end(); ++iter)
	{
		delete *iter;
	}
	m_hasInit = false;
}

CMessageBufPool* CMessageBufPool::getInstance()
{
	static CMessageBufPool instance;
	return &instance;
}

void CMessageBufPool::Init()
{
	for (int i = 0; i<5; ++i)
	{
		MemBuffer *pBuf = new MemBuffer(512);
		m_pool.push_back(pBuf);
	}
	for (int i = 0; i<5; ++i)
	{
		MemBuffer *pBuf = new MemBuffer(1024);
		m_pool.push_back(pBuf);
	}
	MemBuffer *pBuf = new MemBuffer(65536);
	m_pool.push_back(pBuf);
}

MemBuffer* CMessageBufPool::require(int size)
{
	if (m_pool.empty())
	{
		Init();
	}

	if (size == 0)
	{
		MemBuffer* pOut = m_pool.front();
		m_pool.pop_front();
		return pOut;
	}
	else
	{
		std::list<MemBuffer*>::iterator iter = m_pool.begin();
		for (; iter != m_pool.end(); ++iter)
		{
			if ((*iter)->getSize() >= size)
			{
				MemBuffer* pOut = *iter;
				m_pool.erase(iter);
				return pOut;
			}
		}

		MemBuffer* pOut = new MemBuffer(size);
		return pOut;
	}
}

void CMessageBufPool::release(MemBuffer* pBuf)
{
	if (m_hasInit == false)
		return;

	std::list<MemBuffer*>::iterator iter = m_pool.begin();
	for (; m_pool.size() != 0 && iter != m_pool.end(); ++iter)
	{
		if ((*iter)->getSize() >= pBuf->getSize())
		{
			m_pool.insert(iter, pBuf);
			return;
		}
	}
	m_pool.push_back(pBuf);
}


NetMessage::NetMessage(int size)
{   
	m_size = -1;
	m_headSize = 2;
	m_curPos = 0;
	m_bufStartPos = 0;
	m_buffer = NULL;
    createBuf(size);
}

char* NetMessage::getBuf(int pos)
{
	return m_buffer->getBuf(pos);
}

NetMessage::~NetMessage()
{
	m_curPos = 0;
	m_size = -1;
	m_headSize = 2;
	if(m_buffer != NULL)
    {
		CMessageBufPool::getInstance()->release(m_buffer);
    }
    m_buffer = NULL;
}

void NetMessage::onReceive(int size)
{
	m_curPos += size;
	if(m_size >= 0)
	{
	}
	else if(m_size == -1 && m_curPos == m_headSize)
	{
		const char* buf = m_buffer->getBuf(0);
		unsigned short val = 0;
		val |= (buf[0] & 0xFF) << 8;
		val |= (buf[1] & 0xFF);
		if(val == 0xFFFF)
		{
			m_curPos = 0;
			m_size = -2; //need recv another 4 byte
			m_headSize = 4;
		}
		else
		{
			m_size = val;
			m_headSize = 2;
			m_curPos = m_headSize;
			m_bufStartPos = 0;
			checkBufSize(m_size);
		}
	}
	else if(m_size == -2 && m_curPos == m_headSize)
	{
		const char* buf = m_buffer->getBuf(0);
		unsigned int val = 0;
		val |= (buf[0] & 0xFF) << 24;
		val |= (buf[1] & 0xFF) << 16;
		val |= (buf[2] & 0xFF) << 8;
		val |= (buf[3] & 0xFF);
		m_size = val;
		m_curPos = m_headSize;
		m_bufStartPos = 0;
		checkBufSize(m_size);
	}
}

bool NetMessage::isEnd() const
{
	if(m_size < 0)
	{
		return false;
	}
	return m_curPos == m_size + m_headSize + m_bufStartPos;
}

void NetMessage::onSend(int size)
{
	m_curPos += size;
}

int NetMessage::getSendCap()
{
	return m_size - m_curPos + m_bufStartPos + m_headSize;
}


void NetMessage::beginForWrite()
{
	m_curPos = 6; // reserved sizeof(int16) + sizeof(int32)
}

void NetMessage::endForWrite()
{
	int currentPos = m_curPos;
	int structLen = m_curPos - 6; // reserved sizeof(int16) + sizeof(int32)
	m_size = structLen;
	if(m_size >= 0xFFFF)
	{
		m_headSize = 6;
		m_curPos = 0;
		wUShort(0xFFFF);
		wUInt(m_size);
		m_curPos = 0;
		m_bufStartPos = m_curPos;
	}
	else
	{
		m_headSize = 2;
		m_curPos = 4; // skip first 4 bytes
		wUShort(m_size);
		m_curPos = 4;
		m_bufStartPos = m_curPos;
	}
}


void NetMessage::checkBufSize(int size)
{
	while (m_curPos + size > m_buffer->getSize())
    {
        m_buffer->enlarge();
    }
}
void NetMessage::setBufValue(char value)
{
	*(m_buffer->getBuf(m_curPos++)) = value;
}
char NetMessage::getBufValue()
{
	return *(m_buffer->getBuf(m_curPos++));
}

void NetMessage::wInt64(Int64 v)
{
	checkBufSize(sizeof(v));
	int p1 = m_curPos++;
	int p2 = m_curPos++;
	int p3 = m_curPos++;
	int p4 = m_curPos++;
	int p5 = m_curPos++;
	int p6 = m_curPos++;
	int p7 = m_curPos++;
	int p8 = m_curPos++;
	*(m_buffer->getBuf(p8)) = (v)& 0xFF; v = v >> 8;
	*(m_buffer->getBuf(p7)) = (v)& 0xFF; v = v >> 8;
	*(m_buffer->getBuf(p6)) = (v)& 0xFF; v = v >> 8;
	*(m_buffer->getBuf(p5)) = (v)& 0xFF; v = v >> 8;
	*(m_buffer->getBuf(p4)) = (v)& 0xFF; v = v >> 8;
	*(m_buffer->getBuf(p3)) = (v)& 0xFF; v = v >> 8;
	*(m_buffer->getBuf(p2)) = (v)& 0xFF; v = v >> 8;
	*(m_buffer->getBuf(p1)) = (v)& 0xFF;

}


Int64 NetMessage::rInt64()
{

	Int64 val = 0;
	val = (getBufValue() & 0xFF);
	val = (val << 8) + (getBufValue() & 0xFF);
	val = (val << 8) + (getBufValue() & 0xFF);
	val = (val << 8) + (getBufValue() & 0xFF);
	val = (val << 8) + (getBufValue() & 0xFF);
	val = (val << 8) + (getBufValue() & 0xFF);
	val = (val << 8) + (getBufValue() & 0xFF);
	val = (val << 8) + (getBufValue() & 0xFF);
	return val;
}

void NetMessage::wUShort(unsigned short v)
{
	checkBufSize(sizeof(v));
	setBufValue((v >> 8) & 0xFF);
	setBufValue((v)& 0xFF);

}

unsigned short NetMessage::rUShort()
{

    unsigned short val = 0;
	val = (getBufValue()& 0xFF) << 8;
	val |= (getBufValue() & 0xFF);
    return val;
}

void NetMessage::wShort(short v)
{
	checkBufSize(sizeof(v));

	setBufValue((v >> 8) & 0xFF);
	setBufValue((v)& 0xFF);

}
short NetMessage::rShort()
{

	short val = 0;
	val = (getBufValue() & 0xFF) << 8;
	val |= (getBufValue() & 0xFF);
	return val;
}
void NetMessage::wInt8(Int8 v)
{
	checkBufSize(sizeof(v));
	setBufValue((v) & 0xFF);

}
Int8 NetMessage::rInt8()
{
	Int8 val = 0;
	val = (getBufValue()& 0xFF);
	return val;
}
void NetMessage::wUInt8(UInt8 v)
{
	checkBufSize(sizeof(v));

	setBufValue((v) & 0xFF);

}

UInt8 NetMessage::rUInt8()
{

	UInt8 val = 0;
	val = (getBufValue()& 0xFF);
	return val;
}

bool NetMessage::rBoolean()
{
	char val = getBufValue() & 0xFF;
	return val != '\0';
}

void NetMessage::wBoolean(bool v)
{
	checkBufSize(sizeof(char));
    setBufValue(v ? '\1' : '\0');
}





void NetMessage::wInt(int v)
{
	checkBufSize(sizeof(v));

	setBufValue((v >> 24) & 0xFF);
	setBufValue((v >> 16) & 0xFF);
	setBufValue((v >> 8) & 0xFF);
	setBufValue((v)& 0xFF);

}
int NetMessage::rInt()
{

	int val = 0;
	val = (getBufValue() & 0xFF) << 24;
	val |= (getBufValue() & 0xFF) << 16;
	val |= (getBufValue() & 0xFF) << 8;
	val |= (getBufValue() & 0xFF);
	return val;
}

void NetMessage::wUInt(unsigned int v)
{
	checkBufSize(sizeof(v));

	setBufValue((v >> 24) & 0xFF);
	setBufValue((v >> 16) & 0xFF);
	setBufValue((v >> 8) & 0xFF);
	setBufValue((v) & 0xFF);

}

unsigned int NetMessage::rUInt()
{

	unsigned int val = 0;
	val = (getBufValue() & 0xFF) << 24;
	val |= (getBufValue() & 0xFF) << 16;
	val |= (getBufValue() & 0xFF) << 8;
	val |= (getBufValue() & 0xFF);
	return val;
}
void NetMessage::wFloat(float v)
{
	checkBufSize(sizeof(v));

    int& ival = *((int*)&v);
	setBufValue((ival >> 24) & 0xFF);
	setBufValue((ival >> 16) & 0xFF);
	setBufValue((ival >> 8) & 0xFF);
	setBufValue((ival) & 0xFF);
    

}

float NetMessage::rFloat()
{

	float val = 0;
	int& ival = *((int*)&val);
	ival = (getBufValue() & 0xFF) << 24;
	ival |= (getBufValue() & 0xFF) << 16;
	ival |= (getBufValue() & 0xFF) << 8;
	ival |= (getBufValue() & 0xFF);
	return val;
}
void NetMessage::wDouble(double v)
{
	checkBufSize(sizeof(v));

    char *buf = (char *)&v;
	setBufValue(buf[7]);
	setBufValue(buf[6]);
	setBufValue(buf[5]);
	setBufValue(buf[4]);
	setBufValue(buf[3]);
	setBufValue(buf[2]);
	setBufValue(buf[1]);
	setBufValue(buf[0]);

}

double NetMessage::rDouble()
{

	double val = 0;
	char buf[8];
	buf[7] = getBufValue();
	buf[6] = getBufValue();
	buf[5] = getBufValue();
	buf[4] = getBufValue();
	buf[3] = getBufValue();
	buf[2] = getBufValue();
	buf[1] = getBufValue();
	buf[0] = getBufValue();

	val = *((double *)buf);

	return val;
}
void NetMessage::wString(const char* pStr, int size)
{
	checkBufSize(sizeof(int) + size);
    
	wInt((int)size);
	memcpy(getCurrentBuf(), pStr, size);
	m_curPos += size;
}

const char* NetMessage::rString(int& size)
{
	size = rInt();
	if (size <= 0)
        return "";
    
    char* pOut = getCurrentBuf();
	m_curPos += size;
    return pOut;
}

void NetMessage::wRaw(const char* buf, int sz)
{
	checkBufSize(sz);
	memcpy(getCurrentBuf(), buf, sz);
	m_curPos += sz;
}

const char* NetMessage::rRaw(int sz)
{
	if(sz <= 0)
	{
		return "";
	}
	char* pOut = getCurrentBuf();
	m_curPos += sz;
	return pOut;
}

char* NetMessage::getReadBuf()
{
	return getBuf(m_curPos);
}

void NetMessage::dump()
{
    printf("NetMessage content is:\n");
    
	for (int i = 0; i < m_size + m_headSize; ++i)
    {
        printf("%x ", *getBuf(i));
    }
}




