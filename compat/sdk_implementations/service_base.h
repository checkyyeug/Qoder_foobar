/**
 * @file service_base.h
 * @brief service_base 和引用计数的前向声明
 * @date 2025-12-09
 */

#pragma once

#include "../foobar_sdk/foobar2000_sdk.h"
#include <functional>
#include <cstdint>

namespace foobar2000_sdk {

// 前向声明
class service_base;
class service_factory_base;
class service_ptr;

// GUID 结构定义
typedef struct _GUID {
    uint32_t Data1;
    uint16_t Data2;
    uint16_t Data3;
    uint8_t  Data4[8];
    
    bool operator==(const struct _GUID& other) const {
        return Data1 == other.Data1 && Data2 == other.Data2 && 
               Data3 == other.Data3 && 
               Data4[0] == other.Data4[0] && Data4[1] == other.Data4[1] &&
               Data4[2] == other.Data4[2] && Data4[3] == other.Data4[3] &&
               Data4[4] == other.Data4[4] && Data4[5] == other.Data4[5] &&
               Data4[6] == other.Data4[6] && Data4[7] == other.Data4[7];
    }
    
    bool operator!=(const struct _GUID& other) const {
        return !(*this == other);
    }
} GUID;

// 服务回调函数类型定义
typedef std::function<void(service_factory_base* factory, void* ctx)> t_service_list_func;

// 全局服务注册辅助函数（前向声明）
// 将在后续提交中与服务注册中心集成
void service_list(t_service_list_func func, void* ctx);

// 服务工厂基类（抽象接口，带引用计数）
class service_factory_base : public service_base {
public:
    virtual ~service_factory_base() = default;
    
    /**
     * @brief 创建服务实例
     * @return 服务指针
     */
    virtual service_ptr create_service() = 0;
    
    /**
     * @brief 获取服务 GUID
     * @return GUID 引用
     */
    virtual const GUID& get_guid() const = 0;
    
    /**
     * @brief 获取服务名称（可选）
     * @return 服务名称
     */
    virtual const char* get_name() const { return nullptr; }
    
    // service_base 接口的实现（简化版本）
    int service_add_ref() override {
        // 简单实现：工厂通常是单例，返回固定值
        return 1;
    }
    
    int service_release() override {
        // 简单实现：工厂通常是单例，不实际释放
        return 1;
    }
};

// 服务智能指针类定义
class service_ptr {
private:
    service_base* ptr_;
    
public:
    service_ptr() : ptr_(nullptr) {}
    
    explicit service_ptr(service_base* p) : ptr_(p) {
        if (ptr_) add_ref();
    }
    
    service_ptr(const service_ptr& other) : ptr_(other.ptr_) {
        if (ptr_) add_ref();
    }
    
    service_ptr(service_ptr&& other) noexcept : ptr_(other.ptr_) {
        other.ptr_ = nullptr;
    }
    
    ~service_ptr() {
        if (ptr_) release();
    }
    
    service_ptr& operator=(const service_ptr& other) {
        if (this != &other) {
            if (ptr_) release();
            ptr_ = other.ptr_;
            if (ptr_) add_ref();
        }
        return *this;
    }
    
    service_ptr& operator=(service_ptr&& other) noexcept {
        if (this != &other) {
            if (ptr_) release();
            ptr_ = other.ptr_;
            other.ptr_ = nullptr;
        }
        return *this;
    }
    
    service_base* operator->() const { return ptr_; }
    service_base& operator*() const { return *ptr_; }
    service_base* get_ptr() const { return ptr_; }
    
    bool is_valid() const { return ptr_ != nullptr; }
    bool is_empty() const { return ptr_ == nullptr; }
    
    void release() {
        if (ptr_) {
            ptr_->service_release();
            ptr_ = nullptr;
        }
    }
    
private:
    void add_ref();
};

} // namespace foobar2000_sdk
