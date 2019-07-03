#pragma once

#include <list>
#include <vector>
#include <string>
#include <exception>
#include <Python.h>

#include <utils/tinyformat.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

class hVerboseLevel {
public:
    static void set(int n, std::string tag) { _verbose_level = n; _verbose_tag = tag; }
    static int get_level() { return _verbose_level; }
    static std::string get_tag() { return _verbose_tag; }

private:     
   static int _verbose_level;
   static std::string _verbose_tag;
};

class hException: public std::exception { 

public:
   template<typename... Args>
   hException(const char *msg, const Args&... args) {
      _msg = tfm::format(msg, args ...);

   }
   
   virtual const char* what() const throw() { return _msg.c_str(); }

protected:
   std::string _msg;
};

template<typename... Args>
void hAssert(bool condition, const char *msg, const Args&... args) {
#if __DEBUG__    
   std::string fmt = tfm::format(msg, args ...);
   if (!condition) throw hException(msg, args ...);
#endif   
}

template<typename... Args>
void hEXCEPTION(const char *msg, const Args&... args) {
   std::string fmt = tfm::format(msg, args ...);
   throw hException(msg, args ...);
}


template<typename... Args>
void hEXCEPTION_IF(bool condition, const char *msg, const Args&... args) {
   if (condition) { 
       hEXCEPTION(msg, args ...);
   }    
}

template<typename... Args>
void hWARNING(const char *msg, const Args&... args) {
   std::string fmt = tfm::format(msg, args ...);
   PyErr_WarnEx(PyExc_RuntimeWarning, fmt.c_str(), 2);
}

template<typename... Args>
void hWARNING_IF(bool condition, const char *msg, const Args&... args) {
   if (condition) {
      hWARNING(msg, args ...);
   }   
}

template<typename... Args>
void hOUT(const char *msg, const Args&... args) {
   std::string fmt = tfm::format(msg, args ...);
   std::cout << fmt << std::endl;
}

// delayed
template<typename... Args>
void hLOG_fn(int n, std::string tag, std::string file, std::string fn, int line, std::string (*fn_msg)(void *), void *data, const Args&... args) {
   if (hVerboseLevel::get_level() < n) return;
   
   if (tag != "" && !(boost::algorithm::starts_with(tag , hVerboseLevel::get_tag()))) return;

   std::string msg = (*fn_msg)(data);

   std::string msg_info = tfm::format("\033[0;34m[dbg:%d:%s]\033[3;38;2;173;216;230m <%s:%s():%d>\033[0;33m %s\033[0m", n, tag, boost::filesystem::path(file).filename().string(), fn.c_str(), line, msg);

   hOUT(msg_info.c_str(), args ...);
}

template<typename... Args>
void hLOG(int n, std::string tag, std::string file, std::string fn, int line, const char *msg, const Args&... args) {
   if (hVerboseLevel::get_level() < n) return;
   
   if (tag != "" && !(boost::algorithm::starts_with(tag , hVerboseLevel::get_tag()))) return;

   std::string msg_info = tfm::format("\033[0;34m[dbg:%d:%s]\033[3;38;2;173;216;230m <%s:%s():%d>\033[0;33m %s\033[0m", n, tag, boost::filesystem::path(file).filename().string(), fn.c_str(), line, msg);
   hOUT(msg_info.c_str(), args ...);
}

#define hlog_fn(n, tag, fn, data, ...) hLOG_fn(n, tag, __FILE__, __func__, __LINE__, fn, data,##__VA_ARGS__)

#define hlog(n, tag, msg, ...) hLOG(n, tag, __FILE__, __func__, __LINE__, msg,##__VA_ARGS__)

template <class T>
std::ostream& operator<<(std::ostream& s, std::list<T> const& v) {
  s << "["; bool first = true;
  for(auto const& elem : v) {
      if (first) { s << elem; first = false; }
      else  s << ", " << elem; }
  s << "]";
  return s;
}

template <class T>
std::ostream& operator<<(std::ostream& s, std::vector<T> const& v) {
  s << "<"; bool first = true;
  for(auto const& elem : v) {
      if (first) { s << elem; first = false; }
      else  s << ", " << elem; }      
  s << ">";
  return s;
} 
