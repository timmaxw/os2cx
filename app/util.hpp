#ifndef OS2CX_UTIL_HPP_
#define OS2CX_UTIL_HPP_

#include <assert.h>

#include <string>
#include <vector>

namespace os2cx {

typedef std::string FilePath;

std::string build_command_line(
    const std::string &command,
    const std::vector<std::string> &args);

void maybe_create_directory(const std::string &directory);

class TempDir {
public:
    enum class ExpandTemplate { Yes, No };
    enum class AutoCleanup { Yes, No };
    TempDir(const std::string &tmplate, AutoCleanup auto_cleanup);
    ~TempDir();
    void cleanup();
    FilePath path() {
        return _path;
    }
private:
    AutoCleanup auto_cleanup;
    FilePath _path;
};

template<class Key, class Value>
class ContiguousMap {
public:
    ContiguousMap() { }
    explicit ContiguousMap(Key off) : offset(off.to_int()) { }
    ContiguousMap(Key kbegin, Key kend, Value init) : offset(kbegin.to_int()) {
        values.resize(kend.to_int() - offset, init);
    }
    void shift_keys(Key new_offset) { offset = new_offset.to_int(); }
    const Value &operator[](Key k) const {
        assert(k.to_int() >= offset);
        int index = k.to_int() - offset;
        assert(index < static_cast<int>(values.size()));
        return values[index];
    }
    Value &operator[](Key k) {
        assert(k.to_int() >= offset);
        int index = k.to_int() - offset;
        assert(index < static_cast<int>(values.size()));
        return values[index];
    }
    int size() const { return values.size(); }
    bool key_in_range(Key k) const {
        return k.to_int() >= key_begin().to_int() &&
            k.to_int() <= key_end().to_int();
    }
    Key key_begin() const {return Key::from_int(offset); }
    Key key_end() const { return Key::from_int(offset + values.size()); }
    typename std::vector<Value>::iterator begin() {
        return values.begin();
    }
    typename std::vector<Value>::iterator end() {
        return values.end();
    }
    typename std::vector<Value>::const_iterator begin() const {
        return values.begin();
    }
    typename std::vector<Value>::const_iterator end() const {
        return values.end();
    }
    void reserve(int capacity) { values.reserve(capacity); }
    void push_back(const Value &value) { values.push_back(value); }
private:
    int offset;
    std::vector<Value> values;
};

} /* namespace os2cx */

#endif
