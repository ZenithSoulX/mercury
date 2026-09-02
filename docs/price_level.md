# PriceLevel

## Purpose
`PriceLevel` represents all active resting orders at a single price, on a single side. It maintains strict FIFO ordering among those orders (price-time priority) and tracks their aggregate resting volume. 

## Responsibilities
It is responsible for :
- Holding resting orders at one price as time priority.
- O(1) insertion, removal, and front-of-the-queue access.
-  Maintaining an aggregate `total_volume_` in sync with its orders.

It is **not** responsible for :
- Matching logic, or knowing which side is "best".
- Owning `Order` memory (only holds non-owning Order*).
- Enforcing price-time priority across levels (that's `PriceIndex`/`BookSide` job).

## Data Model 
``` cpp
class PriceLevel {
    Price price_;
    Side side_;
    Order* head_ = nullptr;
    Order* tail_ = nullptr;
    std::size_t count_ = 0;
    Volume total_volume_{0};
};
```
Orders are linked directly via `prev_`/`next_` pointers stored on `Order` itself (see Order.md). `PriceLevel` holds only the chain's endpoints (`head_`/`tail_`) and does not own or allocate any separate list-node structure.

## Invariants
- Every order has the level's exact `price_` and `side_`.
- `total_volume_` == sum of every order's `remainingVolume()`.
- (head_ == nullptr) == (tail_ == nullptr) == (total_volume_ == 0).
- Walking the chain via `next()` from `head_` reaches `tail_` in exactly `count_` steps (and the reverse, via `prev()` from `tail_`, is symmetric).

All of the invariants are verified via a debug-only `verifyInvariants()`, called after every mutation.

## Design Decisions
**Intrusive doubly-linked list, not `std::list<Order*>`.** An earlier version used `std::list<Order*>` deliberately, as a simpler-to-verify first implementation. `std::list` iterators are stable across other insertions/removals, which made early correctness easier to reason about. That version was benchmarked first, then replaced with the current intrusive design (linkage pointers stored directly on Order), and re-benchmarked, so the latency difference is backed by a real, controlled measurement rather than assumed. See the project README's optimization case study for the measured before/after results. Median order-submission latency improved by roughly 34–66% (depending on dataset), primarily because order data and linkage now reside in the same object, eliminating a heap allocation per resting order and improving cache locality.

**No separate `Iterator` type — callers hold `Order*` directly.** With std::list, cancellation needed a stored std::list::iterator to erase in O(1) without a scan. The intrusive design removes this need entirely: an Order* is its own position in the chain, since the chain lives on the Order object itself. BookSide's OrderLocation reflects this, thus, it stores {PriceLevel*, Order*}, not an iterator (see BookSide.md).

**`total_volume_` is `Volume` and not `Quantity`.** `Quantity` must always be strictly positive but `Volume` has no such restriction. This distinction matters concretely: if `total_volume_` (or an order's remaining quantity after fills) were typed as `Quantity`, the last order leaving a level, or any order filling to completion, would require constructing a zero value and crash, since `Quantity`'s constructor rejects zero. Mercury uses two distinct numeric types specifically to make this invalid state unrepresentable rather than relying on avoiding it by convention.

**Every mutation carries the responsibility of maintaining its own linkage correctly.** A stale `prev_`/`next_`/`level_` on an erased Order is a real, silent corruption risk. Because the linkage pointers live directly on Order, an incomplete cleanup during erase (e.g., forgetting to null out `level_`) doesn't just leave stale metadata, it leaves part of the actual data structure in an inconsistent state, potentially causing a second erase to corrupt `total_volume_`/`count_`, or route through a `PriceLevel` that has since been destroyed. `erase()` clears `prev_`, `next_`, and `level_` on the removed order as a single atomic step for this reason, and verifyInvariants() checks chain traversal length against `count_` specifically to catch this class of bug early.

## Complexity

| Operation | Complexity |
| --------- | ---------- |
| Add/erase/front | O(1) |
| Reduce volume | O(1) |

Note: "O(1)" here refers to algorithmic complexity, unchanged from the `std::list` version. The intrusive conversion is a constant-factor improvement (no per-node heap allocation, better cache locality), not a change in asymptotic behavior.

`Last Updated ` : 3rd Sept 2026