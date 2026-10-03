#include "byte_stream.hh"

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity)
    : _capacity(capacity)
    , _head(0U)
    , _tail(0U)
    , _remaining_size(capacity)
    , _total_read_bytes(0U)
    , _total_write_bytes(0U)
    , _buf(capacity, '\0')
    , _eof(false) {}

size_t ByteStream::write(const string &data) {
    if (_eof || !_capacity)
        return 0U;
    size_t written_bytes = data.size();
    if (written_bytes > _remaining_size)
        written_bytes = _remaining_size;
    for (size_t pos = 0; pos < written_bytes; pos++, _tail++) {
        _buf[_tail % _capacity] = data[pos];
    }
    _tail = _tail % _capacity;
    _remaining_size -= written_bytes;
    _total_write_bytes += written_bytes;
    return written_bytes;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    if (eof() || !_capacity)
        return {};
    size_t buf_size = _capacity - _remaining_size;
    size_t output_size = min(buf_size, len);
    std::string output{};
    output.reserve(output_size);
    for (size_t pos = 0; pos < output_size; pos++) {
        output += _buf[(_head + pos) % _capacity];
    }
    return output;
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    if (eof() || !_capacity)
        return;
    size_t buf_size = _capacity - _remaining_size;
    size_t output_size = min(buf_size, len);
    _head = (_head + output_size) % _capacity;
    _remaining_size += output_size;
    _total_read_bytes += output_size;
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    string output = peek_output(len);
    pop_output(len);
    return output;
}

void ByteStream::end_input() { _eof = true; }

bool ByteStream::input_ended() const { return _eof; }

size_t ByteStream::buffer_size() const { return _capacity - _remaining_size; }

bool ByteStream::buffer_empty() const { return !buffer_size(); }

bool ByteStream::eof() const { return _eof && buffer_empty(); }

size_t ByteStream::bytes_written() const { return _total_write_bytes; }

size_t ByteStream::bytes_read() const { return _total_read_bytes; }

size_t ByteStream::remaining_capacity() const { return _remaining_size; }