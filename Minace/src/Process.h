#pragma once

#include <string>
#include <streambuf>
#include <vector>
#include <memory>
#include <istream>
#include <ios>
#include <exception>
#include <system_error>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifdef __unix__
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

namespace cm {

/*
 * Reads at least 1 byte. Returns the number bytes read, 0 on EOF and -1 on error.
 */
inline ssize_t readSome(int fd, char* buffer, size_t length)
{
	for (;;) {
		ssize_t r = read(fd, buffer, length);
		if (r == -1 && errno == EINTR)
			continue;
		return r;
	}
}

/*
 * Writes all the data in buffer. Returns length on success and -1 on error.
 */
inline ssize_t writeAll(int fd, const char* data, size_t length)
{
	ssize_t writtenBytes = 0;

	while ((size_t) writtenBytes < length) {
		ssize_t r = write(fd, data, length); // Cannot return 0. If length=0 results unspecified.
		if (r == -1) {
			if (errno == EINTR)
				continue;
			return r == 0 ? writtenBytes : r;
		}

		data += r;
		writtenBytes += r;
	}

	return writtenBytes;
}

/*
 * Creates a process using executable file at path. Opens pipes to child process stdin/stdout and
 * returns file descriptors for them in fdIn/fdOut. Takes a null terminated list of arguments in
 * argv.
 */
inline pid_t popen2(const char *path, int *fdIn, int *fdOut, const char *const argv[])
{
	static const int READ = 0, WRITE = 1;

	// Open executable file.
	int fdFile = open(path, O_RDONLY);
	if (fdFile < 0)
		return -1;

	// Create pipes for stdin/stdout.
	int p_stdin[2], p_stdout[2];
	if (fdIn) {
		if (pipe(p_stdin) != 0) {
			close(fdFile);
			return -1;
		}
	}
	if (fdOut) {
		if (pipe(p_stdout) != 0) {
			if (fdIn) {
				close(p_stdin[READ]);
				close(p_stdin[WRITE]);
			}
			close(fdFile);
			return -1;
		}
	}

	pid_t pid = fork();

	// Error
	if (pid < 0) {
		if (fdOut) {
			close(p_stdout[READ]);
			close(p_stdout[WRITE]);
		}
		if (fdIn) {
			close(p_stdin[READ]);
			close(p_stdin[WRITE]);
		}
		close(fdFile);
		return pid;
	}

	// Child process
	if (pid == 0) {
		if (fdIn) {
			close(p_stdin[WRITE]);
			dup2(p_stdin[READ], STDIN_FILENO);
			close(p_stdin[READ]);
		}

		if (fdOut) {
			close(p_stdout[READ]);
			dup2(p_stdout[WRITE], STDOUT_FILENO);
			close(p_stdout[WRITE]);
		}

		fexecve(fdFile, (char *const*) argv, environ);
		perror("execv");
		exit(1);
	}

	if (fdOut) {
		close(p_stdout[WRITE]);
		*fdOut = p_stdout[READ];
	}

	if (fdIn) {
		*fdIn = p_stdin[WRITE];
		close(p_stdin[READ]);
	}

	close(fdFile);

	return pid;
}

/*
 * Streambuf implementation (used with Pstream) that spawns a new process, writes to its stdin and
 * reads from its stdout.
 */
class PstreamBuf : public std::streambuf
{
private:
	bool mWaited;
	int mFdIn, mFdOut;
	pid_t mPid; // Child process id.
	const std::size_t mPutBackSize, mBufferSize;
	std::unique_ptr<char[] > mBuffer;
	std::unique_ptr<char[] > mWriteBuffer;

public:

	explicit PstreamBuf(const std::string& path, const std::vector<std::string>& args,
			std::size_t bufferSize = 16, std::size_t putBackSize = 8)
	: mWaited(false),
	mPutBackSize(std::max(putBackSize, size_t(1))),
	mBufferSize(std::max(bufferSize, mPutBackSize) + mPutBackSize),
	mBuffer(new char[mBufferSize]),
	mWriteBuffer(new char[mBufferSize])
	{
		// Use executable path as argv[0] and add null terminator as last array element.
		std::vector<const char*> argv;
		argv.push_back(path.c_str());
		for (const std::string& s : args)
			argv.push_back(s.c_str());
		argv.push_back(nullptr);

		signal(SIGPIPE, SIG_IGN);

		mPid = popen2(path.c_str(), &mFdIn, &mFdOut, argv.data());
		if (mPid <= 0)
			throw std::system_error(errno, std::system_category());

		setg(mBuffer.get(), mBuffer.get(), mBuffer.get());
		setp(mWriteBuffer.get(), mWriteBuffer.get() + mBufferSize - 1);
	}

