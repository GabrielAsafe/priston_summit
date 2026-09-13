#pragma once

#include <cstddef>
#include <cstdio>
#include <limits>

namespace summit
{
namespace io
{

// Small RAII wrapper used while legacy parsers are migrated away from raw FILE*.
// It intentionally exposes only the operations required by the current loaders,
// so file format and parsing behaviour stay unchanged during the refactor.
class LegacyFile
{
public:
	LegacyFile()
		: handle_(nullptr)
	{
	}

	LegacyFile(const char *path, const char *mode)
		: handle_(nullptr)
	{
		Open(path, mode);
	}

	~LegacyFile()
	{
		Close();
	}

	LegacyFile(const LegacyFile &) = delete;
	LegacyFile &operator=(const LegacyFile &) = delete;

	explicit operator bool() const
	{
		return handle_ != nullptr;
	}

	bool Open(const char *path, const char *mode)
	{
		Close();

		if(!path || !mode)
			return false;

		handle_ = std::fopen(path, mode);
		return handle_ != nullptr;
	}

	void Close()
	{
		if(handle_)
		{
			std::fclose(handle_);
			handle_ = nullptr;
		}
	}

	char *ReadLine(char *buffer, std::size_t bufferSize)
	{
		if(!handle_ || !buffer || bufferSize == 0 ||
		   bufferSize > static_cast<std::size_t>(std::numeric_limits<int>::max()))
			return nullptr;

		return std::fgets(buffer, static_cast<int>(bufferSize), handle_);
	}

	std::size_t Read(void *buffer, std::size_t elementSize, std::size_t elementCount)
	{
		if(!handle_ || !buffer || elementSize == 0 || elementCount == 0)
			return 0;

		return std::fread(buffer, elementSize, elementCount, handle_);
	}

	std::size_t Write(const void *buffer, std::size_t elementSize, std::size_t elementCount)
	{
		if(!handle_ || !buffer || elementSize == 0 || elementCount == 0)
			return 0;

		return std::fwrite(buffer, elementSize, elementCount, handle_);
	}

	FILE *Get() const
	{
		return handle_;
	}

private:
	FILE *handle_;
};

} // namespace io
} // namespace summit
