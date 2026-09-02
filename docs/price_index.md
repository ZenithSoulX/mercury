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
``` cpp
struct PriceLevelEntry {
    Price price;
    std::unique_ptr<PriceLevel> level;
};
class PriceIndex {
    std::vector<PriceLevelEntry> levels_;
};
```

## Design Decisions
1. Chose sorted `std::vector` and not `std::map` or a dense array-ladder because :
    - Red-black tree nodes are scattered across the heap, giving poor cache locality for a structure accessed on nearly every operation.
    - A dense, price-indexed array requires committing to a bounded price range up front and pays a real memory cost for sparse occupancy. 
    - vector was a middle ground even though it has O(log(n)) lookup, because it is contiguous, cache-friendly and with exactly one structure to keep consistent.

2. Insertions and removals require O(n) element movement after an O(log n) position lookup (since lookups use binary search (`std::lower_bound()`)). Despite the higher asymptotic cost, contiguous memory movement is often faster in practice than tree rebalancing for the relatively small number of occupied price levels typically observed in an order book. 

3. `PriceIndex` is non-copyable but movable. Copying would require duplicating ownership of every `PriceLevel`, while move semantics allow the index to transfer ownership efficently when needed. 

4. `unique_ptr<PriceLevel>` keeps `PriceLevelEntry` cheap to move during vector reshuffling since it is equivalent to moving a pointer and not a whole `PriceLevel` and it gives every level a stable address independent of the vector's own reallocation. 

## Invariants 
`PriceIndex` never holds an empty `PriceLevel`. This is enforced by the callers (`BookSide::removeOrder()` erases the index entry the moment a level empties) and checked in debug builds via `verifyInvariants()`. 
This keeps `size()`/iteration/best-price queries meaningful without checking null and skip on every access.

## Complexity 

| Operation | Complexity |
| --------- | ---------- |
| Find/contains | O(log(n)) |
| getOrCreate/erase | O(log(n)) find + O(n) shift |
| Best (front/back) | O(1) |

### Future Extensions

A sparse-set based implementation is a potential future optimization.

The current sorted-vector design was selected because occupied price levels are typically sparse and lookup performance is already competitive due to cache locality. Before introducing a more complex structure, profiling should demonstrate that PriceIndex is a meaningful contributor to overall latency.

`Last Updated ` : 3rd Sept 2026