#include <utils/hmsg.hpp>
#include <boost/timer/timer.hpp>
#include <string>

class HTimerLog {
public:    
    template<typename... Args>
    HTimerLog(int level, std::string tag, std::string file, std::string fn, int line, const char *msg, const Args&... args) {
        _level = level;
        _tag = tag;
        _file = file;
        _fn = fn;
        _line = line;
        hLOG(_level, _tag, _file, _fn, _line, msg, args ...); 
    }

    ~HTimerLog() {
        hLOG_fn(_level, _tag, _file, _fn, _line, [] (void *data) -> std::string  { 
            unsigned long elapsed = ((boost::timer::cpu_timer *) data)->elapsed().wall;
            unsigned long total_sec = round((double) elapsed / 1000000000.0);
            unsigned long min = round((double) total_sec / 60.0);
            unsigned long sec = total_sec - min*60;

            return tfm::format("... [completed in %lum:%lus]!", min, sec); }, 
        &_timer);   
    }

protected:
    boost::timer::cpu_timer _timer;     
    int _level;
    std::string _tag;
    std::string _file;
    std::string _fn;
    int _line;
    std::string _msg;
};

#define tlog(n, tag, msg, ...) HTimerLog __timer__ (n, tag, __FILE__, __func__, __LINE__, msg,##__VA_ARGS__)