	virtual ~PstreamBuf()
	{
		// Wait must always be called (in the same way as join for std::thread).
		if (!mWaited)
			std::terminate();
	}

	PstreamBuf(const PstreamBuf &) = delete;
	PstreamBuf &operator=(const PstreamBuf &) = delete;

	void closeStdIn()
	{
		if (mFdIn >= 0) {
			close(mFdIn);
			mFdIn = -1;
		}
	}

	void closeStdOut()
	{
		if (mFdOut >= 0) {
			close(mFdOut);
			mFdOut = -1;
		}
	}

	int wait()
	{
		if (mFdIn >= 0) {
			close(mFdIn);
			mFdIn = -1;
		}
		if (mFdOut >= 0) {
			close(mFdOut);
			mFdOut = -1;
		}
		// Wait for child process to terminate. Also prevents zombification without having to
		// do signal handling.
		int status;
		int r = waitpid(mPid, &status, 0);
		if (r < 0)
			throw std::system_error(errno, std::system_category());
		mWaited = true;
		return status;
	}

protected:

	virtual int_type underflow() override
	{
		if (gptr() < egptr())
			return traits_type::to_int_type(*gptr());

		// Putback size cannot be bigger than number of bytes read so far (0 on first call).
		std::size_t pbSize = std::min(mPutBackSize, (size_t) (gptr() - eback()));

		std::memmove(mBuffer.get(), egptr() - pbSize, pbSize);
		char* start = mBuffer.get() + pbSize;

		std::size_t readSize = mBufferSize - (start - mBuffer.get());
		std::size_t n = readSome(mFdOut, start, readSize);
		if (n == 0)
			return traits_type::eof();
		if (n < 0) {
			std::string msg = "Error when reading from pipe: ";
			throw std::ios_base::failure(msg + std::strerror(errno));
		}

		setg(mBuffer.get(), start, start + n);

		return traits_type::to_int_type(*gptr());
	}

	virtual int_type overflow(int_type ch) override
	{
		if (ch != traits_type::eof()) {
			*pptr() = ch;
			pbump(1);
			if (flush() < 0) {
				std::string msg = "Error when writing to pipe: ";
				throw std::ios_base::failure(msg + std::strerror(errno));
			}
		}
		return ch;
	}

	virtual int sync() override
	{
		return flush();
	}

	int flush()
	{
		std::ptrdiff_t n = pptr() - pbase();
		pbump(-n);
		int r = writeAll(mFdIn, mWriteBuffer.get(), n);
		return r < 0 ? -1 : 0;
	}
};

/*
 * Ostream implementation that writes to the standard input of a child process.
 */
class Postream : public std::ostream
{
private:
	PstreamBuf& mBuf;
public:

	explicit Postream(PstreamBuf& buf)
	: mBuf(buf)
	{
		rdbuf(&mBuf);
	}

	void close()
	{
		mBuf.closeStdIn();
	}

	Postream(const Postream &) = delete;
	Postream &operator=(const Postream &) = delete;
};

/*
 * Istream implementation that reads from the standard output of a child process.
 */
class Pistream : public std::istream
{
private:
	PstreamBuf& mBuf;
public:

	explicit Pistream(PstreamBuf& buf)
	: mBuf(buf)
	{
		rdbuf(&mBuf);
	}

	void close()
	{
		mBuf.closeStdOut();
	}

	Pistream(const Pistream &) = delete;
	Pistream &operator=(const Pistream &) = delete;
};

/*
 * Abstraction of a child process. Creates streams for the standard input/output. Wait
 * must always be called when ending the process.
 */
class Process : public std::basic_iostream<char>
{
private:
	PstreamBuf mBuf;
	Postream mStdIn;
	Pistream mStdOut;
public:

	explicit Process(const std::string& path, const std::vector<std::string>& args =
			std::vector<std::string>())
	: mBuf(path, args), mStdIn(mBuf), mStdOut(mBuf)
	{
	}

	Postream& in()
	{
		return mStdIn;
	}

	Pistream& out()
	{
		return mStdOut;
	}

	int wait()
	{
		return mBuf.wait();
	}

	Process(const Process &) = delete;
	Process &operator=(const Process &) = delete;
};

}

#endif