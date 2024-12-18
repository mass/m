#pragma once

#include "u.hpp"

#include <array>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <sys/time.h>

#define LOG(s) (u::log(u::log::s))

namespace u {

//////////////////////////////////////////////////////////////////////////////
/// Simple message logger using RAII. Not designed for speed / efficiency.
class log
{
  public:

    enum Severity { INFO, WARN, ERROR };

    using Callback_t = std::function<void(Severity,std::string_view)>;

    static inline void init_file(std::string_view file) {
      _file = std::ofstream(std::string(file), std::ofstream::out | std::ofstream::app);
    }

    static inline void init_callback(Callback_t&& callback) {
      _callback = std::move(callback);
    }

    log(Severity s);
    ~log();

    template<typename T> inline log& operator<<(const T& mesg) {
      _stream << mesg;
      return *this;
    }
    template<typename T, typename V> inline log& operator<<(const std::pair<T,V>& p) {
      _stream << p.first << "=(" << p.second << ")";
      return *this;
    }

  private:

    log() = delete;
    log(const log& o) = delete;
    log(log&& o) = delete;
    log& operator=(const log& o) = delete;
    log& operator=(log&& o) = delete;

    static inline std::ofstream _file = {};
    static inline Callback_t _callback = {};

    Severity _s;
    std::stringstream _stream;
};

/// Implementation ///////////////////////////////////////////////////////////

inline log::log(Severity s)
  : _s(s)
{
  static constexpr std::array<const char*, 3> SEVERITY_STRING = { "I", "W", "E" };

  struct timeval tv;
  gettimeofday(&tv, nullptr);

  _stream << "[" << SEVERITY_STRING[uint8_t(s)] << "] ";
  _stream << std::put_time(std::localtime(&(tv.tv_sec)), "<%m/%d %H:%M:%S.")
          << std::setfill('0') << std::setw(6) << tv.tv_usec << "> ";
  _stream << "| ";
}

inline log::~log()
{
  _stream << std::endl;
  const auto str = _stream.str();

  std::cout << str;
  std::cout.flush();

  if (_file) {
    _file << str;
    _file.flush();
  }

  if (_callback) {
    _callback(_s, str);
  }
}

};
