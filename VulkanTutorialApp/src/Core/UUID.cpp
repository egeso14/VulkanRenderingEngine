#include "UUID.h"
#include <random>

static std::mt19937_64 s_Engine(std::random_device{}());
static std::uniform_int_distribution<uint64_t> s_UniformDist;

UUID::UUID()
    : m_UUID(s_UniformDist(s_Engine))
{
}

UUID::UUID(uint64_t uuid)
    : m_UUID(uuid)
{
}