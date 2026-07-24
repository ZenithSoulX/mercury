#pragma once
#include <cstdint>
#include <chrono>
#include <compare>

namespace mercury {
    /** Strongly typed wrapper around primitive values.
    *
    * Prevents accidental mixing of semantically different values such as OrderID, Price and Quantity.
    */
    template <typename Tag, typename T>
    class [[nodiscard]] StrongType {
        private : T value;
        public :
            StrongType() = delete;
            constexpr explicit StrongType(T v) noexcept : value(v) {}
            constexpr const T& get() const noexcept {return value;}
            constexpr bool operator==(const StrongType&) const noexcept = default;
            auto operator<=>(const StrongType&) const noexcept = default;
    };
    // TODO(rahul):
    // Provide std::hash specialization.
    struct OrderIDTag {};
    struct PriceTag {};
    struct QuantityTag {};
    struct SequenceNumberTag {};
    struct VolumeTag {};
    using OrderID = StrongType<OrderIDTag, std::uint64_t>;
    using Price = StrongType<PriceTag, std::int64_t>;  //Price represented in integer ticks (e.g., paise, or smallest tradeable unit).
    using Quantity = StrongType<QuantityTag, std::uint64_t>;
    using Volume = StrongType<VolumeTag, std::uint64_t>;
    using SequenceNumber = StrongType<SequenceNumberTag, std::uint64_t>;
    using Timestamp = std::chrono::steady_clock::time_point;
}
