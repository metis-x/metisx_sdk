#pragma once
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <string>
#include <chrono>
#include <vector>
#include <mutex>
#include <fstream>
#include <iostream>

namespace metisx
{
namespace api
{
namespace util
{

template <class T, uint32_t s>
class Logger
{

    public:
        explicit Logger() // 생성자
            : _tail(0),
              _count(0)
        {
        }

        virtual ~Logger()
        {
        }

        T& operator[](uint32_t idx)
        {
            if (idx >= depth())
            {
                assert(0);
            }

            return _arr[(_tail - idx - 1) % s];
        }

        bool push(T x)
        {
            _arr[_tail] = x;
            _tail       = (_tail + 1) % s;
            if (_count < s)
                _count++;

            return true;
        }

        uint32_t depth()
        {
            return s;
        }

        uint32_t count()
        {
            return _count;
        }

    private:
        T        _arr[s];
        uint32_t _tail;
        uint32_t _count = 0;
};

typedef struct
{
        std::string                           funcName;
        int                                   lineNum;
        std::chrono::steady_clock::time_point timePoint;
} TimePointInfo_t;

typedef struct
{
        std::string tag;
        uint32_t    executionTime;
        uint32_t    cycleToUs;
} DevicePerfInfo_t;

using MxLibPerfLog_t  = std::vector<TimePointInfo_t>;
using DevicePerfLog_t = std::vector<DevicePerfInfo_t>;

using timescale_t = std::chrono::microseconds;

template <uint64_t logSize = 5000>
class JobLogger
{
    private:
        Logger<MxLibPerfLog_t, logSize>  _libLogger;
        Logger<DevicePerfLog_t, logSize> _deviceLogger;
        uint32_t                                                _jobId;
        std::mutex                                              _mutex;
        char                                                    _logFileName[100] = {
            '\0',
        };
        uint64_t _slaveUtilSum = 0;
        uint64_t _numTaskCount = 0;
        const uint32_t TimeUnit = 1000000;

    public:
        JobLogger()
        {
        }

        ~JobLogger()
        {
        }

        void operator()(MxLibPerfLog_t& libLog, DevicePerfLog_t& deviceLog, uint64_t slaveUtil)
        {
            std::lock_guard<std::mutex> guard(_mutex);
            _libLogger.push(libLog);
            _deviceLogger.push(deviceLog);
            _slaveUtilSum += slaveUtil;
            _numTaskCount++;
        }

        void printLog(void)
        {
            int rc = system("mkdir -p log");
            assert(rc != -1);
            std::string   filename;
            std::ofstream fout;
            if (strlen(_logFileName) == 0)
            {
                filename = "log/mx_perf_log_";
                filename += std::to_string(_jobId);
                filename += ".csv";
            }
            else
            {
                filename = "log/" + std::string(_logFileName) + ".csv";
            }

            fout.open(filename);

            if (fout.fail())
            {
                std::cout << "Open mx_perf_log.txt Failed...\n";
                return;
            }

            std::string underBar = "_";
            std::string indexStr = "Index";

            uint32_t muClock     = 188;
            double   sysClock    = (double)((double)1. / (double)(muClock * TimeUnit));
            double   cycleTimeUs = (double)(sysClock * TimeUnit);
            fout << indexStr;

            const auto& libLog = _libLogger[0];
            const auto& devLog = _deviceLogger[0];
            for (uint32_t i = 1; i < libLog.size(); i++)
            {
                std::string funcName = libLog[i - 1].funcName;
                funcName += underBar;
                funcName += std::to_string(libLog[i - 1].lineNum);
                fout << ", " << funcName;
            }
            for (uint32_t i = 0; i < devLog.size(); i++)
            {
                fout << ", " << devLog[i].tag;
            }
            fout << std::endl;

            for (size_t i = 0; i < _libLogger.count(); i++)
            {
                const auto& libLog = _libLogger[i];
                const auto& devLog = _deviceLogger[i];
                fout << i;
                for (size_t j = 1; j < libLog.size(); j++)
                {
                    std::pair<std::string, uint32_t> info;
                    timescale_t                      elapsed = std::chrono::duration_cast<timescale_t>(libLog[j].timePoint - libLog[j - 1].timePoint);
                    fout << ", " << elapsed.count();
                }
                for (uint32_t j = 0; j < devLog.size(); j++)
                {
                    if (devLog[j].cycleToUs == 1)
                    {
                        double cycleToUs = devLog[j].executionTime * cycleTimeUs;
                        fout << ", " << (uint32_t)cycleToUs;
                    }
                    else
                    {
                        fout << ", " << devLog[j].executionTime;
                    }
                }
                fout << std::endl;
            }
            fout.close();
        }

        void setJobId(uint32_t jobId)
        {
            _jobId = jobId;
        }

        void setLogFileName(const char* logFileName)
        {
            strcpy(_logFileName, logFileName);
        }

        double getSlaveUtil(void)
        {
            double slaveUtil = _slaveUtilSum / (double)_numTaskCount;
            return slaveUtil;
        }
};

} // namespace util
} // namespace api
} // namespace metisx
