## Why no header files included?

- This is a pure, literal mirror of the CSV. Validation and meaning-assignment happen downstream. 
- It is simpler and avoids Price's validation throwing on a value it hasn't checked belongs to a real order yet.