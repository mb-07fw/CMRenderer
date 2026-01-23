/* Designed to be used in classes that inherit from both IRefCounted (interface)
 *   and RefCounted (implementation). Avoids a bunch of boilerplate of wrapping 
 *   virtual functions while also avoiding diamond problems. */
#define impl_ref_count \
public: \
    virtual [[nodiscard]] size_t RefCount() const noexcept override { return RefCountImpl(); } \
    virtual void Release(uint32_t key = 0) noexcept override { return ReleaseImpl(key); } \
protected: \
    virtual void AddRef(uint32_t key = 0) noexcept override { return AddRefImpl(key); } \
    virtual [[nodiscard]] size_t DecRef(uint32_t key = 0) noexcept override { return DecRefImpl(key); } \
public: