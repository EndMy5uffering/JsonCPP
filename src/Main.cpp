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
    JSON::JSONElement element = parser.Parse();

    auto& container = element.GetValueAs<JSON::JSONObject>();

    auto& test = container["falseBoolean"];
    auto& value = test.GetValueAs<bool>();
    std::cout << "Bool Value: " << value << "\n";


    auto& test2 = container["simpleString"];
    auto& sstring = test2.GetValueAs<std::string>();
    std::cout << "String Value: " << sstring << "\n";

    JSON::JSONObject* tryGetContainer;
    if(element.TryGetValueAs<JSON::JSONObject>(tryGetContainer))
    {
        auto& value = (*tryGetContainer)["simpleString"];
        std::string* strValue;
        if(value.TryGetValueAs<std::string>(strValue)) std::cout << "Value from try get: " << (*strValue) << "\n";

        for(auto& [key, value] : (*tryGetContainer))
        {
            std::cout << key << " : " << value.GetTypeAsString() << "\n";
        }
    }

    std::cout << "Back to string: " << element.ToString() << "\n";

    return 0;
}

