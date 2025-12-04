#pragma once
#include <cstdint>
#include <functional>

class UUID
{
public:
    UUID();                  // generate new
    UUID(uint64_t uuid);     // from existing value

    operator uint64_t() const { return m_UUID; }

    bool operator==(const UUID& other) const { return m_UUID == other.m_UUID; }
    bool operator!=(const UUID& other) const { return !(*this == other); }

private:
    uint64_t m_UUID;
};

// allow UUID as key in unordered_map
namespace std
{
    template<>
    struct hash<UUID>
    {
        size_t operator()(const UUID& uuid) const
        {
            return (uint64_t)uuid;
        }
    };
}
