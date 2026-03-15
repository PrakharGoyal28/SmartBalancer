#include "metrics.h"
#include <sstream>

Metrics::Metrics()
{
    total_requests_ = 0;
    failed_requests_ = 0;
    active_connections_ = 0;
}

void Metrics::increment_requests()
{
    total_requests_++;
}

void Metrics::increment_failed()
{
    failed_requests_++;
}

void Metrics::connection_open()
{
    active_connections_++;
}

void Metrics::connection_close()
{
    if (active_connections_ > 0)
        active_connections_--;
}

void Metrics::record_server_request(const std::string& port)
{
    std::lock_guard<std::mutex> lock(mutex_);
    server_requests_[port]++;
}

std::string Metrics::get_metrics()
{
    std::stringstream ss;

    ss << "total_requests " << total_requests_ << "\n";
    ss << "failed_requests " << failed_requests_ << "\n";
    ss << "active_connections " << active_connections_ << "\n";

    for (auto &p : server_requests_)
    {
        ss << "backend_" << p.first << "_requests " << p.second << "\n";
    }

    return ss.str();
}
