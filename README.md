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

`JSONParser` processes the token list and constructs a tree of `Element` objects.

Each `Element` stores:

- a `ValueType` enum  
- a `std::unique_ptr<BaseValue>` containing the unwrapped value

JSON types map to:

| JSON Type | Internal Representation |
|-----------|--------------------------|
| object    | `std::unordered_map<std::string, Element>` |
| array     | `std::vector<Element>` |
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

int main(void) 
{
    /* 
        Create Json parser to read in file 
        Path to file is optional and can be supplied in JSONParser::Parse as well
    */
    JSON::JSONParser parser{"../testJSON/test_1.json"};

    /* Reads file and creates Json object tree structure */
    JSON::Element element = parser.Parse();

    /* 
        Access contained value 
        GetValueAs returns a reference to the contained JSON::JObject
    */
    auto& container = element.GetValueAs<JSON::JObject>();

    /* 
        Getting a value at a key position 
        Returns a reference to an JSON:Element
    */
    auto& test = container["falseBoolean"];

    /* Getting the value ... again */
    auto& value = test.GetValueAs<bool>();
    std::cout << "Bool Value: " << value << "\n";

    /* Same as above but now its a string */
    auto& test2 = container["simpleString"];
    auto& sstring = test2.GetValueAs<std::string>();
    std::cout << "String Value: " << sstring << "\n";

    /* Itterating over key set in the JSON::JObject */
    JSON::JObject* tryGetContainer;
    /* Using TryGetValueAs to test if value is of type JSON::JObject */
    if(element.TryGetValueAs<JSON::JObject>(tryGetContainer))
    {
        auto& value = (*tryGetContainer)["simpleString"];
        std::string* strValue;
        if(value.TryGetValueAs<std::string>(strValue)) std::cout << "Value from try get: " << (*strValue) << "\n";

        for(auto& [key, value] : (*tryGetContainer))
        {
            std::cout << key << " : " << value.GetTypeAsString() << "\n";
        }
    }

    /* Not formated to string */
    std::cout << "Back to string: \n" << element.ToString() << "\n\n\n";

    /* Format with indents and newlines */
    std::cout << "Back to string indented: \n" << element.ToString(4) << "\n\n\n";

    /* Save elements to file with given path (folders along path will be created) */
    std::cout << "Written JSON: " << parser.SaveToFile(element, "../out/test/output.json") << "\n";

    /* Creates a new json object */
    JSON::Element newElement = JSON::Element::From<JSON::JObject>();
    /* Elements of type JSON::JObject allow for ["key"] access */
    /* Elements of type JSON::JArray allow for [number] access */
    newElement["a"] = JSON::Element::From(42);
    newElement["b"] = JSON::Element::From<std::string>("asdf");
    newElement["c"] = JSON::Element::From<nullptr_t>();

    /* Removing null value again */
    newElement.Remove("c");

    /* Does not exist but will be created */
    newElement["empty"];

    newElement["d"] = JSON::Element::From<JSON::JObject>();
    newElement["d"]["da"] = JSON::Element::From(180);
    newElement["d"]["arr"] = JSON::Element::From<JSON::JArray>();

    /* Assigning multiple values to an array */
    newElement["d"]["arr"] << 1 << 2 << 3 << std::string("asdf");

    /* Creating tmp object "by mistake" */
    newElement["tmp"] = JSON::Element::From<JSON::JObject>();
    /* Adding an array ... by "MISTAKE" */
    newElement["tmp"]["arr2"] = JSON::Element::From<JSON::JArray>();
    /* Filling the array still totaly by "MisTaKe" */
    newElement["tmp"]["arr2"] << 11 << 22 << 33 << std::string("xyz");

    /* Removing all content from tmp object as it was a mistake :D */
    /* Leaving tmp for later as null value */
    newElement["tmp"].Remove();

    //newElement.Remove("tmp") // <-- removes tmp from the root structure as well

    /* Runs and returns number */
    if(int number; newElement["a"] >> number) std::cout << "Number: " << number << "\n";

    /* Does not run as "b" is a string */
    if(int number2; newElement["b"] >> number2) std::cout << "Number2: " << number2 << "\n";

    /* Does not run as "someString" does not exist as a key but is created */
    if(std::string someString; newElement["someString"] >> someString) std::cout << "Some String: " << someString << "\n";

    std::cout << "New json object: " << newElement.ToString(4) << "\n";

    return 0;
}
```
Sample JSON:
```JSON
{
    "falseBoolean": false,
    "simpleString": "Hello",
    "simpleInt": 42
}
```
Output:

```
Bool: 0
String: Hello
Value from try get: 42
edgeCases : OBJECT
whitespaceMadness : ARRAY
nestedObject : OBJECT
emptyObject : OBJECT
stringWithEscapes : STRING_LITERAL
emptyArray : ARRAY
[...]
```

Created JSON object from newElement in main above:

```JSON
{
    "someString": null,
    "tmp": null,
    "b": "asdf",
    "d": {
        "arr": [
            1,
            2,
            3,
            "asdf"
        ],
        "da": 180
    },
    "empty": null,
    "a": 42
}
```

### Error Handling

The parser validates and throws on:

- malformed numbers, booleans, null, and strings

- unterminated arrays or objects

- unexpected tokens

- invalid type casts through GetValueAs<T>

---
### Notes

This parser is designed for clarity and correctness, not raw performance.
Copy avoidance and move semantics are used everywhere possible.


---