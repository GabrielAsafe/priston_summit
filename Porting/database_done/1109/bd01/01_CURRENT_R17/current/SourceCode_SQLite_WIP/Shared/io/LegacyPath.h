#pragma once

#include <cstddef>
#include <cstring>

namespace summit
{
namespace io
{

// Returns the final path component using the separators accepted by the
// legacy ASE loader (/, \ and drive/path colon). The returned pointer
// aliases path; no allocation or normalization is performed.
inline const char *PathLeaf(const char *path)
{
	if(!path)
		return nullptr;

	const char *leaf = path;
	for(const char *cursor = path; *cursor; ++cursor)
	{
		if(*cursor == '/' || *cursor == '\\' || *cursor == ':')
			leaf = cursor + 1;
	}
	return leaf;
}

// Concatenates two already-formatted legacy path fragments. It intentionally
// does not add or normalize separators; callers retain the historical format.
inline bool ConcatenatePathParts(
	const char *prefix,
	const char *suffix,
	char *output,
	std::size_t outputSize)
{
	if(!prefix || !suffix || !output || outputSize == 0)
		return false;

	const std::size_t prefixLength = std::strlen(prefix);
	const std::size_t suffixLength = std::strlen(suffix);
	if(prefixLength + suffixLength + 1 > outputSize)
	{
		output[0] = 0;
		return false;
	}

	std::memcpy(output, prefix, prefixLength);
	std::memcpy(output + prefixLength, suffix, suffixLength + 1);
	return true;
}

// Builds a path using the directory that contains sourceFile and a legacy
// relative file name. No normalization is performed so old data-file path
// semantics remain unchanged; the helper only makes the concatenation bounded.
inline bool BuildPathFromFileDirectory(
	const char *sourceFile,
	const char *relativePath,
	char *output,
	std::size_t outputSize)
{
	if(!sourceFile || !relativePath || !output || outputSize == 0)
		return false;

	const char *slash = std::strrchr(sourceFile, '/');
	const char *backslash = std::strrchr(sourceFile, '\\');
	const char *separator = nullptr;

	if(slash && backslash)
		separator = (slash > backslash) ? slash : backslash;
	else
		separator = slash ? slash : backslash;

	const std::size_t directoryLength = separator
		? static_cast<std::size_t>(separator - sourceFile) + 1
		: 0;
	const std::size_t relativeLength = std::strlen(relativePath);

	if(directoryLength + relativeLength + 1 > outputSize)
	{
		output[0] = 0;
		return false;
	}

	if(directoryLength > 0)
		std::memcpy(output, sourceFile, directoryLength);

	std::memcpy(output + directoryLength, relativePath, relativeLength + 1);
	return true;
}


// Copies only the directory portion of sourceFile, including the trailing
// separator when one exists. The output is always terminated when possible.
inline bool CopyFileDirectory(
	const char *sourceFile,
	char *output,
	std::size_t outputSize)
{
	if(!sourceFile || !output || outputSize == 0)
		return false;

	const char *slash = std::strrchr(sourceFile, '/');
	const char *backslash = std::strrchr(sourceFile, '\\');
	const char *separator = nullptr;

	if(slash && backslash)
		separator = (slash > backslash) ? slash : backslash;
	else
		separator = slash ? slash : backslash;

	const std::size_t directoryLength = separator
		? static_cast<std::size_t>(separator - sourceFile) + 1
		: 0;

	if(directoryLength + 1 > outputSize)
	{
		output[0] = 0;
		return false;
	}

	if(directoryLength > 0)
		std::memcpy(output, sourceFile, directoryLength);
	output[directoryLength] = 0;
	return true;
}

// Replaces the last extension marker using the same legacy semantics as the
// original helper (the last '.' anywhere in the string wins). If no marker is
// present, a new one is appended. No path normalization is performed.
inline bool ReplaceFileExtension(
	const char *sourceFile,
	const char *extension,
	char *output,
	std::size_t outputSize)
{
	if(!sourceFile || !extension || !output || outputSize == 0)
		return false;

	const std::size_t sourceLength = std::strlen(sourceFile);
	const std::size_t extensionLength = std::strlen(extension);
	const char *dot = std::strrchr(sourceFile, '.');
	const std::size_t prefixLength = dot
		? static_cast<std::size_t>(dot - sourceFile) + 1
		: sourceLength + 1;

	if(prefixLength + extensionLength + 1 > outputSize)
	{
		output[0] = 0;
		return false;
	}

	if(dot)
	{
		std::memcpy(output, sourceFile, prefixLength);
	}
	else
	{
		std::memcpy(output, sourceFile, sourceLength);
		output[sourceLength] = '.';
	}

	std::memcpy(output + prefixLength, extension, extensionLength + 1);
	return true;
}

} // namespace io
} // namespace summit
