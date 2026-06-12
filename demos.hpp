#ifndef __TESTING_DEMOS_HPP__
#define __TESTING_DEMOS_HPP__

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
    EmbediList<int> list;  // using default allocator

    Serial.println("=== Demo: EmbediList<int> ===");

    // Push elements
    list.append(10);
    list.append(20);
    list.append(30);
    list.append(40);

    Serial.print("List size after pushes: ");
    Serial.println(list.size());

    // Retrieve elements
    for (TypeMacros::u32 i = 0; i < list.size(); i++)
    {
        int* val = list.get(i);
        if (val)
        {
            Serial.print("Element at index ");
            Serial.print(i);
            Serial.print(": ");
            Serial.println(*val);
        }
    }

    // Set element at index 1
    list.set(99, 1);
    Serial.println("After setting index 1 to 99:");
    for (TypeMacros::u32 i = 0; i < list.size(); i++)
    {
        int* val = list.get(i);
        if (val)
        {
            Serial.print("Element at index ");
            Serial.print(i);
            Serial.print(": ");
            Serial.println(*val);
        }
    }

    // Insert element at index 2
    list.insert(77, 2);
    Serial.println("After inserting 77 at index 2:");
    for (TypeMacros::u32 i = 0; i < list.size(); i++)
    {
        int* val = list.get(i);
        if (val)
        {
            Serial.print("Element at index ");
            Serial.print(i);
            Serial.print(": ");
            Serial.println(*val);
        }
    }

    // Remove element at index 3
    list.remove(3);
    Serial.println("After removing element at index 3:");
    for (TypeMacros::u32 i = 0; i < list.size(); i++)
    {
        int* val = list.get(i);
        if (val)
        {
            Serial.print("Element at index ");
            Serial.print(i);
            Serial.print(": ");
            Serial.println(*val);
        }
    }

    // Clear the list
    list.clear();
    Serial.print("List size after clear: ");
    Serial.println(list.size());
}

void demo_stack()
{
    // Create a stack with capacity 5, using heap allocation
    EmbediStack<int> stack(nullptr, 5);

    Serial.println("=== Demo: EmbediStack<int> ===");

    // Push elements
    stack.push(10);
    stack.push(20);
    stack.push(30);

    Serial.print("Stack size after pushes: ");
    Serial.println(stack.size());

    // Peek top element
    int* peekVal = stack.peek();
    if (peekVal)
    {
        Serial.print("Peek top: ");
        Serial.println(*peekVal);
    }

    // Pop element
    int* popped = stack.pop();
    if (popped)
    {
        Serial.print("Popped: ");
        Serial.println(*popped);
    }

    Serial.print("Stack size after pop: ");
    Serial.println(stack.size());

    // Push more
    stack.push(40);
    stack.push(50);

    Serial.println("Stack contents:");
    const int* stack_buf = stack.get_buffer();
    for (TypeMacros::u32 i = 0; i < stack.size(); i++)
    {
        Serial.print("Index ");
        Serial.print(i);
        Serial.print(": ");
        Serial.println(stack_buf[i]); // not ideal, so better to access buffer directly
    }

    // Compare with another stack
    int buf[5];
    EmbediStack<int> other(buf, 5);
    other.push(10);
    other.push(20);
    other.push(40);
    other.push(50);

    Serial.print("Stacks equal? ");
    Serial.println(stack.equals(other) ? "Yes" : "No");

    // Clear stack
    stack.clear();
    Serial.print("Stack size after clear: ");
    Serial.println(stack.size());
}

void demo_dynamicstack()
{
    EmbediDynamicStack<int> stack;  // preallocate 3 objects per block

    Serial.println("=== Demo: EmbediDynamicStack<int> ===");

    // Push elements
    stack.push(10);
    stack.push(20);
    stack.push(30);

    Serial.print("Stack size after pushes: ");
    Serial.println(stack.size());

    // Peek top element
    int* peekVal = stack.peek();
    if (peekVal)
    {
        Serial.print("Peek top: ");
        Serial.println(*peekVal);
    }

    // Pop element
    int* popped = stack.pop();
    if (popped)
    {
        Serial.print("Popped: ");
        Serial.println(*popped);
    }

    Serial.print("Stack size after pop: ");
    Serial.println(stack.size());

    // Push more
    stack.push(40);
    stack.push(50);

    // Dump contents into buffer
    const int* buf = stack.get_buffer();

    Serial.println("Stack contents (bottom to top):");
    for (TypeMacros::u32 i = 0; i < stack.size(); i++)
    {
        Serial.print("Index ");
        Serial.print(i);
        Serial.print(": ");
        Serial.println(buf[i]);
    }

    // Compare with another stack
    EmbediDynamicStack<int> other;
    other.push(10);
    other.push(20);
    other.push(40);
    other.push(50);

    Serial.print("Stacks equal? ");
    Serial.println(stack.equals(other) ? "Yes" : "No");

    // Clear stack
    stack.clear();
    Serial.print("Stack size after clear: ");
    Serial.println(stack.size());
}

void demo_ringbuffer()
{
    EmbediRingBuffer<int> rb(nullptr, 5);  // capacity 5, allocate on heap

    Serial.println("=== Demo: EmbediRingBuffer<int> ===");

    // Push elements
    rb.push(10);
    rb.push(20);
    rb.push(30);

    Serial.print("Buffer size after pushes: ");
    Serial.println(rb.size());

    // Pop one element
    int* popped = rb.pop();
    if (popped)
    {
        Serial.print("Popped: ");
        Serial.println(*popped);
    }

    Serial.print("Buffer size after pop: ");
    Serial.println(rb.size());

    // Push more elements (wrap around test)
    rb.push(40);
    rb.push(50);
    rb.push(60);

    Serial.print("Buffer size after more pushes: ");
    Serial.println(rb.size());

    // Pop all elements
    Serial.println("Buffer contents (pop all):");
    while (rb.size() > 0)
    {
        int* val = rb.pop();
        if (val)
        {
            Serial.print("Value: ");
            Serial.println(*val);
        }
    }

    Serial.print("Buffer size after draining: ");
    Serial.println(rb.size());

    // Clear buffer
    rb.clear();
    Serial.print("Buffer size after clear: ");
    Serial.println(rb.size());
}

#endif