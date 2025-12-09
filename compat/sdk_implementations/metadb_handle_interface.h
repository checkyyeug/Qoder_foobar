/**
 * @file metadb_handle_interface.h
 * @brief metadb_handle 抽象接口定义
 * @date 2025-12-09
 */

#pragma once

#include "common_includes.h"
#include "file_info_interface.h"
#include <memory>
#include <string>
#include <chrono>

namespace foobar2000_sdk {

// 前向声明
class metadb_impl;

// 位置类
class location_t {
public:
    location_t() = default;
    explicit location_t(const char* path) : path_(path ? path : "") {}
    explicit location_t(const std::string& path) : path_(path) {}
    
    const char* get_path() const { return path_.c_str(); }
    const std::string& get_path_ref() const { return path_; }
    
    bool is_empty() const { return path_.empty(); }
    bool is_valid() const { return !path_.empty() && path_.length() < 4096; }
    
    void set_path(const char* path) { path_ = path ? path : ""; }
    void set_path(const std::string& path) { path_ = path; }
    
    location_t substring(size_t start) const {
        return location_t(path_.substr(start));
    }
    
    bool operator==(const location_t& other) const { return path_ == other.path_; }
    bool operator!=(const location_t& other) const { return path_ != other.path_; }
    bool operator<(const location_t& other) const { return path_ < other.path_; }
    
private:
    std::string path_;
};

class playable_location : public location_t {
public:
    playable_location() = default;
    playable_location(const char* path, uint64_t subsong = 0)
        : location_t(path), subsong_index_(subsong) {}
    playable_location(const location_t& loc, uint64_t subsong = 0)
        : location_t(loc), subsong_index_(subsong) {}
    
    uint64_t get_subsong() const { return subsong_index_; }
    void set_subsong(uint64_t idx) { subsong_index_ = idx; }
    
private:
    uint64_t subsong_index_ = 0;
};

// 跟踪统计
struct TrackStatistics {
    uint64_t play_count = 0;                    // 总播放次数
    uint64_t first_played_timestamp = 0;        // 首次播放时间（Unix 时间戳）
    uint64_t last_played_timestamp = 0;         // 最后播放时间（Unix 时间戳）
    uint64_t added_timestamp = 0;               // 添加到库的时间
    
    float rating = 0.0f;                        // 用户评分（0.0-5.0）
    uint32_t skip_count = 0;                    // 跳过次数
    uint64_t total_play_time_ms = 0;            // 总播放时长（毫秒）
    
    // 记录一次播放
    void on_play();
    
    // 记录一次跳过
    void on_skip();
};

/**
 * @class metadb_handle_interface
 * @brief metadb_handle 抽象接口
 */
class metadb_handle_interface {
public:
    virtual ~metadb_handle_interface() = default;
    
    /**
     * @brief 获取文件位置
     * @return 位置的指针
     */
    virtual const playable_location& get_location() const = 0;
    
    /**
     * @brief 获取元数据（const 版本）
     * @return file_info_interface 的引用
     */
    virtual const file_info_interface& get_info() const = 0;
    
    /**
     * @brief 获取元数据（非 const 版本）
     * @return file_info_interface 的引用
     */
    virtual file_info_interface& get_info() = 0;
    
    /**
     * @brief 从实时文件获取元数据（可能扫描）
     * @param p_info 输出 file_info
     * @param p_abort 中止回调
     * @param p_can_expire 是否允许过期缓存
     */
    virtual void get_info_async(file_info_interface& p_info, abort_callback& p_abort, 
                               bool p_can_expire = false) const = 0;
    
    /**
     * @brief 更新元数据
     * @param p_info 新的元数据
     * @param p_abort 中止回调
     */
    virtual void update_info(const file_info_interface& p_info, abort_callback& p_abort) = 0;
    
    /**
     * @brief 从文件刷新元数据（如果文件已更改）
     * @param p_abort 中止回调
     */
    virtual void refresh_info(abort_callback& p_abort) = 0;
};

} // namespace foobar2000_sdk
