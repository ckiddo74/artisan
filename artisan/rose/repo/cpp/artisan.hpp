#pragma once

#include <map>
#include <string>
#include <vector>
#include <chrono>


class Artisan {
public:    
    enum Op { op_add, op_last, op_store, op_stat };
    static void monitor(std::string key, double val, Op op) {
        _vals[key].store(val, op);
    }


    class Timer {
        std::string _key;
        std::chrono::high_resolution_clock::time_point _t0;
        Op _op;

        public:
        Timer(std::string key, Op op = op_last) {
            _t0 = std::chrono::high_resolution_clock::now();
            _key = key;
            _op = op;
        }
        ~Timer() {
            double t =  std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - _t0).count();
            Artisan::monitor(_key, t, _op);
        }

    };


    static void report(std::string name) {
        FILE *fp = fopen(name.c_str(),"w");
        fprintf(fp, "{\n");
        bool outer_first = true;
        for (auto k: _vals) {
            if (!outer_first) 
                fprintf(fp, "    ],\n");
            outer_first =  false;
            fprintf(fp, "   \"%s\": [\n", k.first.c_str());
            Val &val = k.second;
            bool first = true;
            for (auto d: val.data) {
                if (first) {
                   fprintf(fp, "      %.2f\n", d);
                   first = false;
                } else {
                    fprintf(fp, "    , %.2f\n", d);
                }
            }
            // fprintf(fp, "    ],\n");
        }
        fprintf(fp, "    ]\n}\n");
        fclose(fp);
    }
private:
    struct Val {
        std::vector<double> data;
        void store(double val, Op op) {
            if (op == op_add) {
                if (data.size() != 1) {
                    data = { 0.0 };
                }
                double &d = data[0];
                d += val;
            } else if (op == op_store) {
                data.insert(data.end(), val);
            } else if (op == op_last) {
                if (data.size() != 1) {
                    data = { 0.0 };
                }
                data[0] = val;
            } else if (op == op_stat) {
                if (data.size() != 4) {
                    data = {0, 0.0, 0.0, 0.0};
                }
                if (data[0] == 0) {
                    data[0] = 1; // count
                    data[1] = val; // average
                    data[2] = val; // min
                    data[3] = val; // max
                } else {                    
                    data[1] = ((data[1]*data[0])+val)/++data[0];
                    if (val < data[2]) {
                        data[2] = val;
                    }
                    if (val > data[3]) {
                        data[3] = val;
                    }
                }

            }

        }
    };

    static std::map<std::string, Val> _vals;

};

#ifdef __ARTISAN__INIT__
std::map<std::string, Artisan::Val> Artisan::_vals;
#endif