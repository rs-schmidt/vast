#include "vast/io/file_stream.h"

namespace vast {
namespace io {

file_input_stream::file_input_buffer::file_input_buffer(file& f)
  : file_(f)
{
}

file_input_stream::file_input_buffer::~file_input_buffer()
{
  if (! close_on_delete_)
    return;
  if (file_.is_open())
    file_.close();
}

bool
file_input_stream::file_input_buffer::read(void* data, size_t bytes, size_t* got)
{
  return file_.read(data, bytes, got);
}

bool file_input_stream::file_input_buffer::skip(size_t bytes, size_t *skipped)
{
  return file_.seek(bytes, skipped) || input_buffer::skip(bytes, skipped);
}

void file_input_stream::file_input_buffer::close_on_delete(bool flag)
{
  close_on_delete_ = flag;
}

bool file_input_stream::file_input_buffer::close()
{
  return file_.close();
}

file_input_stream::file_input_stream(file& f, size_t block_size)
  : buffer_(f)
  , buffered_stream_(buffer_, block_size)
{
}

void file_input_stream::close_on_delete(bool flag)
{
  buffer_.close_on_delete(flag);
}

bool file_input_stream::close()
{
  return buffer_.close();
}

bool file_input_stream::next(void const** data, size_t* size)
{
  return buffered_stream_.next(data, size);
}

void file_input_stream::rewind(size_t bytes)
{
  buffered_stream_.rewind(bytes);
}

bool file_input_stream::skip(size_t bytes)
{
  return buffered_stream_.skip(bytes);
}

uint64_t file_input_stream::bytes() const
{
  return buffered_stream_.bytes();
}


file_output_stream::file_output_buffer::file_output_buffer(file& f)
  : file_(f)
{
}

file_output_stream::file_output_buffer::~file_output_buffer()
{
  if (! close_on_delete_)
    return;
  if (file_.is_open())
    file_.close();
}

bool file_output_stream::file_output_buffer::write(
    void const* data, size_t bytes, size_t* put)
{
  return file_.write(data, bytes, put);
}

void file_output_stream::file_output_buffer::close_on_delete(bool flag)
{
  close_on_delete_ = flag;
}

file_output_stream::file_output_stream(file& f, size_t block_size)
  : buffer_(f)
  , buffered_stream_(buffer_, block_size)
{
}

file_output_stream::~file_output_stream()
{
  flush();
}

void file_output_stream::close_on_delete(bool flag)
{
  buffer_.close_on_delete(flag);
}

bool file_output_stream::flush()
{
  return buffered_stream_.flush();
}

bool file_output_stream::next(void** data, size_t* size)
{
  return buffered_stream_.next(data, size);
}

void file_output_stream::rewind(size_t bytes)
{
  buffered_stream_.rewind(bytes);
}

uint64_t file_output_stream::bytes() const
{
  return buffered_stream_.bytes();
}

} // namespace io
} // namespace vast
