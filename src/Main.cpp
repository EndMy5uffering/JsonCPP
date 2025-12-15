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
    //JSON::Parse("../test_1.json");

    JSON::JSONParser parser{"../testJSON/test_1.json"};
    JSON::Element element = parser.Parse();

    auto& container = element.GetValueAs<JSON::JObject>();

    auto& test = container["falseBoolean"];
    auto& value = test.GetValueAs<bool>();
    std::cout << "Bool Value: " << value << "\n";


    auto& test2 = container["simpleString"];
    auto& sstring = test2.GetValueAs<std::string>();
    std::cout << "String Value: " << sstring << "\n";

    JSON::JObject* tryGetContainer;
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

    std::cout << "Back to string: \n" << element.ToString() << "\n\n\n";
    std::cout << "Back to string indented: \n" << element.ToString(4) << "\n\n\n";

    std::cout << "Written JSON: " << parser.SaveToFile(element, "../out/test/output.json") << "\n";


    JSON::Element newElement = JSON::Element::From<JSON::JObject>();
    newElement.GetValueAs<JSON::JObject>()["a"] = JSON::Element::From(42);
    newElement.GetValueAs<JSON::JObject>()["b"] = JSON::Element::From<std::string>("asdf");
    newElement.GetValueAs<JSON::JObject>()["c"] = JSON::Element::From<nullptr_t>();

    std::cout << "Written JSON 2: " << parser.SaveToFile(newElement, "../out/test/output2.json") << "\n";


    return 0;
}

