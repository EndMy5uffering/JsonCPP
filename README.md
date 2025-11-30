# JSON Parser (C++)

This project is a simple JSON parser written in C++. It includes a simple lexer to generate tokens, a parser for the token list, token and value abstractions, and a simple typed interface for accessing parsed JSON data. Everything is designed around strong ownership semantics, strict type safety, and zero copying wherever possible.

( Lexer and Parser are based on the implementation in `https://craftinginterpreters.com/` )

---

## Features

- Full JSON object, array, string, number, boolean, and null support.
- Manual lexer turning raw text into structured tokens.
- Recursive-descent parser constructing a strongly typed JSON tree.
- Strict type checking through `GetValueAs<T>()`.
---

## Architecture Overview

### 1. Parsing

`JSONParser` processes the token list and constructs a tree of `JSONElement` objects.

Each `JSONElement` stores:

- a `ValueType` enum  
- a `std::unique_ptr<BaseValue>` containing the unwrapped value

JSON types map to:

| JSON Type | Internal Representation |
|-----------|--------------------------|
| object    | `std::unordered_map<std::string, JSONElement>` |
| array     | `std::vector<JSONElement>` |
| string    | `Value<std::string>` |
| number    | `Value<double>` |
| boolean   | `Value<bool>` |
| null      | `Value<std::nullptr_t>` |

All object and array construction uses move semantics to avoid copies.

---

### 2. Accessing Values

Example usage:

```cpp
#include "JsonParser.hpp"

int main() 
{
    JSON::JSONParser parser{"../testJSON/test_1.json"};
    parser.LoadSource();

    JSON::JSONElement root = parser.Parse();

    auto& obj = root.GetValueAs<JSON::JSONContainer>().GetValue();

    bool flag = obj["falseBoolean"].GetValueAs<bool>().GetValue();
    std::cout << "Bool: " << flag << "\n";

    std::string text = obj["simpleString"].GetValueAs<std::string>().GetValue();
    std::cout << "String: " << text << "\n";
}
```
```JSON
Sample JSON:

{
    "falseBoolean": false,
    "simpleString": "Hello"
}
```
```
Output:

Bool: 0
String: Hello
```

Error Handling

The parser validates and throws on:

    malformed numbers, booleans, null, and strings

    unterminated arrays or objects

    unexpected tokens

    invalid type casts through GetValueAs<T>

---
### Notes

This parser is designed for clarity and correctness, not raw performance.
Copy avoidance and move semantics are used everywhere possible.


---