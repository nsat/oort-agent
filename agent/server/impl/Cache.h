/**
 * Cache.h
 * 
 * A lightweight cache template.
 * 
 * Copyright (c) 2022 Spire Global, Inc.
 * 
 */
#pragma once

#include <chrono> // NOLINT(build/c++11)
#include <mutex> // NOLINT(build/c++11)
#include <unordered_map>
#include <vector>
#include <string>
#include <functional>
#include <utility>

#include <Log.h>

template<class T> class Cache {
    typedef std::function<T(const std::string&)> GetFn;
    const std::string m_name;
    GetFn m_wrapfn;
    bool enabled = true;
    unsigned m_ttl_ms = 5000;
    std::chrono::time_point<
        std::chrono::steady_clock> m_cache_last_cleaned_at =
        std::chrono::steady_clock::now();
    std::mutex m_lock;
    // <key, (created time, data)>
    std::unordered_map<
        std::string,
        std::pair<
            std::chrono::time_point<std::chrono::steady_clock>, T
        >> contents;
    // stats
    unsigned m_gets = 0, m_hits = 0, m_cleaned = 0;
    static const T default_fn(const std::string& key) {return T();}
    bool is_cache_ttl_expired() {
        auto now = std::chrono::steady_clock::now();
        auto limit = now - std::chrono::milliseconds(m_ttl_ms);
        return m_cache_last_cleaned_at < limit;
    }

 public:
    Cache<T>(std::string name, GetFn fn = Cache<T>::default_fn, unsigned timeout_ms = 5000)
              : m_name(name), m_ttl_ms(timeout_ms), m_wrapfn(fn) {}
    Cache<T>(std::string name, unsigned timeout_ms, GetFn fn = Cache<T>::default_fn)
              : m_name(name), m_ttl_ms(timeout_ms), m_wrapfn(fn) {}
    void set_fn(const GetFn &fn) { m_wrapfn = fn; }
    void enable() {enabled = true;}
    void disable() {enabled = false;}
    // remove any expired entries, or drop an expired cache
    void clean_expired() {
        auto now = std::chrono::steady_clock::now();
        auto limit = now - std::chrono::milliseconds(m_ttl_ms);
        for (auto it = contents.begin(); it != contents.end();) {
            if (it->second.first < limit) {
                it = contents.erase(it);
                ++m_cleaned;
            } else {
                ++it;
            }
        }
        m_cache_last_cleaned_at = now;
    }
    // remove all entries
    void flush() {
        const std::lock_guard<std::mutex> guard{m_lock};
        contents.clear();
        Log::info("Flushed cache ?", m_name);
        m_cache_last_cleaned_at = std::chrono::steady_clock::now();
    }
    void stats() {
        Log::info("? cache stats: ? gets, ? hits, ? cleaned",
            m_name, m_gets, m_hits, m_cleaned);
    }
    T get(const std::string& key) {
        return get(key, m_wrapfn);
    }
    T get(const std::string& key, const GetFn &fn) {
        // if cache is not enabled, just return the wrapped function call
        if (!enabled) return fn(key);
        const std::lock_guard<std::mutex> guard{m_lock};
        ++m_gets;
        // prevent unlimited cache growth on consistent misses
        if (is_cache_ttl_expired()) {clean_expired();}
        auto el = contents.find(key);
        auto now = std::chrono::steady_clock::now();
        if (el == contents.end()) {
            Log::debug("Cache miss for ?", key);
            contents.emplace(key, std::make_pair(now, fn(key)));
        } else if (el->second.first < (now- std::chrono::milliseconds(m_ttl_ms))) {
            Log::debug("Cache expired for key: ? cache: ?", key, m_name);
            clean_expired();
            contents.emplace(key, std::make_pair(now, fn(key)));
        } else {
            ++m_hits;
            Log::debug("Cache hit for ?", key);
        }
        if (m_gets % 10000 == 0) {
            stats();
        }
        return contents.at(key).second;
    }
};
