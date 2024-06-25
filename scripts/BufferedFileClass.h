#ifndef TT_INCLUDE_BUFFEREDFILECLASS_H
#define TT_INCLUDE_BUFFEREDFILECLASS_H

#include "RawFileClass.h"
#include <memory>

class BufferedFileClass : public RawFileClass
{
	static constexpr int DefaultBufferSize = 65536;
	std::unique_ptr<unsigned char[]> Buffer;
	int BufferSize = DefaultBufferSize;
	int BufferAvailable = 0;
	int BufferPos = 0;
	int BufferStart = 0;      // buffer start offset within the file
	int BufferEnd = 0;        // buffer end offset within the file
	int BufferDirtyStart = 0; // dirty data in the buffer (offset within buffer)
	int BufferDirtyEnd = 0;
	int FileOffset = 0;       // current position in the file

public:

	BufferedFileClass(const char* filename, int buffer_size = DefaultBufferSize) : RawFileClass(filename), BufferSize(buffer_size)
	{
	}
	BufferedFileClass(unsigned int buffer_size = DefaultBufferSize) : BufferSize(buffer_size)
	{
	}

	~BufferedFileClass()
	{
		Reset();
	}

private:

	int TryReadFromBuffer(void*& dest, int& count)
	{
		TT_ASSERT(BufferAvailable >= 0);
		int read_count = min(count, BufferAvailable);
		if (read_count > 0)
		{
			memcpy(dest, Buffer.get() + BufferPos, read_count);
			BufferAvailable -= read_count;
			BufferPos += read_count;
			count -= read_count;
			dest = (char*)dest + read_count;
			FileOffset += read_count;
		}
		return read_count;
	}

	int TryWriteToBuffer(const void*& src, int& count)
	{
		TT_ASSERT(BufferAvailable >= 0);
		int write_count = min(count, BufferAvailable);
		if (write_count > 0)
		{
			if (BufferDirtyEnd == BufferDirtyStart) {
				BufferDirtyStart = BufferPos;
				BufferDirtyEnd = BufferDirtyStart;
			}
			memcpy(Buffer.get() + BufferPos, src, write_count);
			BufferAvailable -= write_count;
			BufferPos += write_count;
			count -= write_count;
			src = (char*)src + write_count;
			BufferDirtyEnd = max(BufferDirtyEnd, BufferPos);
			FileOffset += write_count;
		}
		return write_count;
	}

	void Flush_Write_Buffer()
	{
		if (BufferDirtyEnd > BufferDirtyStart)
		{
			RawFileClass::Seek(BufferStart + BufferDirtyStart, ORIGIN_START);
			RawFileClass::Write(Buffer.get() + BufferDirtyStart, BufferDirtyEnd - BufferDirtyStart);
			BufferDirtyStart = BufferPos;
			BufferDirtyEnd = BufferDirtyStart;
		}
	}

	void Reset_Buffer()
	{
		BufferAvailable = 0;
		BufferPos = 0;
		BufferDirtyStart = 0;
		BufferDirtyEnd = 0;
	}

	void Reset_File()
	{
		BufferAvailable = 0;
		BufferPos = 0;
		BufferStart = 0;
		BufferEnd = 0;
		BufferDirtyStart = 0;
		BufferDirtyEnd = 0;
		FileOffset = 0;
	}

public:

	int Open(const char* name, int mode = 1) final
	{
		int result = RawFileClass::Open(name, mode);
		if (result && !Buffer) Buffer = std::unique_ptr<unsigned char[]>(new unsigned char[BufferSize]); // raw new to avoid memset, we are okay with uninitialized memory
		return result;
	}
	int Open(int mode = 1) final
	{
		int result = RawFileClass::Open(mode);
		if (result && !Buffer) Buffer = std::unique_ptr<unsigned char[]>(new unsigned char[BufferSize]); // raw new to avoid memset, we are okay with uninitialized memory
		return result;
	}

	int Read(void* dest, int count) final
	{
		TT_ASSERT(count >= 0);

		int result = 0;

		// Try to read the entire request from the buffer
		result += TryReadFromBuffer(dest, count);

		if (count) // we still have some data left to read
		{
			TT_ASSERT(BufferAvailable == 0);

			if (count > BufferSize) // request too big, don't buffer
			{
				int read_count = RawFileClass::Read(dest, count);
				FileOffset += read_count;
				result += read_count;
				return result;
			}

			if (BufferAvailable == 0) // fill buffer
			{
				BufferAvailable = RawFileClass::Read(Buffer.get(), BufferSize);
				BufferPos = 0;
				BufferStart = Tell();
				BufferEnd = BufferStart + BufferAvailable;
			}

			result += TryReadFromBuffer(dest, count);
		}
		return result;
	}

	int Seek(int offset, int origin) final
	{
		if (origin == ORIGIN_CURRENT && offset == 0)
			return Tell();

		int target_offset = 0;
		switch (origin) {
		case ORIGIN_START:   target_offset = offset; break;
		case ORIGIN_CURRENT: target_offset = FileOffset + offset; break;
		case ORIGIN_END:     target_offset = RawFileClass::Size() + offset; break; // TODO: not sure querying the size is a good idea, seeking directly might be a smaller perf hit if unbiased?
		}

		FileOffset = target_offset;

		if (BufferEnd > BufferStart) // if we have anything in the buffer at all
		{
			if (target_offset >= BufferStart && target_offset < BufferEnd) // the offset is within our buffer
			{
				BufferAvailable = BufferEnd - target_offset;
				BufferPos = target_offset - BufferStart;
				if (BufferDirtyEnd > BufferDirtyStart && (BufferPos < BufferDirtyStart || BufferPos > BufferDirtyEnd)) // offset is not within dirty part
				{
					Flush_Write_Buffer();
				}
				return target_offset;
			}
		}
		
		Flush_Write_Buffer();
		Reset_Buffer();

		if (Get_Mode() == OPEN_WRITE)
			return FileOffset;
		else
			return RawFileClass::Seek(FileOffset, ORIGIN_START);
	}

	int Tell() final
	{
		return FileOffset;
	}

	int Write(const void* buffer, int size) final
	{
		TT_ASSERT(size >= 0);

		if (Get_Mode() == OPEN_READ_WRITE) // don't even bother trying to make this work
		{
			int result = RawFileClass::Write(buffer, size);
			FileOffset += result;
			return result;
		}

		int result = 0;

		// Try to write the entire request to the buffer
		result += TryWriteToBuffer(buffer, size);
		
		if (size) // we still have some data left to write
		{
			TT_ASSERT(BufferAvailable == 0);

			Flush_Write_Buffer();

			if (size > BufferSize) // request too big, don't buffer
			{
				RawFileClass::Seek(FileOffset, ORIGIN_START);
				Reset_Buffer();
				int write_count = RawFileClass::Write(buffer, size);
				FileOffset += write_count;
				result += write_count;
				return result;
			}

			if (BufferAvailable == 0) // create buffer
			{
				BufferAvailable = BufferSize;
				BufferPos = 0;
				BufferStart = FileOffset;
				BufferEnd = BufferStart + BufferAvailable;
				BufferDirtyStart = 0;
				BufferDirtyEnd = BufferDirtyStart;
			}

			result += TryWriteToBuffer(buffer, size);
		}
		return result;
	}

	void Close() final
	{
		Flush_Write_Buffer();
		RawFileClass::Close();
		Reset_Buffer();
		Reset_File();
	}
};

#endif