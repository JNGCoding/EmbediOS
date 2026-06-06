#include <Arduino.h>
#include "TypeMacros.hpp"
#include "DataStructures.hpp"
#include "Allocator.hpp"

void demo_pairs();
void demo_strings();
void demo_list();

void setup() {
    Serial.begin(115200);
}

void loop() {
    // Keep empty to avoid repeating the demo continuously
}

void demo_pairs() 
{
    Serial.println(F("[DEMO] EmbediPair Functional Checks:"));

    EmbediPair<int, const char*> pair1;
    pair1.set(200, "OK");

    EmbediPair<int, const char*> pair2;
    pair2.set(200, "OK");

    Serial.print(F("  Pair 1 Values -> a: "));
    Serial.print(pair1.a);
    Serial.print(F(", b: "));
    Serial.println(pair1.b);

    Serial.print(F("  Does pair1 equal pair2? "));
    Serial.println(pair1.equals(pair2) ? F("Yes") : F("No"));
    Serial.println();
}

void demo_strings() 
{
    Serial.println(F("[DEMO] EmbediString Functional Checks:"));

    // 1. Initialization
    EmbediString str1("Embedi Engine");
    Serial.print(F("  Initial String: \""));
    Serial.print(str1.c_str());
    Serial.print(F("\" (Length: "));
    Serial.print(str1.length());
    Serial.println(F(")"));

    // 2. Case Transformations
    str1.upper();
    Serial.print(F("  After upper():  \""));
    Serial.print(str1.c_str());
    Serial.println(F("\""));

    str1.lower();
    Serial.print(F("  After lower():  \""));
    Serial.print(str1.c_str());
    Serial.println(F("\""));

    // 3. Appending
    str1.append(" 2026");
    Serial.print(F("  After append(): \""));
    Serial.print(str1.c_str());
    Serial.println(F("\""));

    // 4. Searching & Content Validation
    Serial.print(F("  Is purely digits? "));
    Serial.println(str1.is_digit() ? F("Yes") : F("No"));

    if (str1.contains("2026")) {
        i64 index = str1.find("2026");
        Serial.print(F("  Found token \"2026\" at index: "));
        Serial.println((long)index); // Cast i64 for print compatibility
    }

    // 5. Substring Extraction
    EmbediString buffer;
    if (str1.get_substring(buffer, 0, 6)) {
        Serial.print(F("  Extracted Substring [0,6): \""));
        Serial.print(buffer.c_str());
        Serial.println(F("\""));
    }

    // 6. Direct Character Manipulation via pointer
    char* firstChar = str1.get_char(0);
    if (firstChar != nullptr) {
        *firstChar = 'E'; 
        Serial.print(F("  Manual char edit pointer: \""));
        Serial.print(str1.c_str());
        Serial.println(F("\""));
    }

    // 7. Cleanup
    str1.clear();
    Serial.print(F("  After clear() -> Length is: "));
    Serial.println(str1.length());
}

void demo_list()
{
    EmbediList<int> integerList;

    auto print_list = [&]()
    {
        for (u32 i = 0; i < integerList.size(); i++)
        {
            int* element = integerList.get(i);
            if (element == nullptr)
            {
                Serial.print(F("A nullptr encountered at index: "));
                Serial.println(i);
            }
            else
            {
                Serial.print(F("Element at index "));
                Serial.print(i);
                Serial.print(" : ");
                Serial.println(*element);
            }
        }
    };

    // Append items
    for (int i = 0; i < 15; i++)
        integerList.push(i);

    // Print list [ Greater Time Complexity due to functional overhead ]
    print_list();

    // set operation
    integerList.set(1293, 0);
    print_list();

    // push but in middle of the list
    integerList.push(12391, 3);
    print_list();

    // remove from somewhere in the middle
    integerList.remove(6);
    integerList.remove(4);
    print_list();

    // Clear operation
    integerList.clear();
    Serial.print("Size of list after clear(): ");
    Serial.println(integerList.size());
}