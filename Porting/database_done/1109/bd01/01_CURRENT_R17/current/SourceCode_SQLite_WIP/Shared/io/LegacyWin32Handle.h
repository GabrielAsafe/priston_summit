#pragma once

namespace summit
{
namespace io
{
namespace legacy_win32
{

class ScopedHandle
{
public:
	explicit ScopedHandle(HANDLE handle = INVALID_HANDLE_VALUE) : handle_(handle) {}
	~ScopedHandle() { Close(); }

	HANDLE Get() const { return handle_; }
	bool IsValid() const { return handle_ != INVALID_HANDLE_VALUE; }

	void Reset(HANDLE handle = INVALID_HANDLE_VALUE)
	{
		Close();
		handle_ = handle;
	}

	void Close()
	{
		if(IsValid())
			::CloseHandle(handle_);
		handle_ = INVALID_HANDLE_VALUE;
	}

private:
	ScopedHandle(const ScopedHandle &);
	ScopedHandle &operator=(const ScopedHandle &);
	HANDLE handle_;
};

} // namespace legacy_win32
} // namespace io
} // namespace summit
