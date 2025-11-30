
#include "JsonParser.hpp"

int main(void) 
{
    //JSON::Parse("../test_1.json");

    JSON::JSONParser parser{"../testJSON/test_1.json"};
    parser.LoadSource();

    JSON::JSONElement element = parser.Parse();

    auto& container = element.GetValueAs<JSON::JSONContainer>();

    auto& test = container.GetValue()["falseBoolean"];
    auto& value = test.GetValueAs<bool>().GetValue();
    std::cout << "Bool Value: " << value << "\n";


    auto& test2 = container.GetValue()["simpleString"];
    auto& sstring = test2.GetValueAs<std::string>().GetValue();
    std::cout << "String Value: " << sstring << "\n";

    return 0;
}

