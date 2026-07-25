#pragma once
#include <cstdint>
#include <compare>
#include <stdexcept>
#include <cassert>

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
            constexpr T get() const noexcept {return value;}
            constexpr auto operator<=>(const StrongType&) const noexcept = default;
    };
    class [[nodiscard]] Price {
        public :
            explicit constexpr Price(std::int64_t v) : value(validate(v)) {}
            constexpr std::int64_t get() const noexcept {return value;}
            constexpr auto operator<=>(const Price&) const noexcept = default;

        private : 
            static constexpr std::int64_t validate(std::int64_t v){
                assert(v > 0 && "Price must be positive");
                return v;
            }
            std::int64_t value;
    };
    class [[nodiscard]] Quantity {
        public :
            explicit constexpr Quantity(std::uint64_t v) : value(validate(v)) {}
            constexpr std::uint64_t get() const noexcept {return value;}
            constexpr auto operator<=>(const Quantity&) const noexcept = default;

        private : 
            static constexpr std::uint64_t validate(std::uint64_t v){
                assert(v > 0 && "Quantity must be positive");
                return v;
            }
            std::uint64_t value;
    };

    // TODO(rahul):
    // Provide std::hash specialization.
    struct OrderIDTag {};
    struct SequenceNumberTag {};
    struct VolumeTag {};
    struct EventTimestampTag {};
    using OrderID = StrongType<OrderIDTag, std::uint64_t>;
    using Volume = StrongType<VolumeTag, std::uint64_t>;
    using SequenceNumber = StrongType<SequenceNumberTag, std::uint64_t>;
    using EventTimestamp = StrongType<EventTimestampTag, std::uint64_t>;
}
