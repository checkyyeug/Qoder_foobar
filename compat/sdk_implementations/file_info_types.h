/**
 * @file file_info_types.h
 * @brief file_info 相关类型定义
 * @date 2025-12-09
 */

#pragma once

#include <cstdint>

namespace foobar2000_sdk {

// 音频信息结构
struct audio_info {
    uint32_t m_sample_rate;      // 采样率
    uint32_t m_channels;         // 通道数
    uint32_t m_bitrate;          // 比特率
    double   m_length;           // 时长（秒）
    
    audio_info() : m_sample_rate(44100), m_channels(2), m_bitrate(0), m_length(0) {}
};

typedef audio_info audio_info_impl;

// 文件统计信息
struct file_stats {
    uint64_t m_size;             // 文件大小
    uint64_t m_timestamp;        // 时间戳
    
    file_stats() : m_size(0), m_timestamp(0) {}
};

// 重采样模式在 audio_chunk_interface.h 中定义

} // namespace foobar2000_sdk
