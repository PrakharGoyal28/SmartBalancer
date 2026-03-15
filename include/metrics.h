#pragma once
#include <atomic>
#include <unordered_map>
#include <string>
#include <mutex>

class Metrics
{
private:
    std::atomic<long> total_requests_;
    std::atomic<long> failed_requests_;
    std::atomic<long> active_connections_;

    std::unordered_map<std::string, long> server_requests_;
    std::mutex mutex_;

public:
    Metrics();

    void increment_requests();
    void increment_failed();
    void connection_open();
    void connection_close();

    void record_server_request(const std::string& port);

    std::string get_metrics();
};
