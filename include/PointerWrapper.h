#ifndef POINTERWRAPPER_H
#define POINTERWRAPPER_H

#include <utility>
#include <stdexcept>

template<typename T>
class PointerWrapper {
private:
    T* ptr;  // Raw pointer to the managed object

public:
    // ========== CONSTRUCTION AND DESTRUCTION ==========

    /**
     * Default constructor - creates empty wrapper
     */
    PointerWrapper() : ptr(nullptr) {}

    /**
     * Constructor from raw pointer - wraps the pointer
     */
    explicit PointerWrapper(T* p) : ptr(p) {}

    ~PointerWrapper() {
        delete ptr;
    }

    // ========== COPY OPERATIONS (DELETED) ==========

    /**
     * Copy constructor is DELETED
     * Each object has exactly one owner.
     */
    PointerWrapper(const PointerWrapper& other) = delete;

    PointerWrapper& operator=(const PointerWrapper& other) = delete;

    PointerWrapper(PointerWrapper&& other) noexcept : ptr(other.ptr){
        other.ptr = nullptr;
    }

    PointerWrapper& operator=(PointerWrapper&& other) noexcept {
        if (this != &other) {
            delete ptr;
            ptr = other.ptr;
            other.ptr = nullptr;
        }
        return *this;
    }

    // ========== ACCESS OPERATIONS ==========

    T& operator*() const {
        if (!ptr) throw std::runtime_error("Cannot dereference an empty PointerWrapper");
        return *ptr;
    };

    T* operator->() const {
        if (!ptr) throw std::runtime_error("Cannot access an empty PointerWrapper");
        return ptr;
    }

    T* get() const {
        return ptr;
    }

    // ========== OWNERSHIP MANAGEMENT ==========

    T* release() {
        T* temp = ptr;
        ptr = nullptr;
        return temp;
    }

    void reset(T* new_ptr = nullptr) {
        if (ptr != new_ptr) {
            delete ptr;
            ptr = new_ptr;
        }
    }

    // ========== UTILITY FUNCTIONS ==========

    explicit operator bool() const {
        return ptr != nullptr;
    }

    /**
     * Swap two PointerWrapper objects
     * Exchanges ownership without copying objects.
     */
    void swap(PointerWrapper& other) noexcept {
        std::swap(ptr, other.ptr);
    }
};

// ========== NON-MEMBER FUNCTIONS ==========

template<typename T, typename... Args>
PointerWrapper<T> make_pointer_wrapper(Args&&... args) {
    return PointerWrapper<T>(new T(std::forward<Args>(args)...));
}

template<typename T>
void swap(PointerWrapper<T>& lhs, PointerWrapper<T>& rhs) noexcept {
    lhs.swap(rhs);
}

#endif // POINTERWRAPPER_H
