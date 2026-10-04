#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : 
    _output(capacity), 
    _capacity(capacity), 
    _eof(false),
    _eof_index(0U),
    _unassembled_bytes(0U),
    _storages() {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    // border check - right border
    bool right_truncated = false, rejected_data = false, rejected_eof = false;
    size_t recv_size = data.size(), start_index = index, drop_head = 0U;
    if (index > _output.bytes_read() + _capacity || index + data.size() <= _output.bytes_written()) {
        // reject exceed index
        rejected_data = true;
        rejected_eof = true;
        if (index + data.size() == _output.bytes_written()) rejected_eof = false;

    } else {
        if (index + data.size() > _output.bytes_read() + _capacity) {
            // right-truncate
            recv_size = recv_size + (_output.bytes_read() + _capacity) - (index + data.size());
            right_truncated = true;
        } 
        if (index < _output.bytes_written()) {
            // left-truncate
            drop_head = _output.bytes_written() - index;
            recv_size = recv_size - drop_head;
            start_index = _output.bytes_written();
        }
    }

    // eof handle
    if (eof && !_eof && !right_truncated && !rejected_eof) {
        _eof = true;
        _eof_index = index + data.size();
    }

    // merge
    if (!rejected_data && recv_size)  merge_string(data.substr(drop_head, recv_size), start_index);

    // reassemble
    reassemble_string();
}

void StreamReassembler::merge_string(std::string data, uint64_t index) {
    auto it = _storages.begin(), end = _storages.end();

    while (it != end) {
        // not reach
        if (it->first + it->second.size() < index) it++;
        else if (it->first > index + data.size()) break; 
        else {
            // overlap, merge it:
            size_t left_index = min(it->first, index), right_index = max(it->first + it->second.size(), index + data.size());
            data.insert(0, index - left_index, ' ');
            data.resize(right_index - left_index);
            std::copy(it->second.begin(), it->second.end(), data.begin() + (it->first - left_index));
            // update data and index
            it = _storages.erase(it);
            index = left_index;
        }
    }
    _storages.insert({index, data});
    
    _unassembled_bytes = 0U;
    for (auto it1 = _storages.begin(); it1 != _storages.end(); it1++) {
        _unassembled_bytes += it1->second.size();
    }
}

void StreamReassembler::reassemble_string() {
    auto it = _storages.find(_output.bytes_written());
    if (it != _storages.end()) {
        // dont have to border check since merge func has already solved it
        _output.write(it->second);

        // update _unassemble_bytes
        _unassembled_bytes -= it->second.size();
        
        // remove
        _storages.erase(it);
    }

    if (_eof && _eof_index == _output.bytes_written()) {
        _output.end_input();
    }
}

size_t StreamReassembler::unassembled_bytes() const { return _unassembled_bytes; }

bool StreamReassembler::empty() const { return _storages.empty(); }
