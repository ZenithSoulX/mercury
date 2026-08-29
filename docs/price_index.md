# PriceIndex

## Purpose
It maps `Price` to `PriceLevel` for one side of the book, keeping levels sorted so top-of-the-book and ordered iteration are always available without a scan.

## Responsibilities
It is responsible for :
- Creating, finding and removing `PriceLevel`s by price.
- Keeping levels sorted in ascending order by price at all times.

It is **not** responsible for :
- Deciding what "best" means (highest vs lowest) since that's `BookSide`'s job. `PriceIndex` itself has no concept of bid/ask.
- Matching, order ownership or market state.

## Data Model
struct PriceLevelEntry {
    Price price;
    std::unique_ptr<PriceLevel> level;
};
class PriceIndex {
    std::vector<PriceLevelEntry> levels_;
};

## Design Decisions
1. Chose sorted `std::vector` and not `std::map` or a dense array-ladder because :
    - Red-black tree nodes are scattered across the heap, giving poor cache locality for a structure accessed on nearly every operation.
    - A dense, price-indexed array requires committing to a bounded price range up front and pays a real memory cost for sparse occupancy. 
    - vector was a middle ground even though it has O(log(n)) lookup since it is contiguous, cache-friendly and with exactly one structure to keep consistent.

2. Insertion and removal is O(n) and not O(log(n)) and finding the position is O(log(n)). This is stated honestly rather than rounded up : for realistic numbers of occupied price levels, a contiguous shift is typically faster in practice than a tree rebalance, despite larger asymptotic bound, precisely because it's a cache-friendly memmove rather than pointer-chasing. 

3. `unique_ptr<PriceLevel>` keeps `PriceLevelEntry` cheap to move during vector reshuffling since it is equivalent to moving a pointer and not a whole `PriceLevel` and it gives every level a stable address independent of the vector's own reallocation. 

## Invariants 
`PriceIndex` never holds an empty `PriceLevel`. This is enforced by the callers (`BookSide::removeOrder()` erases the index entry the moment a level empties) and checked in debug builds via `verifyInvariants()`. 
This keeps `size()`/iteration/best-price queries meaningful without checking null and skip on every access.

## Complexity 

| Operation | Complexity |
| --------- | ---------- |
| Find/contains | O(log(n)) |
| getOrCreate/erase | O(log(n)) find + O(n) shift |
| Best (front/back) | O(1) |

## Future Extensions (Author's Notes)
`std::vector` may be replaced with a Sparse set bu right now I am not sure if this is the actual bottleneck and contributing meaningfully to the tail latency spike. Right now my first priority is to shift to intrusive-list conversion. 

`Last Updated ` : 29th Aug 2026