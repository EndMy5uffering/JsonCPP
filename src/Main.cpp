/*
    =============================================================================
    JSON Parser - Copyright (c) 2025 Your Philipp Havemann
    =============================================================================

    All code in this file is original work and is protected under copyright law.
    Unauthorized copying, redistribution, or use without proper credit is strictly
    prohibited. 

    Usage is permitted **only** if proper credit is given to the original author.
    Any use of this code is entirely at the user's own risk. The author accepts
    no responsibility for any damage, loss, or issues arising from the use of
    this code.

    For questions or permissions beyond crediting, please contact the author.
    
    =============================================================================
*/

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

